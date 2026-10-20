#pragma once

#include <memory>
#include <SFML/System/Vector2.hpp>
#include "../enum/EdgeType.hpp"


class Vertex; // Forward declaration of Vertex class

class Edge
{
    private:
        Vertex* v1;
        Vertex* v2;
        EdgeType type;
        sf::Vector2f controlPoint1; // For Bezier curves
        sf::Vector2f controlPoint2; // For Bezier curves
    
    public:
        Edge(Vertex* vertex1, Vertex* vertex2, EdgeType t) : v1(vertex1), v2(vertex2), type(t) {}

        Vertex* getVertex1() const { return v1; }
        void setVertex1(Vertex* vertex) { v1 = vertex; }

        Vertex* getVertex2() const { return v2; }
        void setVertex2(Vertex* vertex) { v2 = vertex; }

        EdgeType getType() const { return type; }
        void setType(EdgeType t) { type = t; }

        sf::Vector2f getControlPoint1() const { return controlPoint1; }
        void setControlPoint1(const sf::Vector2f& cp) { controlPoint1 = cp; }

        sf::Vector2f getControlPoint2() const { return controlPoint2; }
        void setControlPoint2(const sf::Vector2f& cp) { controlPoint2 = cp; }
};