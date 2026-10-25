#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../model/polygon.hpp"
#include "../enum/continuity.hpp"


class Scene {
public:
    Scene();
    ~Scene() = default;

    // Inicjalizacja sceny początkowej (Wymaganie 8)
    void initPredefinedScene();
    void clearScene();

    // Obsługa wejścia (Wymaganie 6)
    void handleMouseEvent(const sf::Event& event, const sf::RenderWindow& window);
    void handleKeyboardEvent(const sf::Event& event);

    // Renderowanie (Wymagania 5, 7, 9)
    void render(sf::RenderWindow& window);
    void renderUI(sf::RenderWindow& window);

private:
    std::vector<std::unique_ptr<Polygon>> polygons;

    // Current drawing stategy (Library or Bresenham)
    std::unique_ptr<Drawer> currentDrawer; 


    // Stan interakcji myszy (przeciąganie LPM)
    bool isDraggingVertex = false;
    bool isDraggingControlPoint = false;
    bool isDraggingPolygon = false;
    sf::Vector2f lastMousePos;

    // Wskaźniki/indeksy aktualnie wybranych elementów (do edycji i relacji)
    int selectedPolygonIdx = -1;
    int selectedVertexIdx = -1;
    int selectedControlPointIdx = -1; // 0 lub 1 dla wybranej krawędzi Béziera
    
    // Para krawędzi do nakładania ograniczeń (np. równoległość / prostopadłość)
    struct EdgeRef {
        int polygonIdx = -1;
        int edgeIdx = -1;
        bool isValid() const { return polygonIdx >= 0 && edgeIdx >= 0; }
    };
    EdgeRef primarySelectedEdge;
    EdgeRef secondarySelectedEdge;

    // Zmienne pomocnicze dla UI
    float targetEdgeLength = 100.0f;
    bool showDocumentationWindow = false;
};