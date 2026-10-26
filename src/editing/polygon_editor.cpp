#include "polygon_editor.hpp"
#include "../model/polygon.hpp"
#include "../model/edge/edge.hpp"

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
