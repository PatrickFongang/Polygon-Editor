#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include "../scene/scene.hpp"
#include "../utils/drawer.hpp"
#include "../utils/library_drawer.hpp"
#include "../utils/bresenham_drawer.hpp"

struct DrawerOption {
    std::string name;
    std::function<std::unique_ptr<Drawer>(sf::RenderWindow&)> createDrawer;
};

class UIManager {
private:
    std::vector<DrawerOption> drawingAlgorithms;
    int currentDrawerIndex = 0;
    
    float targetEdgeLength = 50.0f;

public:
    UIManager();
    
    void renderUI(Scene& scene, sf::RenderWindow& window, bool& showDocs);
};