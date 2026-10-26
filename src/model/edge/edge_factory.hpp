#pragma once
#include <memory>
#include "edge.hpp"
#include "../../enum/edge_type.hpp"

// creates an edge of the requested type between two vertices
std::unique_ptr<Edge> createEdge(EdgeType type, Vertex* v1, Vertex* v2);
