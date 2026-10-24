#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "imgui.h"
#include "imgui-SFML.h"
#include "ui/scene.hpp"

int main() {
    // 1. Inicjalizacja okna SFML
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    sf::RenderWindow window(sf::VideoMode(1280, 800), "Edytor Wielokatow - Projekt GK", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    // 2. Inicjalizacja ImGui-SFML
    if (!ImGui::SFML::Init(window)) {
        return -1;
    }

    // 3. Stworzenie sceny i załadowanie predefiniowanego układu początkowego
    Scene scene;
    scene.initPredefinedScene();

    sf::Clock deltaClock;

    // 4. Główna pętla programu
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Przekazanie zdarzeń do ImGui
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Blokada przekazywania kliknięć na scenę, jeśli kursor znajduje się nad oknem ImGui
            bool imguiCapturesMouse = ImGui::GetIO().WantCaptureMouse;
            bool imguiCapturesKeyboard = ImGui::GetIO().WantCaptureKeyboard;

            if (!imguiCapturesMouse) {
                scene.handleMouseEvent(event, window);
            }
            if (!imguiCapturesKeyboard) {
                scene.handleKeyboardEvent(event);
            }
        }

        // 5. Aktualizacja stanu ImGui
        ImGui::SFML::Update(window, deltaClock.restart());

        // 6. Rysowanie panelu sterowania i dokumentacji (ImGui)
        scene.renderUI();

        // 7. Renderowanie zawartości okna
        window.clear(sf::Color(30, 30, 35)); // Ciemne, czytelne tło
        
        scene.render(window);                // Rysowanie wielokątów, krzywych i ikon ograniczeń
        ImGui::SFML::Render(window);         // Rysowanie interfejsu na wierzchu

        window.display();
    }

    // 8. Zwolnienie zasobów
    ImGui::SFML::Shutdown();
    return 0;
}