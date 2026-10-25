#include "input_handler.hpp"

void InputHandler::handleEvent(const sf::Event& event, const sf::RenderWindow& window, Scene& scene, bool& showDocs) {
    
    // keyboard event handling
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F1) {
            showDocs = !showDocs;
        }
    }

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    // mouse button pressed
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        lastMousePos = mousePos;
        
        
    }

    // mouse button released
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        isDraggingVertex = false;
        isDraggingControlPoint = false;
        isDraggingPolygon = false;
    }

    // mouse moved
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f delta = mousePos - lastMousePos;

        if (isDraggingVertex && scene.selectedPolygonIdx != -1 && scene.selectedVertexIdx != -1) {
            
        }
        else if (isDraggingPolygon && scene.selectedPolygonIdx != -1) {
        }

        lastMousePos = mousePos;
    }
}