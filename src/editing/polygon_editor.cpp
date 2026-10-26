#include "polygon_editor.hpp"
#include <algorithm>
#include "../model/polygon.hpp"
#include "../model/edge/edge.hpp"
#include "../model/edge/edge_factory.hpp"
#include "../model/continuity_policy.hpp"
#include "../model/constraint/constraint_system.hpp"
#include "../model/constraint/length_constraint.hpp"
#include "../model/constraint/parallel_constraint.hpp"

void PolygonEditor::addRule(std::unique_ptr<PostEditRule> rule)
{
    rules.push_back(std::move(rule));
}

void PolygonEditor::moveVertex(Polygon& polygon, size_t vertexIndex, const sf::Vector2f& position)
{
    Vertex* vertex = polygon.getVertex(vertexIndex);
    if (!vertex) return;

    polygon.moveVertex(vertexIndex, position);

    EditEvent event;
    event.pinned.push_back(vertex); // follows the cursor, constraints adapt to it
    event.seeds.push_back(vertex);
    for (Vertex* adjacent : vertex->adjacentVertices())
        event.seeds.push_back(adjacent);
    notify(event);
}

void PolygonEditor::moveControlPoint(const ControlHandle& handle, const sf::Vector2f& position)
{
    handle.point->setPosition(position);

    Vertex* vertex = handle.end == EdgeEnd::Start ? handle.edge->getVertex1() : handle.edge->getVertex2();

    EditEvent event;
    event.seeds.push_back(vertex);
    event.pinned.push_back(vertex); // the control point travels with its vertex, so the user's point must not drift
    event.driver = handle.edge;
    notify(event);
}

void PolygonEditor::toggleEdgeType(Polygon& polygon, size_t edgeIndex, EdgeType type)
{
    Edge* edge = polygon.getEdge(edgeIndex);
    if (!edge) return;

    if (edge->type() == type)
        return;

    Vertex* v1 = edge->getVertex1();
    Vertex* v2 = edge->getVertex2();
    // constraints concern segments only, and they must not point at the edge which is about to be replaced
    removeConstraints(constraintsOf({edge}));
    polygon.replaceEdge(edgeIndex, createEdge(type, v1, v2));
    sanitizeAndNotify({v1, v2});
}

void PolygonEditor::splitEdgeAtMidpoint(Polygon& polygon, size_t edgeIndex)
{
    Edge* edge = polygon.getEdge(edgeIndex);
    if (!edge) return;

    Vertex* v1 = edge->getVertex1();
    Vertex* v2 = edge->getVertex2();
    removeConstraints(constraintsOf({edge}));
    polygon.splitEdgeAtMidpoint(edgeIndex);

    // the first of the two new edges starts at v1 and ends at the new vertex
    Vertex* middle = v1->getNextEdge()->getVertex2();
    sanitizeAndNotify({v1, middle, v2});
}

bool PolygonEditor::removeVertex(Polygon& polygon, size_t vertexIndex)
{
    Vertex* vertex = polygon.getVertex(vertexIndex);
    if (!vertex || !vertex->getPrevEdge() || !vertex->getNextEdge()) return false;

    Vertex* prevVertex = vertex->getPrevEdge()->getVertex1();
    Vertex* nextVertex = vertex->getNextEdge()->getVertex2();

    // the two edges of the vertex are replaced by one; the constraints go only if the removal really happens
    std::vector<Constraint*> doomed = constraintsOf({vertex->getPrevEdge(), vertex->getNextEdge()});
    if (!polygon.removeVertex(vertexIndex)) return false;
    removeConstraints(doomed);

    sanitizeAndNotify({prevVertex, nextVertex});
    return true;
}

bool PolygonEditor::setContinuity(Vertex& vertex, Continuity continuity)
{
    if (!ContinuityPolicy::isAllowed(vertex, continuity))
        return false;

    vertex.setContinuity(continuity);
    vertex.hasExplicitContinuity = true;
    sanitizeAndNotify({&vertex});
    return true;
}

LengthConstraint* PolygonEditor::setEdgeLength(Edge& edge, float length)
{
    if (edge.isCurved() || !(length > 0.0f))
        return nullptr;

    LengthConstraint* result = nullptr;
    for (Constraint* constraint : constraints.constraintsOf(&edge))
    {
        if (auto* existing = dynamic_cast<LengthConstraint*>(constraint))
        {
            existing->setLength(length);
            result = existing;
        }
    }
    if (!result)
        result = static_cast<LengthConstraint*>(constraints.add(std::make_unique<LengthConstraint>(&edge, length)));

    notifyAround({&edge});
    return result;
}

ParallelConstraint* PolygonEditor::addParallelConstraint(Edge& first, Edge& second)
{
    if (&first == &second || first.isCurved() || second.isCurved())
        return nullptr;

    for (Constraint* constraint : constraints.constraintsOf(&first))
    {
        if (dynamic_cast<ParallelConstraint*>(constraint) && constraint->involves(&second))
            return nullptr;
    }

    auto* result = static_cast<ParallelConstraint*>(constraints.add(std::make_unique<ParallelConstraint>(&first, &second)));
    notifyAround({&first, &second});
    return result;
}

bool PolygonEditor::removeConstraint(const Constraint* constraint)
{
    return constraints.remove(constraint);
}

std::vector<Constraint*> PolygonEditor::constraintsOf(const std::vector<Edge*>& edges) const
{
    std::vector<Constraint*> result;
    for (const Edge* edge : edges)
    {
        for (Constraint* constraint : constraints.constraintsOf(edge))
        {
            if (std::find(result.begin(), result.end(), constraint) == result.end())
                result.push_back(constraint);
        }
    }
    return result;
}

void PolygonEditor::removeConstraints(const std::vector<Constraint*>& doomed)
{
    for (const Constraint* constraint : doomed)
        constraints.remove(constraint);
}

void PolygonEditor::notifyAround(const std::vector<Edge*>& edges)
{
    EditEvent event;
    for (Edge* edge : edges)
    {
        event.seeds.push_back(edge->getVertex1());
        event.seeds.push_back(edge->getVertex2());
    }
    notify(event);
}

void PolygonEditor::sanitizeAndNotify(const std::vector<Vertex*>& vertices)
{
    ContinuityPolicy::sanitize(vertices);

    EditEvent event;
    for (Vertex* vertex : vertices)
    {
        event.seeds.push_back(vertex);
        for (Vertex* adjacent : vertex->adjacentVertices())
            event.seeds.push_back(adjacent);
    }
    notify(event);
}

void PolygonEditor::enforceAll(Polygon& polygon)
{
    EditEvent event;
    for (const auto& vertex : polygon.getVertices())
        event.seeds.push_back(vertex.get());
    notify(event);
}

void PolygonEditor::notify(EditEvent event)
{
    for (int round = 0; round < MAX_ROUNDS; ++round)
    {
        std::vector<Vertex*> moved;
        for (auto& rule : rules)
        {
            std::vector<Vertex*> movedByRule = rule->apply(event);
            moved.insert(moved.end(), movedByRule.begin(), movedByRule.end());
        }
        if (moved.empty())
            break;

        // the driver and the pinned vertices stay: the user holds them during all the rounds
        event.seeds.clear();
        for (Vertex* vertex : moved)
        {
            event.seeds.push_back(vertex);
            for (Vertex* adjacent : vertex->adjacentVertices())
                event.seeds.push_back(adjacent);
        }
    }
}
