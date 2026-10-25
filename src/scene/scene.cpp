#include "scene.hpp"
#include "../builder/polygon_director.hpp"

Scene::Scene(std::unique_ptr<Drawer> initialDrawer) 
    : currentDrawer(std::move(initialDrawer)) {}

void Scene::clearScene() {
    polygons.clear();
    selectedPolygonIdx = -1;
    selectedVertexIdx = -1;
    primarySelectedEdge.invalidate();
    secondarySelectedEdge.invalidate();
}

void Scene::initPredefinedScene() {
    clearScene();

    // Create predefined polygons using the PolygonDirector
    polygons.push_back(PolygonDirector::createDemoPolygon1());
    polygons.push_back(PolygonDirector::createDemoPolygon2());
}

void Scene::render(sf::RenderWindow& window) const {
    // draw each polygon using the current drawer
    for (const auto& poly : polygons) {
        for (const auto& edge : poly->getEdges()) {
            if (currentDrawer) {
                edge->draw(*currentDrawer);
            }
        }
    }
}