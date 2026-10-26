#include "line_edge.hpp"
#include "../vertex.hpp"

void LineEdge::draw(const Drawer& drawer) const {
    drawer.drawLine(v1->getPosition().x, v1->getPosition().y, v2->getPosition().x, v2->getPosition().y);
}

sf::Vector2f LineEdge::tangentAt(EdgeEnd) const {
    return v2->getPosition() - v1->getPosition();
}

std::vector<Vertex*> LineEdge::adaptTangent(EdgeEnd end, const sf::Vector2f& desired) {
    if (end == EdgeEnd::Start) {
        v2->moveWithEdges(v1->getPosition() + desired);
        return {v2};
    }
    v1->moveWithEdges(v2->getPosition() - desired);
    return {v1};
}
