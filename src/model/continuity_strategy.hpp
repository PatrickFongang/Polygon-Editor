#pragma once
#include <optional>
#include <SFML/System/Vector2.hpp>
#include "../enum/continuity.hpp"
#include "../utils/vec_math.hpp"

// A continuity class describes the relation between the tangent vectors of two edges meeting at a vertex.
// Both tangents are the derivatives of the edges at the vertex, oriented along the traversal of the polygon,
// so a smooth join has both vectors pointing the same way.
// Strategies know nothing about concrete edge types - they only do vector math.
class ContinuityStrategy {
public:
    virtual ~ContinuityStrategy() = default;
    virtual Continuity getType() const = 0;

    // Tangent the follower edge should have to satisfy the relation with the driver edge.
    // Returns nullopt when the relation puts no constraint on the follower.
    virtual std::optional<sf::Vector2f> desiredFollower(const sf::Vector2f& driver, const sf::Vector2f& follower) const = 0;

    // Is the relation satisfied? eps is an angular tolerance (sine of the angle) for G1 and a length tolerance for C1.
    virtual bool isSatisfied(const sf::Vector2f& a, const sf::Vector2f& b, float eps) const = 0;

    // strategies are stateless, so one shared instance per continuity class is enough
    static const ContinuityStrategy& get(Continuity type);
};

class G0Strategy : public ContinuityStrategy {
public:
    Continuity getType() const override { return Continuity::G0; }

    std::optional<sf::Vector2f> desiredFollower(const sf::Vector2f&, const sf::Vector2f&) const override {
        return std::nullopt;
    }

    bool isSatisfied(const sf::Vector2f&, const sf::Vector2f&, float) const override { return true; }
};

// same direction of the unit tangent vector, the follower keeps its own length
class G1Strategy : public ContinuityStrategy {
public:
    Continuity getType() const override { return Continuity::G1; }

    std::optional<sf::Vector2f> desiredFollower(const sf::Vector2f& driver, const sf::Vector2f& follower) const override {
        float driverLen = vec::length(driver);
        if (driverLen <= 0.0f)
            return std::nullopt; // direction is undefined, nothing to follow

        float followerLen = vec::length(follower);
        if (followerLen <= 0.0f)
            followerLen = driverLen; // a degenerate follower has no length worth keeping

        return driver / driverLen * followerLen;
    }

    bool isSatisfied(const sf::Vector2f& a, const sf::Vector2f& b, float eps) const override {
        sf::Vector2f ua = vec::normalized(a);
        sf::Vector2f ub = vec::normalized(b);
        if (ua == sf::Vector2f(0.f, 0.f) || ub == sf::Vector2f(0.f, 0.f))
            return true; // a zero tangent has no direction to compare
        return std::abs(vec::cross(ua, ub)) <= eps && vec::dot(ua, ub) > 0.0f;
    }
};

// equal tangent vectors (direction and length)
class C1Strategy : public ContinuityStrategy {
public:
    Continuity getType() const override { return Continuity::C1; }

    std::optional<sf::Vector2f> desiredFollower(const sf::Vector2f& driver, const sf::Vector2f&) const override {
        return driver;
    }

    bool isSatisfied(const sf::Vector2f& a, const sf::Vector2f& b, float eps) const override {
        return vec::length(a - b) <= eps;
    }
};

inline const ContinuityStrategy& ContinuityStrategy::get(Continuity type) {
    static const G0Strategy g0;
    static const G1Strategy g1;
    static const C1Strategy c1;
    switch (type) {
        case Continuity::G0: return g0;
        case Continuity::G1: return g1;
        case Continuity::C1: return c1;
    }
    return g0;
}
