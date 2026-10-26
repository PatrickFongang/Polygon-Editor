#pragma once
#include <vector>
#include "post_edit_rule.hpp"

class Vertex;
class Edge;

// Restores the continuity (G0 / G1 / C1) at the vertices touched by an edit.
//
// At every vertex the two adjacent edges are compared through their tangents. If the relation chosen for the
// vertex is broken, one edge (the follower) is adapted to the other (the driver):
//  - the edge the user is dragging is always the driver,
//  - otherwise the edge with the lower adaptCost() gives way (arc < Bezier < line).
// Adapting a line moves its other vertex, so the check is repeated around every moved vertex.
class ContinuityEnforcer : public PostEditRule {
public:
    void apply(const EditEvent& event) override;

private:
    static constexpr float TOLERANCE = 1e-3f;
    // protects against relations that cannot be satisfied all at once
    static constexpr int MAX_STEPS = 1000;

    // returns the vertices moved while restoring the continuity at the given vertex
    static std::vector<Vertex*> enforceAt(Vertex& vertex, Edge* driver);
};
