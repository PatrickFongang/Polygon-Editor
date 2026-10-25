#include "drawer.hpp"

class BresenhamDrawer : public Drawer {
private:
    sf::RenderWindow& window;

    void drawPixel(int x, int y) const;

public:
    explicit BresenhamDrawer(sf::RenderWindow& win) : window(win) {}

    void drawLine(float x1, float y1, float x2, float y2) const override;
};