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

    // length typed into the field of the panel, for the edge it was prepared for
    const Edge* lengthInputEdge = nullptr;
    float lengthInput = 0.0f;

    // writes the continuity class next to every vertex where it matters
    void drawContinuityLabels(Scene& scene, sf::RenderWindow& window);

    // marks every constrained edge with the labels of its constraints, near the middle of the edge
    void drawConstraintLabels(Scene& scene, sf::RenderWindow& window);

    // adding and removing the constraints of the selected edges
    void drawConstraintPanel(Scene& scene);

public:
    explicit UIManager(PolygonEditor& polygonEditor);
    
    void renderUI(Scene& scene, sf::RenderWindow& window, bool& showDocs);
};