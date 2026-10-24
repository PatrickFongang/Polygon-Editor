#include "polygon_builder.hpp"

PolygonBuilder::PolygonBuilder() {
    currentPolygon = std::make_unique<Polygon>();
}

PolygonBuilder& PolygonBuilder::addVertex(float x, float y, Continuity continuity) {
    auto newVertex = std::make_unique<Vertex>(x, y);
    newVertex->setContinuity(continuity);
    
    Vertex* vertexPtr = newVertex.get(); 
    currentPolygon->addVertex(std::move(newVertex)); 

    if (!firstVertex) {
        firstVertex = vertexPtr;
    } else {
        auto newEdge = std::make_unique<Edge>(lastVertex, vertexPtr);
        currentPolygon->addEdge(std::move(newEdge));
    }
    
    lastVertex = vertexPtr;
    return *this;
}

PolygonBuilder& PolygonBuilder::withLastEdgeAs(EdgeType type) {
    if (!currentPolygon->getEdges().empty()) {
        auto* lastEdge = currentPolygon->getEdges().back().get();
        lastEdge->setType(type);
        
    }
    return *this;
}

PolygonBuilder& PolygonBuilder::closePolygon(EdgeType closingEdgeType) {
    if (firstVertex && lastVertex && firstVertex != lastVertex) {
        auto closingEdge = std::make_unique<Edge>(lastVertex, firstVertex);
        closingEdge->setType(closingEdgeType);
        
        
        currentPolygon->addEdge(std::move(closingEdge));
    }
    return *this;
}

std::unique_ptr<Polygon> PolygonBuilder::build() {
    return std::move(currentPolygon);
}