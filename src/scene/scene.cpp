#include "scene.hpp"
#include "../builder/polygon_director.hpp"
#include "../model/constraint/length_constraint.hpp"
#include "../model/constraint/parallel_constraint.hpp"

Scene::Scene(std::unique_ptr<Drawer> initialDrawer) 
    : currentDrawer(std::move(initialDrawer)) {}

void Scene::clearScene() {
    // the constraints point at the edges of the polygons, so they go first
    constraints.clear();
    polygons.clear();
    selectedPolygonIdx = -1;
    selectedVertexIdx = -1;
    primarySelectedEdge.invalidate();
    secondarySelectedEdge.invalidate();
}

void Scene::initPredefinedScene() {
    clearScene();

    // Polygons: one line for each, built by the PolygonDirector.
    Polygon& triangle = addPolygon(PolygonDirector::createDemoPolygon1()); // a Bezier edge (0), segments (1, 2)
    Polygon& diamond = addPolygon(PolygonDirector::createDemoPolygon2());  // an arc (0), segments (1, 2, 3)

    // Constraints: edges are named by the polygon and the index of the edge (the edge i leads from vertex i to
    // the next one). They only apply to segments. The positions above need not satisfy them: the solver
    // brings the scene in line when it starts and when it is reset.

    // two constraints on one edge: the length and the parallelism to an edge of the other polygon
    constraints.add(std::make_unique<LengthConstraint>(triangle.getEdge(1), 150.0f));
    constraints.add(std::make_unique<ParallelConstraint>(triangle.getEdge(1), diamond.getEdge(3)));

    // a relation between the polygons, and a length of the edge of the second one
    constraints.add(std::make_unique<ParallelConstraint>(triangle.getEdge(2), diamond.getEdge(2)));
    constraints.add(std::make_unique<LengthConstraint>(diamond.getEdge(2), 120.0f));
}

void Scene::render(sf::RenderWindow& window) const {
    for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx) {
        const auto& poly = polygons[pIdx];

        // draw edges with highlighting for selected edges
        const auto& edges = poly->getEdges();
        for (size_t eIdx = 0; eIdx < edges.size(); ++eIdx) {
            const auto& edge = edges[eIdx];

            bool isPrimary = (primarySelectedEdge.isValid() && 
                              primarySelectedEdge.polygonIdx == static_cast<int>(pIdx) && 
                              primarySelectedEdge.edgeIdx == static_cast<int>(eIdx));
                              
            bool isSecondary = (secondarySelectedEdge.isValid() && 
                                secondarySelectedEdge.polygonIdx == static_cast<int>(pIdx) && 
                                secondarySelectedEdge.edgeIdx == static_cast<int>(eIdx));

            sf::Color edgeColor = sf::Color::White; 
            if (isPrimary) {
                edgeColor = sf::Color::Yellow;  
            } else if (isSecondary) {
                edgeColor = sf::Color::Magenta; 
            }

            if (currentDrawer) {
                currentDrawer->setColor(edgeColor); 
                edge->draw(*currentDrawer);
            }
            
            edge->drawHelpers(window);
        }

        // draw vertices with highlighting for selected vertices
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