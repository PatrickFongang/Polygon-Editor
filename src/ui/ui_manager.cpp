#include "ui_manager.hpp"
#include "imgui.h"

UIManager::UIManager() {
    drawingAlgorithms = {
        {"Biblioteczny (SFML)", [](sf::RenderWindow& w) { return std::make_unique<LibraryDrawer>(w); }},
        {"Wlasny (Bresenham)", [](sf::RenderWindow& w) { return std::make_unique<BresenhamDrawer>(w); }}
    };
}

void UIManager::renderUI(Scene& scene, sf::RenderWindow& window, bool& showDocs) {
    ImGui::Begin("Panel Sterowania");

    // choice of drawing algorithm
    ImGui::Text("Algorytm rysowania odcinkow:");
    for (size_t i = 0; i < drawingAlgorithms.size(); ++i) {
        if (ImGui::RadioButton(drawingAlgorithms[i].name.c_str(), &currentDrawerIndex, i)) {
            scene.setDrawer(drawingAlgorithms[i].createDrawer(window));
        }
        if (i < drawingAlgorithms.size() - 1) ImGui::SameLine();
    }

    ImGui::Separator();

    // Scene control buttons
    if (ImGui::Button("Resetuj do sceny poczatkowej")) {
        scene.initPredefinedScene();
    }
    ImGui::SameLine();
    if (ImGui::Button("Dokumentacja (F1)")) {
        showDocs = !showDocs;
    }

    ImGui::Separator();

    ImGui::Text("Edycja wybranego elementu:");
    if (scene.primarySelectedEdge.isValid()) {
        ImGui::Text("Wybrana krawedz: Wielokat #%d, Krawedz #%d",
                    scene.primarySelectedEdge.polygonIdx, scene.primarySelectedEdge.edgeIdx);

        if (ImGui::Button("Dodaj wierzcholek na srodku")) {
        }

        ImGui::Text("Typ krawedzi:");
        if (ImGui::Button("Odcinek")) { /* ... */ }
        ImGui::SameLine();
        if (ImGui::Button("Bezier 3. st.")) { /* ... */ }
        ImGui::SameLine();
        if (ImGui::Button("Luk okregu")) { /* ... */ }

        ImGui::Spacing();
        ImGui::InputFloat("Zadana dlugosc", &targetEdgeLength, 1.0f, 10.0f, "%.1f");
        if (ImGui::Button("Ustaw ograniczenie dlugosci")) {
        }
    } else {
        ImGui::TextDisabled("Kliknij LPM na krawedz lub wierzcholek...");
    }
    ImGui::End();

    if (showDocs) {
        ImGui::SetNextWindowSize(ImVec2(550.0f, 450.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Dokumentacja projektu", &showDocs);

        if (ImGui::CollapsingHeader("1. Instrukcja obslugi", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BulletText("LPM (przeciaganie): Przesuwanie wierzcholka.");
            ImGui::BulletText("F1: Wlaczenie / wylaczenie okna dokumentacji.");
        }
        ImGui::End();
    }
}