#include "bresenham_drawer.hpp"

void BresenhamDrawer::drawPixel(int x, int y) const
{
    sf::Vertex pixel(sf::Vector2f(x, y), currentColor);
    window.draw(&pixel, 1, sf::Points);
}

void BresenhamDrawer::drawLine(float x1, float y1, float x2, float y2) const
{
    int ix1 = static_cast<int>(x1);
    int iy1 = static_cast<int>(y1);
    int ix2 = static_cast<int>(x2);
    int iy2 = static_cast<int>(y2);

    int dx = std::abs(ix2 - ix1);
    int dy = std::abs(iy2 - iy1);
    int sx = (ix1 < ix2) ? 1 : -1;
    int sy = (iy1 < iy2) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        drawPixel(ix1, iy1);
        if (ix1 == ix2 && iy1 == iy2)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            ix1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            iy1 += sy;
        }
    }
}