#include <SFML/Graphics.hpp>
#include "bezier_edge.hpp"
#include "../vertex.hpp"

void BezierEdge::draw() const {
    const int N = 50; 
    const float dt = 1.0f / N;

    sf::Vector2f P0(v1->getPosition().x, v1->getPosition().y);
    sf::Vector2f P1(controlPoint1->getPosition().x, controlPoint1->getPosition().y);
    sf::Vector2f P2(controlPoint2->getPosition().x, controlPoint2->getPosition().y);
    sf::Vector2f P3(v2->getPosition().x, v2->getPosition().y);

    // calculate the coefficients for the cubic Bezier curve
    sf::Vector2f a = -P0 + 3.0f * P1 - 3.0f * P2 + P3;
    sf::Vector2f b =  3.0f * P0 - 6.0f * P1 + 3.0f * P2;
    sf::Vector2f c = -3.0f * P0 + 3.0f * P1;
    sf::Vector2f d =  P0;

    // incremental drawing using forward differences
    float dt2 = dt * dt;
    float dt3 = dt2 * dt;

    sf::Vector2f currentPos = d;

    sf::Vector2f delta1 = a * dt3 + b * dt2 + c * dt;
    sf::Vector2f delta2 = 6.0f * a * dt3 + 2.0f * b * dt2;
    sf::Vector2f delta3 = 6.0f * a * dt3;

    sf::Vector2f prevPos = currentPos;

    for (int i = 1; i <= N; ++i) {
        currentPos += delta1;
        delta1 += delta2;
        delta2 += delta3;

        // Draw line segment from prevPos to currentPos
        
        prevPos = currentPos;
    }
}