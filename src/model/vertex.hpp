#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
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
        : position(0.f, 0.f), color(sf::Color::White), continuityStrategy(ContinuityStrategy::create(Continuity::G1)) {}

    Vertex(sf::Vector2f pos, sf::Color col = sf::Color::White, Continuity cont = Continuity::G1)
        : position(pos), color(col), continuityStrategy(ContinuityStrategy::create(cont)) {}

    sf::Vector2f getPosition() const { return position; }
    void setPosition(const sf::Vector2f &pos) { position = pos; }
    void move(const sf::Vector2f &delta) { position += delta; }

    sf::Color getColor() const { return color; }
    void setColor(const sf::Color &col) { color = col; }

    Continuity getContinuity() const { return continuityStrategy->getType(); }
    void setContinuity(Continuity cont) { continuityStrategy = ContinuityStrategy::create(cont); }

    const ContinuityStrategy* getContinuityStrategy() const { return continuityStrategy.get(); }

    Edge *getPrevEdge() const { return prevEdge; }
    void setPrevEdge(Edge *edge) { prevEdge = edge; }

    Edge *getNextEdge() const { return nextEdge; }
    void setNextEdge(Edge *edge) { nextEdge = edge; }

private:
    sf::Vector2f position;
    sf::Color color;
    Edge *prevEdge = nullptr;
    Edge *nextEdge = nullptr;
    std::unique_ptr<ContinuityStrategy> continuityStrategy;
};