#include <SFML/Graphics.hpp>
#include <cmath>
#include "arc_edge.hpp"
#include "../vertex.hpp"

namespace
{
    constexpr float PI = 3.14159265358979323846f;
    // below this sine of the angle between the tangent and the chord the arc is treated as a segment
    constexpr float STRAIGHT_EPS = 1e-4f;

    // arc from p to q which leaves p along the direction t
    ArcEdge::Geometry arcFromStartTangent(sf::Vector2f p, sf::Vector2f q, sf::Vector2f t)
    {
        ArcEdge::Geometry g;
        sf::Vector2f chord = q - p;
        float chordLen = vec::length(chord);
        sf::Vector2f unitT = vec::normalized(t);
        if (chordLen <= 0.0f || unitT == sf::Vector2f(0.f, 0.f))
        {
            g.straight = true;
            return g;
        }

        float sine = vec::cross(unitT, chord) / chordLen;
        if (std::abs(sine) < STRAIGHT_EPS)
        {
            g.straight = true;
            return g;
        }

        // the angle between the tangent and the chord is half of the central angle
        float alpha = std::atan2(vec::cross(unitT, chord), vec::dot(unitT, chord));
        float signedRadius = chordLen / (2.0f * sine);
        g.center = p + vec::perp(unitT) * signedRadius;
        g.radius = std::abs(signedRadius);
        g.sweep = 2.0f * alpha;
        g.startAngle = std::atan2(p.y - g.center.y, p.x - g.center.x);
        return g;
    }
}

ArcEdge::Geometry ArcEdge::geometry() const
{
    sf::Vector2f p = v1->getPosition();
    sf::Vector2f q = v2->getPosition();

    Geometry g;
    if (!anchor)
    {
        // semicircle over the chord
        g.center = (p + q) * 0.5f;
        g.radius = vec::length(p - g.center);
        g.startAngle = std::atan2(p.y - g.center.y, p.x - g.center.x);
        g.sweep = PI;
        g.straight = g.radius <= 0.0f;
        return g;
    }

    if (anchor->end == EdgeEnd::Start)
        return arcFromStartTangent(p, q, anchor->direction);

    // anchored at the end: build the arc travelled backwards (from q to p, leaving q along -direction) and reverse it
    g = arcFromStartTangent(q, p, -anchor->direction);
    if (!g.straight)
    {
        g.startAngle += g.sweep;
        g.sweep = -g.sweep;
    }
    return g;
}

sf::Vector2f ArcEdge::pointAt(const Geometry& g, float t) const
{
    if (g.straight)
        return v1->getPosition() + (v2->getPosition() - v1->getPosition()) * t;

    float angle = g.startAngle + g.sweep * t;
    return {g.center.x + g.radius * std::cos(angle), g.center.y + g.radius * std::sin(angle)};
}

sf::Vector2f ArcEdge::tangentAt(EdgeEnd end) const
{
    Geometry g = geometry();
    if (g.straight)
        return v2->getPosition() - v1->getPosition();

    // d/dt of the point at t = 0 or t = 1
    float angle = g.startAngle + (end == EdgeEnd::Start ? 0.0f : g.sweep);
    return g.radius * g.sweep * sf::Vector2f(-std::sin(angle), std::cos(angle));
}

std::vector<Vertex*> ArcEdge::adaptTangent(EdgeEnd end, const sf::Vector2f& desired)
{
    if (vec::length(desired) > 0.0f)
        anchor = Anchor{end, desired};
    return {};
}

void ArcEdge::releaseTangent(EdgeEnd end)
{
    if (anchor && anchor->end == end)
        anchor.reset();
}

void ArcEdge::draw(const Drawer& drawer) const
{
    Geometry g = geometry();

    // longer arcs need more segments to stay smooth
    int segments = g.straight ? 1 : std::clamp(static_cast<int>(std::abs(g.sweep) / PI * 30.0f), 8, 90);

    sf::Vector2f prevPos = v1->getPosition();
    for (int i = 1; i <= segments; ++i)
    {
        sf::Vector2f currentPos = pointAt(g, static_cast<float>(i) / segments);
        drawer.drawLine(prevPos.x, prevPos.y, currentPos.x, currentPos.y);
        prevPos = currentPos;
    }
}

float ArcEdge::distanceToPoint(const sf::Vector2f& pt) const
{
    Geometry g = geometry();
    if (g.straight && v1->getPosition() == v2->getPosition())
        return vec::length(pt - v1->getPosition());

    const int N = 100;
    float minDist = 1e9f;
    sf::Vector2f prev = v1->getPosition();

    for (int i = 1; i <= N; ++i)
    {
        sf::Vector2f curr = pointAt(g, static_cast<float>(i) / N);

        sf::Vector2f ab = curr - prev;
        sf::Vector2f ap = pt - prev;
        float lenSq = ab.x * ab.x + ab.y * ab.y;
        float segT = (lenSq > 0.0f) ? std::clamp((ap.x * ab.x + ap.y * ab.y) / lenSq, 0.0f, 1.0f) : 0.0f;
        sf::Vector2f proj = prev + segT * ab;
        minDist = std::min(minDist, vec::length(pt - proj));

        prev = curr;
    }
    return minDist;
}
