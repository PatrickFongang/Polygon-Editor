#pragma once

#include <memory>
#include <cmath>
#include <algorithm>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "../../enum/continuity.hpp"
#include "../../enum/edge_end.hpp"
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

    // derivative of the edge at the given end (parameter t in [0, 1]), oriented along the traversal of the polygon
    virtual sf::Vector2f tangentAt(EdgeEnd end) const = 0;

    // changes the edge so that tangentAt(end) == desired, keeping the vertex at that end in place
    // returns the vertices that had to be moved to achieve it (the caller must re-check their continuity)
    virtual std::vector<Vertex*> adaptTangent(EdgeEnd end, const sf::Vector2f& desired) = 0;

    // can this edge be a neighbour of a vertex with the given continuity class?
    virtual bool supports(Continuity continuity) const { return true; }

    // how willing the edge is to give way when a continuity relation has to be restored: lower adapts first
    virtual int adaptCost() const = 0;

    // draws any helper elements (like control points) for the edge
    virtual void drawHelpers(sf::RenderWindow& window) const {}

protected:
    Vertex* v1;
    Vertex* v2;
};