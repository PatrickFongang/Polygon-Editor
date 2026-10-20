#pragma once

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "polygon.hpp"
// #include "constraint.hpp" // in the future, when constraints are implemented

class Scene
{
    private:
        std::vector<std::unique_ptr<Polygon>> polygons;
        // std::vector<std::unique_ptr<Constraint>> constraints;

        Vertex* selectedVertex = nullptr;
        Edge* selectedEdge = nullptr;
        sf::Vector2f* selectedControlPoint = nullptr; // Pointer to the selected control point of a Bezier edge

        bool useBresenham = false; 

    public:
        Scene();
        ~Scene();


        void loadDefaultScene();

        void draw(sf::RenderWindow& window);

        void handleMousePress(const sf::Vector2f& mousePos, bool rightClick = false);
        void handleMouseDrag(const sf::Vector2f& mousePos);
        void handleMouseRelease();

        void solveConstraints(); 
};