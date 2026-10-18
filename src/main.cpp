#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

int main() {
    // W SFML 3 wymiary podajemy jako wektor w klamrach: {800, 600}
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Edytor Wielokatow - Test ImGui");
    window.setFramerateLimit(60);

    // Inicjalizacja ImGui z SFML
    if (!ImGui::SFML::Init(window)) {
        return -1;
    }

    sf::Clock deltaClock;
    while (window.isOpen()) {
        // W SFML 3 pollEvent() zwraca std::optional<sf::Event>
        while (const std::optional event = window.pollEvent()) {
            // Przekazywanie zdarzeń do ImGui (z dereferencją *event)
            ImGui::SFML::ProcessEvent(window, *event);

            // Sprawdzanie typu zdarzenia za pomocą metody is<T>()
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Aktualizacja ImGui
        sf::Time dt = deltaClock.restart();
        ImGui::SFML::Update(window, dt);

        // --- TUTAJ RYSUJESZ SWÓJ INTERFEJS ---
        ImGui::Begin("Panel Sterowania");
        ImGui::Text("Witaj w projekcie 1!");
        if (ImGui::Button("Kliknij mnie")) {
            // Reakcja na przycisk
        }
        ImGui::End();
        // -------------------------------------

        window.clear(sf::Color::Black);
        
        // --- TUTAJ BĘDZIESZ RYSOWAĆ SWOJE WIELOKĄTY (Bresenham/SFML) ---
        
        // Rysowanie UI na samej górze
        ImGui::SFML::Render(window);
        window.display();
    }

    // Sprzątanie po zamknięciu
    ImGui::SFML::Shutdown();
    return 0;
}