#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "ui/scene.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Edytor Wielokatow - Test ImGui");
    window.setFramerateLimit(60);

    Scene* scene = new Scene();
    scene->draw(window);

    //delete scene;
    return 0;
}