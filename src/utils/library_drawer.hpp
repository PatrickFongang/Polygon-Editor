#include "drawer.hpp"


class LibraryDrawer : public Drawer {
private:
    sf::RenderWindow& window;

public:
    explicit LibraryDrawer(sf::RenderWindow& win) : window(win) {}

    void drawLine(float x1, float y1, float x2, float y2) const override;
};