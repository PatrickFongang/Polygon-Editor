#pragma once
#include "edge.hpp"
#include "../../utils/vec_math.hpp"

class ArcEdge : public Edge {
public:
    ArcEdge(Vertex* v1, Vertex* v2) : Edge(v1, v2) {}

    void draw(const Drawer& drawer) const override;

    float distanceToPoint(const sf::Vector2f& pt) const override;

    // for now the arc is always a semicircle (both ends G0), so only the tangent can be read
    sf::Vector2f tangentAt(EdgeEnd end) const override;
    // TODO: anchor the arc to a tangent at one end (needs the arc geometry from the design)
    std::vector<Vertex*> adaptTangent(EdgeEnd, const sf::Vector2f&) override { return {}; }
    bool supports(Continuity continuity) const override { return continuity != Continuity::C1; }
    int adaptCost() const override { return 0; }
};