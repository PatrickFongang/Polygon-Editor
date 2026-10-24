#pragma once
#include "edge.hpp"
#include "../vertex.hpp"

class BezierEdge : public Edge {
private:
    std::unique_ptr<Vertex> controlPoint1;
    std::unique_ptr<Vertex> controlPoint2;

public:
    BezierEdge(Vertex* v1, Vertex* v2) : Edge(v1, v2) {
        controlPoint1 = std::make_unique<Vertex>(sf::Vector2f((v1->getPosition().x + v2->getPosition().x) / 2, v1->getPosition().y));
        controlPoint2 = std::make_unique<Vertex>(sf::Vector2f((v1->getPosition().x + v2->getPosition().x) / 1.5, v2->getPosition().y + 50.0f));
    }

    // incremental drawing method for Bezier curve
    void draw() const override;
};