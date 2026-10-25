#include "line_edge.hpp"
#include "../vertex.hpp"

void LineEdge::draw(const Drawer& drawer) const {
    drawer.drawLine(v1->getPosition().x, v1->getPosition().y, v2->getPosition().x, v2->getPosition().y);
}