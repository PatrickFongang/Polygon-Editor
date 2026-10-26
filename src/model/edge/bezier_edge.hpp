#pragma once
#include "edge.hpp"
#include "../vertex.hpp"

class BezierEdge : public Edge {
private:
    std::unique_ptr<Vertex> controlPoint1;
    std::unique_ptr<Vertex> controlPoint2;

public:
    BezierEdge(Vertex* v1, Vertex* v2);

    Vertex* getControlPoint1() const { return controlPoint1.get(); }
    Vertex* getControlPoint2() const { return controlPoint2.get(); }

    void draw(const Drawer& drawer) const override;

    std::optional<ControlHandle> hitTestControlPoint(const sf::Vector2f& pt, float radius) override;

    void translateControlPoints(const sf::Vector2f& delta) override;

    void onEndpointMoved(Vertex* movedVertex, const sf::Vector2f& delta) override;

    // B'(0) = 3 (P1 - P0), B'(1) = 3 (P3 - P2)
    sf::Vector2f tangentAt(EdgeEnd end) const override;
    // moves the control point next to the end, no vertex has to move
    std::vector<Vertex*> adaptTangent(EdgeEnd end, const sf::Vector2f& desired) override;
    int adaptCost() const override { return 1; }
    bool isCurved() const override { return true; }

    float distanceToPoint(const sf::Vector2f& pt) const override;
    void drawHelpers(sf::RenderWindow& window) const override;
};