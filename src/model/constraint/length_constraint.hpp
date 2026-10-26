#pragma once
#include "constraint.hpp"

// The edge has a given length. The correction is shared between the two ends; a pinned end stays in place.
class LengthConstraint : public Constraint {
public:
    LengthConstraint(Edge* edge, float length) : edge(edge), length(length) {}

    Edge* getEdge() const { return edge; }
    float getLength() const { return length; }
    void setLength(float newLength) { length = newLength; }

    std::vector<Edge*> edges() const override { return {edge}; }
    bool isSatisfied(float eps) const override;
    std::vector<Vertex*> project(const PinnedSet& pinned) override;
    std::string label() const override;

private:
    Edge* edge;
    float length;
};
