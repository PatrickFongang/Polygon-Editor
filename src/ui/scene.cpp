#include "scene.hpp"



void Scene::loadDefaultScene(){

}

void Scene::draw(sf::RenderWindow& window){
    while (window.isOpen())
{
   // Event processing
   while (const std::optional event = window.pollEvent())
   {
       // Request for closing the window
       if (event->is<sf::Event::Closed>())
           window.close();
   }
 
   // Clear the whole window before rendering a new frame
   window.clear();
 
   // End the current frame and display its contents on screen
   window.display();
}
}

void Scene::handleMousePress(const sf::Vector2f& mousePos, bool rightClick = false){

}
void Scene::handleMouseDrag(const sf::Vector2f& mousePos){

}
void Scene::handleMouseRelease(){

}