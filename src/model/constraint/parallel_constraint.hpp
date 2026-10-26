#pragma once
#include "constraint.hpp"

// Two edges lie on parallel lines (the direction of the edges does not matter).
// Both edges are rotated by a part of the angle between them, so the relation is restored symmetrically:
// an edge turns around its pinned end if it has one, otherwise around its midpoint. An edge with both ends
// pinned stays, and the other one turns by the whole angle.
class ParallelConstraint : public Constraint {
public:
    ParallelConstraint(Edge* first, Edge* second) : first(first), second(second) {}

    Edge* getFirst() const { return first; }
    Edge* getSecond() const { return second; }

    std::vector<Edge*> edges() const override { return {first, second}; }
    // eps is the angle between the directions in radians (modulo pi)
    bool isSatisfied(float eps) const override;
    std::vector<Vertex*> project(const PinnedSet& pinned) override;
    std::string label() const override { return "||"; }

    // signed angle in (-pi/2, pi/2] by which the second edge has to be turned to be parallel to the first one
    float angleError() const;

private:
    Edge* first;
    Edge* second;
};
