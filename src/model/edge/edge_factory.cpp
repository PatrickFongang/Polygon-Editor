#include "edge_factory.hpp"
#include "line_edge.hpp"
#include "bezier_edge.hpp"
#include "arc_edge.hpp"

std::unique_ptr<Edge> createEdge(EdgeType type, Vertex* v1, Vertex* v2)
{
    switch (type)
    {
    case EdgeType::Bezier: return std::make_unique<BezierEdge>(v1, v2);
    case EdgeType::Arc: return std::make_unique<ArcEdge>(v1, v2);
    case EdgeType::Line: break;
    }
    return std::make_unique<LineEdge>(v1, v2);
}
