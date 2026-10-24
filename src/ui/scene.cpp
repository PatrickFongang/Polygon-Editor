#include "scene.hpp"
#include "imgui.h"
#include <cmath>
#include <iostream>

Scene::Scene() {}

void Scene::clearScene() {
    polygons.clear();
    selectedPolygonIdx = -1;
    selectedVertexIdx = -1;
    primarySelectedEdge = {-1, -1};
    secondarySelectedEdge = {-1, -1};
}

// Adds polygon to the scene and returns a pointer to it. The polygon is constructed from a list of points.
Polygon* Scene::addPolygon(const std::vector<sf::Vector2f>& points) {
    auto poly_ptr = std::make_unique<Polygon>();
    for (const auto& pt : points) {
        poly_ptr->addVertex(pt.x, pt.y); 
    }
    polygons.push_back(std::move(poly_ptr));
    return polygons.back().get(); 
}

// Predefined Scene
void Scene::initPredefinedScene() {
    clearScene();

    Polygon* poly1 = addPolygon({
        {150.f, 200.f}, 
        {400.f, 200.f}, 
        {400.f, 450.f}, 
        {150.f, 450.f}  
    });

    Continuity defaultCont = Continuity::G1;



    Polygon* poly2 = addPolygon({
        {600.f, 200.f}, 
        {800.f, 150.f}, 
        {900.f, 300.f}, 
        {800.f, 500.f}, 
        {600.f, 450.f}  
    });

}

void Scene::handleMouseEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        lastMousePos = mousePos;
        // Logic to determine if a vertex, control point, or polygon is selected and set the appropriate flags
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        isDraggingVertex = false;
        isDraggingControlPoint = false;
        isDraggingPolygon = false;
    }

    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f delta = mousePos - lastMousePos;

        if (isDraggingVertex && selectedPolygonIdx != -1 && selectedVertexIdx != -1) {
            // logic to move the selected vertex to the new mouse position
        } 
        else if (isDraggingControlPoint && selectedPolygonIdx != -1) {
            // logic to move the selected control point to the new mouse position
        } 
        else if (isDraggingPolygon && selectedPolygonIdx != -1) {
            // logic to move the entire selected polygon by delta
        }

        lastMousePos = mousePos;
    }
}

void Scene::handleKeyboardEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F1) {
            showDocumentationWindow = !showDocumentationWindow;
        }
    }
}

