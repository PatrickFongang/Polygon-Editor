#pragma once
#include <cmath>
#include <SFML/System/Vector2.hpp>

// small header-only helpers for sf::Vector2f (no linking against SFML needed)
namespace vec
{
    inline float dot(const sf::Vector2f &a, const sf::Vector2f &b) { return a.x * b.x + a.y * b.y; }
    inline float cross(const sf::Vector2f &a, const sf::Vector2f &b) { return a.x * b.y - a.y * b.x; }
    inline float length(const sf::Vector2f &a) { return std::hypot(a.x, a.y); }

    // returns a unit vector, or (0, 0) for a zero-length input
    inline sf::Vector2f normalized(const sf::Vector2f &a)
    {
        float len = length(a);
        return len > 0.0f ? a / len : sf::Vector2f(0.f, 0.f);
    }

    // rotates the vector by +90 degrees (in the coordinate system used by sin/cos)
    inline sf::Vector2f perp(const sf::Vector2f &a) { return {-a.y, a.x}; }
}
