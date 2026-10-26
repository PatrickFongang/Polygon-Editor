#include <SFML/Graphics.hpp>
#include <cmath>
#include "arc_edge.hpp"
#include "../vertex.hpp"


void ArcEdge::draw(const Drawer& drawer) const {
    sf::Vector2f p1(v1->getPosition().x, v1->getPosition().y);
    sf::Vector2f p2(v2->getPosition().x, v2->getPosition().y);

    sf::Vector2f center = (p1 + p2) / 2.0f;

    sf::Vector2f radiusVec = p1 - center;
    
    float radius = std::sqrt(radiusVec.x * radiusVec.x + radiusVec.y * radiusVec.y);

    float startAngle = std::atan2(radiusVec.y, radiusVec.x);
    float endAngle = startAngle + M_PI;

    const int segments = 30; 
    float angleStep = (endAngle - startAngle) / segments;

    sf::Vector2f prevPos = p1;

    for (int i = 1; i <= segments; ++i) {
        float angle = startAngle + i * angleStep;
        
        sf::Vector2f currentPos(
            center.x + radius * std::cos(angle),
            center.y + radius * std::sin(angle)
        );

        drawer.drawLine(prevPos.x, prevPos.y, currentPos.x, currentPos.y);

        prevPos = currentPos;
    }
}

float ArcEdge::distanceToPoint(const sf::Vector2f& pt) const {
    const int N = 100; 
    const float dt = 1.0f / N;
    sf::Vector2f P0 = v1->getPosition();
    sf::Vector2f P1 = v2->getPosition();

    sf::Vector2f center = (P0 + P1) * 0.5f;
    sf::Vector2f radiusVec = P0 - center;
    
    float radius = std::hypot(radiusVec.x, radiusVec.y);
    if (radius == 0.0f) {
        return std::hypot(pt.x - P0.x, pt.y - P0.y);
    }

    float startAngle = std::atan2(radiusVec.y, radiusVec.x);
    float endAngle = startAngle + M_PI;

    float minDist = 1e9f;
    sf::Vector2f prev = P0;
    
    for (int i = 1; i <= N; ++i) {
        float t = i * dt; 
        float angle = startAngle + t * (endAngle - startAngle);
        
        sf::Vector2f curr(
            center.x + radius * std::cos(angle),
            center.y + radius * std::sin(angle)
        );

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

// the arc runs a half turn around the midpoint of the chord: P(t) = C + r (cos(a0 + pi t), sin(a0 + pi t))
// so P'(0) = pi * perp(r) and P'(1) = -pi * perp(r), where r = v1 - C
sf::Vector2f ArcEdge::tangentAt(EdgeEnd end) const {
    sf::Vector2f radiusVec = (v1->getPosition() - v2->getPosition()) * 0.5f;
    sf::Vector2f derivative = static_cast<float>(M_PI) * vec::perp(radiusVec);
    return end == EdgeEnd::Start ? derivative : -derivative;
}
