#pragma once
#include <memory>
#include "../model/polygon.hpp"
#include "../model/vertex.hpp"
#include "../model/edge.hpp"
#include "../enum/continuity.hpp"
#include "../enum/edge_type.hpp"

class PolygonBuilder {
private:
    std::unique_ptr<Polygon> currentPolygon;
    Vertex* firstVertex = nullptr;
    Vertex* lastVertex = nullptr;

public:
    PolygonBuilder();
    
    PolygonBuilder& addVertex(float x, float y, Continuity continuity = Continuity::G0);
    PolygonBuilder& withLastEdgeAs(EdgeType type);
    PolygonBuilder& closePolygon(EdgeType closingEdgeType = EdgeType::Line);
    
    std::unique_ptr<Polygon> build();
};