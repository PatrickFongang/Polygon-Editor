#include "edge.hpp"

sf::Vector2f Edge::getMidpoint() const
{
    return (v1->getPosition() + v2->getPosition()) * 0.5f;
}

float Edge::getLength() const
{
    sf::Vector2f diff = v2->getPosition() - v1->getPosition();
    return std::hypot(diff.x, diff.y);
}

float Edge::distanceToPoint(const sf::Vector2f &pt) const
{
    sf::Vector2f a = v1->getPosition();
    sf::Vector2f b = v2->getPosition();
    sf::Vector2f ab = b - a;
    sf::Vector2f ap = pt - a;
    float lenSq = ab.x * ab.x + ab.y * ab.y;
    if (lenSq == 0.0f)
    {
        return std::hypot(ap.x, ap.y);
    }
    float t = std::clamp((ap.x * ab.x + ap.y * ab.y) / lenSq, 0.0f, 1.0f);
    sf::Vector2f proj = a + t * ab;
    return std::hypot(pt.x - proj.x, pt.y - proj.y);
}
