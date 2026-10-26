#include "polygon_editor.hpp"
#include "../model/polygon.hpp"
#include "../model/edge/edge.hpp"
#include "../model/edge/edge_factory.hpp"
#include "../model/continuity_policy.hpp"

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
    event.seeds.push_back(vertex);
    for (Vertex* adjacent : vertex->adjacentVertices())
        event.seeds.push_back(adjacent);
    notify(event);
}

void PolygonEditor::moveControlPoint(const ControlHandle& handle, const sf::Vector2f& position)
{
    handle.point->setPosition(position);

    EditEvent event;
    event.seeds.push_back(handle.end == EdgeEnd::Start ? handle.edge->getVertex1() : handle.edge->getVertex2());
    event.driver = handle.edge;
    notify(event);
}

void PolygonEditor::toggleEdgeType(Polygon& polygon, size_t edgeIndex, EdgeType type)
{
    Edge* edge = polygon.getEdge(edgeIndex);
    if (!edge) return;

    Vertex* v1 = edge->getVertex1();
    Vertex* v2 = edge->getVertex2();
    polygon.replaceEdge(edgeIndex, createEdge(type, v1, v2));
    sanitizeAndNotify({v1, v2});
}

void PolygonEditor::splitEdgeAtMidpoint(Polygon& polygon, size_t edgeIndex)
{
    Edge* edge = polygon.getEdge(edgeIndex);
    if (!edge) return;

    Vertex* v1 = edge->getVertex1();
    Vertex* v2 = edge->getVertex2();
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
    if (!polygon.removeVertex(vertexIndex)) return false;

    sanitizeAndNotify({prevVertex, nextVertex});
    return true;
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

void PolygonEditor::notify(const EditEvent& event)
{
    for (auto& rule : rules)
        rule->apply(event);
}
