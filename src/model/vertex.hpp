#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>
#include "../enum/continuity.hpp"
#include "continuity_strategy.hpp"

class Edge; // Forward declaration of Edge class

class Vertex
{
public:
    bool isSelected = false;              // Flag to indicate if the vertex is selected
    bool isHovered = false;               // Flag to indicate if the vertex is hovered over
    bool hasPredefinedContinuity = false; // Flag to indicate if the vertex has a predefined continuity

    Vertex()
        : position(0.f, 0.f), color(sf::Color::White), continuity(Continuity::G1) {}

    Vertex(sf::Vector2f pos, sf::Color col = sf::Color::White, Continuity cont = Continuity::G1)
        : position(pos), color(col), continuity(cont) {}

    sf::Vector2f getPosition() const { return position; }
    void setPosition(const sf::Vector2f &pos) { position = pos; }
    void move(const sf::Vector2f &delta) { position += delta; }

    // moves the vertex and lets the adjacent edges react (e.g. Bezier control points follow the endpoint)
    void moveWithEdges(const sf::Vector2f &pos);

    // the vertices at the other ends of the adjacent edges
    std::vector<Vertex *> adjacentVertices() const;

    sf::Color getColor() const { return color; }
    void setColor(const sf::Color &col) { color = col; }

    Continuity getContinuity() const { return continuity; }
    void setContinuity(Continuity cont) { continuity = cont; }

    const ContinuityStrategy &getContinuityStrategy() const { return ContinuityStrategy::get(continuity); }

    Edge *getPrevEdge() const { return prevEdge; }
    void setPrevEdge(Edge *edge) { prevEdge = edge; }

    Edge *getNextEdge() const { return nextEdge; }
    void setNextEdge(Edge *edge) { nextEdge = edge; }

private:
    sf::Vector2f position;
    sf::Color color;
    Edge *prevEdge = nullptr;
    Edge *nextEdge = nullptr;
    Continuity continuity;
};