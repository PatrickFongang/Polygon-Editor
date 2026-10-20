#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include "../enum/continuity.hpp"

class Edge; // Forward declaration of Edge class

class Vertex
{
    private:
        sf::Vector2f position;  
        sf::Color    color;
        Edge* prevEdge = nullptr;
        Edge* nextEdge = nullptr;     
        Continuity   continuity;

    public:
        bool isSelected = false; // Flag to indicate if the vertex is selected
        bool isHovered = false;  // Flag to indicate if the vertex is hovered over
        bool hasPredefinedContinuity = false; // Flag to indicate if the vertex has a predefined continuity

        Vertex() : position(0.f, 0.f), color(sf::Color::Black), continuity(Continuity::G0) {}

        Vertex(sf::Vector2f pos, sf::Color col, Continuity cont = Continuity::G0)
            : position(pos), color(col), continuity(cont) {}

        sf::Vector2f getPosition() const { return position; }
        void setPosition(const sf::Vector2f& pos) { position = pos; }

        sf::Color getColor() const { return color; }
        void setColor(const sf::Color& col) { color = col; }

        Continuity getContinuity() const { return continuity; }
        void setContinuity(Continuity cont) { continuity = cont; }

        Edge* getPrevEdge() const { return prevEdge; }
        void setPrevEdge(Edge* edge) { prevEdge = edge; }

        Edge* getNextEdge() const { return nextEdge; }
        void setNextEdge(Edge* edge) { nextEdge = edge; }
};
