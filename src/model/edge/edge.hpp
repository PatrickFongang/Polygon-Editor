#pragma once

#include <memory>
#include <SFML/System/Vector2.hpp>


class Vertex; // Forward declaration of Vertex class

class Edge
{
    public:
        Edge(Vertex* vertex1, Vertex* vertex2) : v1(vertex1), v2(vertex2) {}
        virtual ~Edge() = default; 

        virtual void draw() const = 0;

        Vertex* getVertex1() const { return v1; }
        void setVertex1(Vertex* vertex) { v1 = vertex; }

        Vertex* getVertex2() const { return v2; }
        void setVertex2(Vertex* vertex) { v2 = vertex; }

    protected:
        Vertex* v1;
        Vertex* v2;
};