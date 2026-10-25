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

    Vertex* hitTestControlPoint(const sf::Vector2f& pt, float radius) const override;

    void translateControlPoints(const sf::Vector2f& delta) override;

    void onEndpointMoved(Vertex* movedVertex, const sf::Vector2f& delta) override;

    float distanceToPoint(const sf::Vector2f& pt) const override;
    void drawHelpers(sf::RenderWindow& window) const override;
};