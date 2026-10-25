#include "scene.hpp"
#include "imgui.h"
#include <cmath>
#include <iostream>
#include "../builder/polygon_director.hpp"
#include "../utils/library_drawer.hpp"
#include "../utils/bresenham_drawer.hpp"

Scene::Scene() {}

void Scene::clearScene()
{
    polygons.clear();
    selectedPolygonIdx = -1;
    selectedVertexIdx = -1;
    primarySelectedEdge = {-1, -1};
    secondarySelectedEdge = {-1, -1};
}

// Predefined Scene
void Scene::initPredefinedScene()
{
    clearScene();

    PolygonDirector director;

    polygons.push_back(director.createDemoPolygon1());
    polygons.push_back(director.createDemoPolygon2());
}

void Scene::handleMouseEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        lastMousePos = mousePos;
        // Logic to determine if a vertex, control point, or polygon is selected and set the appropriate flags
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        isDraggingVertex = false;
        isDraggingControlPoint = false;
        isDraggingPolygon = false;
    }

    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f delta = mousePos - lastMousePos;

        if (isDraggingVertex && selectedPolygonIdx != -1 && selectedVertexIdx != -1)
        {
            // logic to move the selected vertex to the new mouse position
        }
        else if (isDraggingControlPoint && selectedPolygonIdx != -1)
        {
            // logic to move the selected control point to the new mouse position
        }
        else if (isDraggingPolygon && selectedPolygonIdx != -1)
        {
            // logic to move the entire selected polygon by delta
        }

        lastMousePos = mousePos;
    }
}

void Scene::handleKeyboardEvent(const sf::Event &event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::F1)
        {
            showDocumentationWindow = !showDocumentationWindow;
        }
    }
}

void Scene::render(sf::RenderWindow &window)
{
    if (!currentDrawer) {
        currentDrawer = std::make_unique<LibraryDrawer>(window);
    }
    
    for (const auto &poly : polygons)
    {
        for (const auto &edge : poly->getEdges())
        {
            edge->draw(*currentDrawer);
        }
    }
}

// Draw the ImGui UI for the scene
void Scene::renderUI(sf::RenderWindow &window)
{
    ImGui::Begin("Panel Sterowania");

    // Drawing algorithm selection
    ImGui::Text("Algorytm rysowania odcinkow:");
    int algo = 0;
    if (ImGui::RadioButton("Biblioteczny (SFML)", &algo, 1))
    {
        currentDrawer = std::make_unique<LibraryDrawer>(window);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Wlasny (Bresenham)", &algo, 1))
    {
        currentDrawer = std::make_unique<BresenhamDrawer>(window);
    }

    ImGui::Separator();

    // Scene control buttons
    if (ImGui::Button("Resetuj do sceny poczatkowej"))
    {
        initPredefinedScene();
    }
    ImGui::SameLine();
    if (ImGui::Button("Dokumentacja (F1)"))
    {
        showDocumentationWindow = !showDocumentationWindow;
    }

    ImGui::Separator();

    // 4. Selected element editing panel
    ImGui::Text("Edycja wybranego elementu:");
    if (primarySelectedEdge.isValid())
    {
        ImGui::Text("Wybrana krawedz: Wielokat #%d, Krawedz #%d",
                    primarySelectedEdge.polygonIdx, primarySelectedEdge.edgeIdx);

        if (ImGui::Button("Dodaj wierzcholek na srodku"))
        {
            // polygons[primarySelectedEdge.polygonIdx].splitEdge(primarySelectedEdge.edgeIdx);
        }

        ImGui::Text("Typ krawedzi:");
        if (ImGui::Button("Odcinek"))
        { /* ... */
        }
        ImGui::SameLine();
        if (ImGui::Button("Bezier 3. st."))
        { /* ... */
        }
        ImGui::SameLine();
        if (ImGui::Button("Luk okregu"))
        { /* ... */
        }

        ImGui::Spacing();
        ImGui::InputFloat("Zadana dlugosc", &targetEdgeLength, 1.0f, 10.0f, "%.1f");
        if (ImGui::Button("Ustaw ograniczenie dlugosci"))
        {
            // ConstraintManager::getInstance().setLengthConstraint(primarySelectedEdge, targetEdgeLength);
        }
    }
    else
    {
        ImGui::TextDisabled("Kliknij LPM na krawedz lub wierzcholek...");
    }

    ImGui::End();

    // Documentation window
    if (showDocumentationWindow)
    {

        ImGui::SetNextWindowSize(ImVec2(550.0f, 450.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Dokumentacja projektu", &showDocumentationWindow);

        if (ImGui::CollapsingHeader("1. Instrukcja obslugi i klawiszologia", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BulletText("LPM (przeciaganie na wierzcholku/punkcie kontrolnym): Przesuwanie elementu.");
            ImGui::BulletText("Shift + LPM (przeciaganie): Przesuwanie calego wielokata.");
            ImGui::BulletText("LPM (klikniecie na krawedz): Wybor krawedzi Glównej.");
            ImGui::BulletText("Ctrl + LPM (klikniecie na druga krawedz): Wybor krawedzi Drugorzednej (do relacji).");
            ImGui::BulletText("F1: Wlaczenie / wylaczenie okna dokumentacji.");
        }

        if (ImGui::CollapsingHeader("2. Przyjete zalozenia"))
        {
            ImGui::TextWrapped(
                "- Podczas przesuwania wierzcholka priorytet ma pozycja kursora myszy.\n"
                "- Dodanie lub usuniecie wierzcholka automatycznie czysci ograniczenia sasiadujacych krawedzi.\n"
                "- Dla luku okregu z ciagloscia G0-G0 promien wynosi dokladnie polowe dlugosci cieciwy.");
        }

        if (ImGui::CollapsingHeader("3. Model danych"))
        {
            ImGui::TextWrapped(
                "Scena sklada sie z listy obiektow Polygon. Kazdy Polygon przechowuje cykliczna liste "
                "wierzcholkow (Vertex z klasa ciaglosci G0/G1/C1) oraz krawedzi (Edge: Line, Bezier, Arc). "
                "Relacje miedzy krawedziami przechowywane sa w grafie ograniczen.");
        }

        if (ImGui::CollapsingHeader("4. Zaimplementowane algorytmy"))
        {
            ImGui::BulletText("Rysowanie odcinkow: Algorytm Bresenhama dla wszystkich 8 oktantow.");
            ImGui::BulletText("Krzywe Beziera: Algorytm przyrostowy (roznice progresywne 3. rzedu w bazie potegowej).");
            ImGui::BulletText("Rozwiazywanie ograniczen: Algorytm propagacji BFS/iteracyjny od poruszanego wierzcholka.");
        }

        ImGui::End();
    }
}