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