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

    PolygonBuilder& addVertex(float x, float y, Continuity continuity, bool isExplicit);

public:
    PolygonBuilder();
    
    // the continuity is decided by the ContinuityPolicy once the polygon is built
    PolygonBuilder& addVertex(float x, float y);
    // the continuity is fixed explicitly (the policy only downgrades it if it is impossible)
    PolygonBuilder& addVertex(float x, float y, Continuity continuity);
    PolygonBuilder& withLastEdgeAsBezier();
    PolygonBuilder& withLastEdgeAsArc();
    PolygonBuilder& closePolygon();
    
    std::unique_ptr<Polygon> build();
};