// Drawing Algorithms
void Scene::drawLineBresenham(sf::RenderTarget& target, sf::Vector2f p1, sf::Vector2f p2, sf::Color color) {
    int x0 = static_cast<int>(std::round(p1.x));
    int y0 = static_cast<int>(std::round(p1.y));
    int x1 = static_cast<int>(std::round(p2.x));
    int y1 = static_cast<int>(std::round(p2.y));

    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    sf::VertexArray pixels(sf::Points);

    while (true) {
        pixels.append(sf::Vertex(sf::Vector2f(static_cast<float>(x0), static_cast<float>(y0)), color));
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
    target.draw(pixels);
}

void Scene::drawBezierIncremental(sf::RenderTarget& target, sf::Vector2f V0, sf::Vector2f V1, sf::Vector2f V2, sf::Vector2f V3, sf::Color color) {
    const int steps = 60;
    const float dt = 1.0f / steps;
    const float dt2 = dt * dt;
    const float dt3 = dt2 * dt;

    sf::Vector2f A = -V0 + 3.f * V1 - 3.f * V2 + V3;
    sf::Vector2f B = 3.f * V0 - 6.f * V1 + 3.f * V2;
    sf::Vector2f C = -3.f * V0 + 3.f * V1;
    sf::Vector2f D = V0;

    sf::Vector2f P = D;
    sf::Vector2f dP = A * dt3 + B * dt2 + C * dt;
    sf::Vector2f ddP = 6.f * A * dt3 + 2.f * B * dt2;
    sf::Vector2f dddP = 6.f * A * dt3;

    sf::VertexArray curve(sf::LineStrip, steps + 1);
    curve[0] = sf::Vertex(P, color);

    for (int i = 1; i <= steps; ++i) {
        P += dP;
        dP += ddP;
        ddP += dddP;
        curve[i] = sf::Vertex(P, color);
    }

    target.draw(curve);
}

void Scene::render(sf::RenderWindow& window) {

}


// Draw the ImGui UI for the scene
void Scene::renderUI() {
    ImGui::Begin("Panel Sterowania");

    // Drawing algorithm selection
    ImGui::Text("Algorytm rysowania odcinkow:");
    int algo = static_cast<int>(currentAlgorithm);
    if (ImGui::RadioButton("Biblioteczny (SFML)", &algo, 0)) {
        currentAlgorithm = DrawAlgorithm::Library;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Wlasny (Bresenham)", &algo, 1)) {
        currentAlgorithm = DrawAlgorithm::Bresenham;
    }

    ImGui::Separator();

    // Scene control buttons
    if (ImGui::Button("Resetuj do sceny poczatkowej")) {
        initPredefinedScene();
    }
    ImGui::SameLine();
    if (ImGui::Button("Dokumentacja (F1)")) {
        showDocumentationWindow = !showDocumentationWindow;
    }

    ImGui::Separator();

    // 4. Selected element editing panel
    ImGui::Text("Edycja wybranego elementu:");
    if (primarySelectedEdge.isValid()) {
        ImGui::Text("Wybrana krawedz: Wielokat #%d, Krawedz #%d", 
                    primarySelectedEdge.polygonIdx, primarySelectedEdge.edgeIdx);

        if (ImGui::Button("Dodaj wierzcholek na srodku")) {
            // polygons[primarySelectedEdge.polygonIdx].splitEdge(primarySelectedEdge.edgeIdx);
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
            // ConstraintManager::getInstance().setLengthConstraint(primarySelectedEdge, targetEdgeLength);
        }
    } else {
        ImGui::TextDisabled("Kliknij LPM na krawedz lub wierzcholek...");
    }

    ImGui::End();

    // Documentation window
    if (showDocumentationWindow) {

        ImGui::SetNextWindowSize(ImVec2(550.0f, 450.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Dokumentacja projektu", &showDocumentationWindow);
        
        if (ImGui::CollapsingHeader("1. Instrukcja obslugi i klawiszologia", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BulletText("LPM (przeciaganie na wierzcholku/punkcie kontrolnym): Przesuwanie elementu.");
            ImGui::BulletText("Shift + LPM (przeciaganie): Przesuwanie calego wielokata.");
            ImGui::BulletText("LPM (klikniecie na krawedz): Wybor krawedzi Glównej.");
            ImGui::BulletText("Ctrl + LPM (klikniecie na druga krawedz): Wybor krawedzi Drugorzednej (do relacji).");
            ImGui::BulletText("F1: Wlaczenie / wylaczenie okna dokumentacji.");
        }

        if (ImGui::CollapsingHeader("2. Przyjete zalozenia")) {
            ImGui::TextWrapped(
                "- Podczas przesuwania wierzcholka priorytet ma pozycja kursora myszy.\n"
                "- Dodanie lub usuniecie wierzcholka automatycznie czysci ograniczenia sasiadujacych krawedzi.\n"
                "- Dla luku okregu z ciagloscia G0-G0 promien wynosi dokladnie polowe dlugosci cieciwy."
            );
        }

        if (ImGui::CollapsingHeader("3. Model danych")) {
            ImGui::TextWrapped(
                "Scena sklada sie z listy obiektow Polygon. Kazdy Polygon przechowuje cykliczna liste "
                "wierzcholkow (Vertex z klasa ciaglosci G0/G1/C1) oraz krawedzi (Edge: Line, Bezier, Arc). "
                "Relacje miedzy krawedziami przechowywane sa w grafie ograniczen."
            );
        }

        if (ImGui::CollapsingHeader("4. Zaimplementowane algorytmy")) {
            ImGui::BulletText("Rysowanie odcinkow: Algorytm Bresenhama dla wszystkich 8 oktantow.");
            ImGui::BulletText("Krzywe Beziera: Algorytm przyrostowy (roznice progresywne 3. rzedu w bazie potegowej).");
            ImGui::BulletText("Rozwiazywanie ograniczen: Algorytm propagacji BFS/iteracyjny od poruszanego wierzcholka.");
        }

        ImGui::End();
    }
}