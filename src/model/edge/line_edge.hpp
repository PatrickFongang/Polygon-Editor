#include "edge.hpp"

class LineEdge : public Edge {
public:
    LineEdge(Vertex* v1, Vertex* v2) : Edge(v1, v2) {}

    void draw() const override;
};