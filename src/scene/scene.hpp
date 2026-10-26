#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "../model/polygon.hpp"
#include "../model/constraint/constraint_system.hpp"
#include "../utils/drawer.hpp"

struct EdgeSelection {
    int polygonIdx = -1;
    int edgeIdx = -1;

    bool isValid() const { return polygonIdx != -1 && edgeIdx != -1; }
    void invalidate() { polygonIdx = -1; edgeIdx = -1; }
};

class Scene {
private:
    std::vector<std::unique_ptr<Polygon>> polygons;
    std::unique_ptr<Drawer> currentDrawer;
    ConstraintSystem constraints;

public:
    int selectedPolygonIdx = -1;
    int selectedVertexIdx = -1;
    EdgeSelection primarySelectedEdge;
    EdgeSelection secondarySelectedEdge;

    explicit Scene(std::unique_ptr<Drawer> initialDrawer);

    // the edges are selected by their indices, which stop being valid when the topology changes
    void clearEdgeSelection() {
        primarySelectedEdge.invalidate();
        secondarySelectedEdge.invalidate();
    }

    void clearScene();
    void initPredefinedScene();
    void render(sf::RenderWindow& window) const;

    void setDrawer(std::unique_ptr<Drawer> newDrawer) {
        currentDrawer = std::move(newDrawer);
    }

    std::vector<std::unique_ptr<Polygon>>& getPolygons() { return polygons; }
    ConstraintSystem& getConstraints() { return constraints; }
};