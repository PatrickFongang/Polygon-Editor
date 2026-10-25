#include "library_drawer.hpp"

void LibraryDrawer::drawLine(float x1, float y1, float x2, float y2) const
{
    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(x1, y1)),
        sf::Vertex(sf::Vector2f(x2, y2))};
    window.draw(line, 2, sf::Lines);
}