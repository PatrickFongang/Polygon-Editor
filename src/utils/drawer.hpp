#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

// abstract class for drawing lines
class Drawer {
public:
    virtual ~Drawer() = default;
    
    virtual void drawLine(float x1, float y1, float x2, float y2) const = 0;

    virtual void setColor(const sf::Color& color) = 0;
};