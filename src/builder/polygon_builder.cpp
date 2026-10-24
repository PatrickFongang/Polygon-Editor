#include "polygon_builder.hpp"
#include "../model/edge/bezier_edge.hpp"
#include "../model/edge/arc_edge.hpp"
#include "../model/edge/line_edge.hpp"


PolygonBuilder::PolygonBuilder() {
    currentPolygon = std::make_unique<Polygon>();
}

PolygonBuilder& PolygonBuilder::addVertex(float x, float y, Continuity continuity) {
    auto newVertex = std::make_unique<Vertex>(sf::Vector2f(x, y));
    newVertex->setContinuity(continuity);
    
    Vertex* vertexPtr = newVertex.get(); 
    currentPolygon->addVertex(std::move(newVertex)); 

    if (!firstVertex) {
        firstVertex = vertexPtr;
    } else {
        auto newEdge = std::make_unique<LineEdge>(lastVertex, vertexPtr);
        currentPolygon->addEdge(std::move(newEdge));
    }
    
    lastVertex = vertexPtr;
    return *this;
}

PolygonBuilder& PolygonBuilder::withLastEdgeAsBezier() {
    if (!currentPolygon->getEdges().empty()) {
        auto& edges = currentPolygon->getEdges();
        Vertex* v1 = edges.back()->getVertex1();
        Vertex* v2 = edges.back()->getVertex2();

        // change the last edge to BezierEdge
        currentPolygon->replaceLastEdge(std::make_unique<BezierEdge>(v1, v2));
    }
    return *this;
}

PolygonBuilder& PolygonBuilder::withLastEdgeAsArc() {
    if (!currentPolygon->getEdges().empty()) {
        auto& edges = currentPolygon->getEdges();
        Vertex* v1 = edges.back()->getVertex1();
        Vertex* v2 = edges.back()->getVertex2();

        // change the last edge to ArcEdge
        currentPolygon->replaceLastEdge(std::make_unique<ArcEdge>(v1, v2));
    }
    return *this;
}


PolygonBuilder& PolygonBuilder::closePolygon() {
    if (firstVertex && lastVertex && firstVertex != lastVertex) {
        auto closingEdge = std::make_unique<LineEdge>(lastVertex, firstVertex);
        
        currentPolygon->addEdge(std::move(closingEdge));
    }
    return *this;
}

std::unique_ptr<Polygon> PolygonBuilder::build() {
    return std::move(currentPolygon);
}