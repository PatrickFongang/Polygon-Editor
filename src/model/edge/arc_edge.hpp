#pragma once
#include <optional>
#include "edge.hpp"
#include "../../utils/vec_math.hpp"

// Circular arc between v1 and v2.
//
// Without an anchor (both ends G0) it is a semicircle whose radius is half of the chord.
// With an anchor - a tangent direction required at one end (G1) - the arc is the only circular arc through
// both endpoints that has this tangent at the anchored end. The other end is then free (G0).
class ArcEdge : public Edge {
public:
    ArcEdge(Vertex* v1, Vertex* v2) : Edge(v1, v2) {}

    void draw(const Drawer& drawer) const override;

    float distanceToPoint(const sf::Vector2f& pt) const override;

    sf::Vector2f tangentAt(EdgeEnd end) const override;
    // anchors the arc to the desired tangent direction at the given end (the length of the vector is ignored)
    std::vector<Vertex*> adaptTangent(EdgeEnd end, const sf::Vector2f& desired) override;
    void releaseTangent(EdgeEnd end) override;
    bool supports(Continuity continuity) const override { return continuity != Continuity::C1; }
    EdgeType type() const override { return EdgeType::Arc; }
    bool isCurved() const override { return true; }
    bool derivesTangent() const override { return true; }
    int adaptCost() const override { return 0; }

    // geometry of the arc, public for tests; points are given by center + radius * (cos a, sin a), a = startAngle + sweep * t
    struct Geometry {
        bool straight = false; // degenerate arc (infinite radius or coinciding ends), drawn as a segment
        sf::Vector2f center;
        float radius = 0.0f;
        float startAngle = 0.0f;
        float sweep = 0.0f; // signed central angle
    };
    Geometry geometry() const;
    sf::Vector2f pointAt(const Geometry& geometry, float t) const;

private:
    struct Anchor {
        EdgeEnd end;
        sf::Vector2f direction; // tangent along the traversal of the polygon
    };
    std::optional<Anchor> anchor;
};
