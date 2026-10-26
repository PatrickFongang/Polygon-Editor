#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include "scene/scene.hpp"
#include "ui/ui_manager.hpp"
#include "ui/input_handler.hpp"
#include "editing/polygon_editor.hpp"
#include "editing/continuity_enforcer.hpp"

int main() {
    // initialize SFML window
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    sf::RenderWindow window(sf::VideoMode(1280, 800), "Edytor Wielokatow - Projekt GK", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    // initialize ImGui-SFML
    if (!ImGui::SFML::Init(window)) {
        return -1;
    }

    // initialize the scene with a default drawer
    auto initialDrawer = std::make_unique<LibraryDrawer>(window);
    Scene scene(std::move(initialDrawer));
    scene.initPredefinedScene();

    // edits go through the editor, which restores the continuity at the vertices after every change
    PolygonEditor editor;
    editor.addRule(std::make_unique<ContinuityEnforcer>());
    for (auto& polygon : scene.getPolygons()) {
        editor.enforceAll(*polygon);
    }

    UIManager uiManager;
    InputHandler inputHandler(editor);
    
    bool showDocs = false; 

    sf::Clock deltaClock;

    // main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }

            bool imguiCapturesMouse = ImGui::GetIO().WantCaptureMouse;
            bool imguiCapturesKeyboard = ImGui::GetIO().WantCaptureKeyboard;

            if (!imguiCapturesMouse && !imguiCapturesKeyboard) {
                inputHandler.handleEvent(event, window, scene, showDocs);
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        uiManager.renderUI(scene, window, showDocs);

        window.clear(sf::Color(30, 30, 35)); 
        
        scene.render(window);                
        ImGui::SFML::Render(window);         

        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}