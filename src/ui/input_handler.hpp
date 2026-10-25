#pragma once
#include <SFML/Graphics.hpp>
#include "../scene/scene.hpp"

class InputHandler {
private:
    sf::Vector2f lastMousePos;
    bool isDraggingVertex = false;
    bool isDraggingControlPoint = false;
    bool isDraggingPolygon = false;

    Vertex* draggedControlPoint = nullptr;

    static constexpr float HIT_RADIUS = 8.0f;
    static float distance(const sf::Vector2f& a, const sf::Vector2f& b);

public:
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window, Scene& scene, bool& showDocs);
};