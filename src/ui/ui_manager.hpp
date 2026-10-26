#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include "../scene/scene.hpp"
#include "../editing/polygon_editor.hpp"
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

    PolygonEditor& editor;

    // writes the continuity class next to every vertex where it matters
    void drawContinuityLabels(Scene& scene, sf::RenderWindow& window);

public:
    explicit UIManager(PolygonEditor& polygonEditor);
    
    void renderUI(Scene& scene, sf::RenderWindow& window, bool& showDocs);
};