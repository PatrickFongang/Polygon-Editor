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
    for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx) {
        const auto& poly = polygons[pIdx];

        // draw edges and their helpers
        for (const auto& edge : poly->getEdges()) {
            edge->drawHelpers(window);
            if (currentDrawer) {
                edge->draw(*currentDrawer);
            }
        }

        // draw vertices
        const auto& vertices = poly->getVertices();
        for (size_t vIdx = 0; vIdx < vertices.size(); ++vIdx) {
            bool isSelected = (static_cast<int>(pIdx) == selectedPolygonIdx &&
                               static_cast<int>(vIdx) == selectedVertexIdx);

            float radius = isSelected ? 7.0f : 5.0f;
            sf::CircleShape circle(radius);
            circle.setOrigin(radius, radius);
            circle.setPosition(vertices[vIdx]->getPosition());
            circle.setFillColor(isSelected ? sf::Color(80, 220, 100) : sf::Color::Cyan);
            circle.setOutlineThickness(1.5f);
            circle.setOutlineColor(sf::Color::White);

            window.draw(circle);
        }
    }
}