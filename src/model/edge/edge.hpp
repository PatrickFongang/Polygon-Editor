#pragma once

#include <memory>
#include <cmath>
#include <algorithm>
#include <SFML/System/Vector2.hpp>
#include "../../utils/drawer.hpp"
#include "../vertex.hpp"

class Edge
{
public:
    Edge(Vertex* vertex1, Vertex* vertex2) : v1(vertex1), v2(vertex2) {}
    virtual ~Edge() = default; 

    virtual void draw(const Drawer& drawer) const = 0;

    Vertex* getVertex1() const { return v1; }
    void setVertex1(Vertex* vertex) { v1 = vertex; }

    Vertex* getVertex2() const { return v2; }
    void setVertex2(Vertex* vertex) { v2 = vertex; }

    sf::Vector2f getMidpoint() const;

    float getLength() const;

    // distanceToPoint calculates the shortest distance from a point to the edge (line segment)
    virtual float distanceToPoint(const sf::Vector2f& pt) const;

    // returns the control point if the click was within the hitbox radius, otherwise returns nullptr
    virtual Vertex* hitTestControlPoint(const sf::Vector2f& pt, float radius) const {
        return nullptr;
    }

    // translates the control points of the edge by a given delta vector
    virtual void translateControlPoints(const sf::Vector2f& delta) {}

    virtual void onEndpointMoved(Vertex* movedVertex, const sf::Vector2f& delta) {}

    // draws any helper elements (like control points) for the edge
    virtual void drawHelpers(sf::RenderWindow& window) const {}

protected:
    Vertex* v1;
    Vertex* v2;
};