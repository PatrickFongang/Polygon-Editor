#include <SFML/Graphics.hpp>
#include "bezier_edge.hpp"
#include "../vertex.hpp"

BezierEdge::BezierEdge(Vertex *v1, Vertex *v2) : Edge(v1, v2)
{
    sf::Vector2f p1 = v1->getPosition();
    sf::Vector2f p2 = v2->getPosition();
    sf::Vector2f dir = p2 - p1;
    sf::Vector2f normal(-dir.y, dir.x);
    float len = std::hypot(normal.x, normal.y);
    if (len > 0.0f)
        normal /= len;

    controlPoint1 = std::make_unique<Vertex>(p1 + dir * (1.0f / 3.0f) + normal * 40.0f);
    controlPoint2 = std::make_unique<Vertex>(p1 + dir * (2.0f / 3.0f) + normal * 40.0f);
}

Vertex *BezierEdge::hitTestControlPoint(const sf::Vector2f &pt, float radius) const
{
    sf::Vector2f d1 = pt - controlPoint1->getPosition();
    if (std::hypot(d1.x, d1.y) <= radius)
        return controlPoint1.get();

    sf::Vector2f d2 = pt - controlPoint2->getPosition();
    if (std::hypot(d2.x, d2.y) <= radius)
        return controlPoint2.get();

    return nullptr;
}

void BezierEdge::translateControlPoints(const sf::Vector2f &delta)
{
    controlPoint1->move(delta);
    controlPoint2->move(delta);
}

void BezierEdge::onEndpointMoved(Vertex *movedVertex, const sf::Vector2f &delta)
{
    if (movedVertex == v1)
        controlPoint1->move(delta);
    if (movedVertex == v2)
        controlPoint2->move(delta);
}

void BezierEdge::draw(const Drawer &drawer) const
{
    const int N = 50;
    const float dt = 1.0f / N;

    sf::Vector2f P0(v1->getPosition().x, v1->getPosition().y);
    sf::Vector2f P1(controlPoint1->getPosition().x, controlPoint1->getPosition().y);
    sf::Vector2f P2(controlPoint2->getPosition().x, controlPoint2->getPosition().y);
    sf::Vector2f P3(v2->getPosition().x, v2->getPosition().y);

    // calculate the coefficients for the cubic Bezier curve
    sf::Vector2f a = -P0 + 3.0f * P1 - 3.0f * P2 + P3;
    sf::Vector2f b = 3.0f * P0 - 6.0f * P1 + 3.0f * P2;
    sf::Vector2f c = -3.0f * P0 + 3.0f * P1;
    sf::Vector2f d = P0;

    // incremental drawing using forward differences
    float dt2 = dt * dt;
    float dt3 = dt2 * dt;

    sf::Vector2f currentPos = d;

    sf::Vector2f delta1 = a * dt3 + b * dt2 + c * dt;
    sf::Vector2f delta2 = 6.0f * a * dt3 + 2.0f * b * dt2;
    sf::Vector2f delta3 = 6.0f * a * dt3;

    sf::Vector2f prevPos = currentPos;

    for (int i = 1; i <= N; ++i)
    {
        currentPos += delta1;
        delta1 += delta2;
        delta2 += delta3;

        drawer.drawLine(prevPos.x, prevPos.y, currentPos.x, currentPos.y);

        prevPos = currentPos;
    }
}

// This function calculates the shortest distance from a point to the Bezier curve
// it samples the curve at N line segments and finds the minimum distance to those segments
float BezierEdge::distanceToPoint(const sf::Vector2f& pt) const {
    const int N = 100;
    const float dt = 1.0f / N;
    sf::Vector2f P0 = v1->getPosition();
    sf::Vector2f P1 = controlPoint1->getPosition();
    sf::Vector2f P2 = controlPoint2->getPosition();
    sf::Vector2f P3 = v2->getPosition();

    float minDist = 1e9f;
    sf::Vector2f prev = P0;
    for (int i = 1; i <= N; ++i) {
        float t = i * dt;
        float u = 1.0f - t;
        sf::Vector2f curr = (u*u*u)*P0 + (3*u*u*t)*P1 + (3*u*t*t)*P2 + (t*t*t)*P3;

        sf::Vector2f ab = curr - prev;
        sf::Vector2f ap = pt - prev;
        float lenSq = ab.x * ab.x + ab.y * ab.y;
        float segT = (lenSq > 0.0f) ? std::clamp((ap.x * ab.x + ap.y * ab.y) / lenSq, 0.0f, 1.0f) : 0.0f;
        sf::Vector2f proj = prev + segT * ab;
        float d = std::hypot(pt.x - proj.x, pt.y - proj.y);
        if (d < minDist) minDist = d;

        prev = curr;
    }
    return minDist;
}

static void drawDashedSegment(sf::RenderWindow& window, sf::Vector2f p1, sf::Vector2f p2, sf::Color color) {
    sf::Vector2f diff = p2 - p1;
    float len = std::hypot(diff.x, diff.y);
    if (len < 1.0f) return;
    sf::Vector2f dir = diff / len;
    const float dashLen = 6.0f;
    const float gapLen = 6.0f;

    for (float d = 0.0f; d < len; d += dashLen + gapLen) {
        sf::Vector2f start = p1 + dir * d;
        sf::Vector2f end = p1 + dir * std::min(d + dashLen, len);
        sf::Vertex line[] = { sf::Vertex(start, color), sf::Vertex(end, color) };
        window.draw(line, 2, sf::Lines);
    }
}

void BezierEdge::drawHelpers(sf::RenderWindow& window) const {
    sf::Color polyColor(150, 150, 150);
    drawDashedSegment(window, v1->getPosition(), controlPoint1->getPosition(), polyColor);
    drawDashedSegment(window, controlPoint1->getPosition(), controlPoint2->getPosition(), polyColor);
    drawDashedSegment(window, controlPoint2->getPosition(), v2->getPosition(), polyColor);

    sf::CircleShape cpShape(5.0f);
    cpShape.setOrigin(5.0f, 5.0f);
    cpShape.setFillColor(sf::Color(255, 165, 0));
    cpShape.setOutlineThickness(1.0f);
    cpShape.setOutlineColor(sf::Color::Black);

    cpShape.setPosition(controlPoint1->getPosition());
    window.draw(cpShape);

    cpShape.setPosition(controlPoint2->getPosition());
    window.draw(cpShape);
}