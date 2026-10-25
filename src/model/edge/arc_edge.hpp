#pragma once
#include "edge.hpp"

class ArcEdge : public Edge {
public:
    ArcEdge(Vertex* v1, Vertex* v2) : Edge(v1, v2) {}

    void draw(const Drawer& drawer) const override;
};