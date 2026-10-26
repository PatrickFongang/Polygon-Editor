#pragma once
#include "edge.hpp"

class LineEdge : public Edge {
public:
    LineEdge(Vertex* v1, Vertex* v2) : Edge(v1, v2) {}

    void draw(const Drawer& drawer) const override;

    // a line has the same tangent at both ends: the whole v1 -> v2 vector
    sf::Vector2f tangentAt(EdgeEnd end) const override;
    // rotates (and scales) the line around the vertex at `end` by moving the opposite vertex
    std::vector<Vertex*> adaptTangent(EdgeEnd end, const sf::Vector2f& desired) override;
    int adaptCost() const override { return 2; }
};