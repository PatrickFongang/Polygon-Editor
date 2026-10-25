#pragma once
#include <SFML/Graphics.hpp>
#include "../scene/scene.hpp"

class InputHandler {
private:
    sf::Vector2f lastMousePos;
    bool isDraggingVertex = false;
    bool isDraggingControlPoint = false;
    bool isDraggingPolygon = false;

public:
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window, Scene& scene, bool& showDocs);
};