#pragma once
#include <memory>
#include "../model/polygon.hpp"
#include "../model/vertex.hpp"
#include "../model/edge/edge.hpp"
#include "../enum/continuity.hpp"

class PolygonBuilder {
private:
    std::unique_ptr<Polygon> currentPolygon;
    Vertex* firstVertex = nullptr;
    Vertex* lastVertex = nullptr;

public:
    PolygonBuilder();
    
    PolygonBuilder& addVertex(float x, float y, Continuity continuity = Continuity::G0);
    PolygonBuilder& withLastEdgeAsBezier();
    PolygonBuilder& withLastEdgeAsArc();
    PolygonBuilder& closePolygon();
    
    std::unique_ptr<Polygon> build();
};