#include "polygon_director.hpp"

std::unique_ptr<Polygon> PolygonDirector::createDemoPolygon1()
{
    return PolygonBuilder()
        .addVertex(100.0f, 100.0f, Continuity::G0)
        .addVertex(300.0f, 100.0f, Continuity::G1)
        .withLastEdgeAsBezier()
        .addVertex(200.0f, 200.0f, Continuity::G0)
        .closePolygon()
        .build();
}

std::unique_ptr<Polygon> PolygonDirector::createDemoPolygon2()
{
    return PolygonBuilder()
        .addVertex(400.0f, 300.0f)
        .addVertex(500.0f, 400.0f)
        .withLastEdgeAsArc()
        .addVertex(600.0f, 300.0f)
        .addVertex(500.0f, 200.0f)
        .closePolygon()
        .build();
}