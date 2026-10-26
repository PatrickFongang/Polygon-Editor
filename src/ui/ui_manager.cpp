#include "ui_manager.hpp"
#include "imgui.h"
#include "../model/continuity_policy.hpp"

static const char *continuityName(Continuity continuity)
{
    switch (continuity)
    {
    case Continuity::G0: return "G0";
    case Continuity::G1: return "G1";
    case Continuity::C1: return "C1";
    }
    return "";
}

UIManager::UIManager(PolygonEditor &polygonEditor) : editor(polygonEditor)
{
    drawingAlgorithms = {
        {"Biblioteczny (SFML)", [](sf::RenderWindow &w)
         { return std::make_unique<LibraryDrawer>(w); }},
        {"Wlasny (Bresenham)", [](sf::RenderWindow &w)
         { return std::make_unique<BresenhamDrawer>(w); }}};
}

void UIManager::renderUI(Scene &scene, sf::RenderWindow &window, bool &showDocs)
{
    ImGui::Begin("Panel Sterowania");

    // choice of drawing algorithm
    ImGui::Text("Algorytm rysowania odcinkow:");
    for (size_t i = 0; i < drawingAlgorithms.size(); ++i)
    {
        if (ImGui::RadioButton(drawingAlgorithms[i].name.c_str(), &currentDrawerIndex, i))
        {
            scene.setDrawer(drawingAlgorithms[i].createDrawer(window));
        }
        if (i < drawingAlgorithms.size() - 1)
            ImGui::SameLine();
    }

    ImGui::Separator();

    // Scene control buttons
    if (ImGui::Button("Resetuj do sceny poczatkowej"))
    {
        scene.initPredefinedScene();
        for (auto &polygon : scene.getPolygons())
        {
            editor.enforceAll(*polygon);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Dokumentacja (F1)"))
    {
        showDocs = !showDocs;
    }

    ImGui::Separator();

    ImGui::Text("Edycja wybranego elementu:");
    if (scene.primarySelectedEdge.isValid())
    {
        ImGui::Text("Wybrana krawedz: Wielokat #%d, Krawedz #%d",
                    scene.primarySelectedEdge.polygonIdx, scene.primarySelectedEdge.edgeIdx);

        Polygon *poly = scene.getPolygons()[scene.primarySelectedEdge.polygonIdx].get();
        int eIdx = scene.primarySelectedEdge.edgeIdx;
        Edge *edge = poly->getEdge(eIdx);

        if (ImGui::Button("Dodaj wierzcholek na srodku"))
        {
            editor.splitEdgeAtMidpoint(*poly, eIdx);
            scene.primarySelectedEdge.invalidate();
        }

        if (edge)
        {
            ImGui::Text("Typ krawedzi:");
            if (ImGui::Button("Odcinek"))
            {
                editor.toggleEdgeType(*poly, eIdx, EdgeType::Line);
            }
            ImGui::SameLine();
            if (ImGui::Button("Bezier 3. st."))
            {
                editor.toggleEdgeType(*poly, eIdx, EdgeType::Bezier);
            }
            ImGui::SameLine();
            if (ImGui::Button("Luk okregu"))
            {
                editor.toggleEdgeType(*poly, eIdx, EdgeType::Arc);
            }
        }
    }
    else
    {
        ImGui::TextDisabled("Kliknij LPM na krawedz lub wierzcholek...");
    }
    if (scene.selectedPolygonIdx != -1 && scene.selectedVertexIdx != -1)
    {
        Polygon *poly = scene.getPolygons()[scene.selectedPolygonIdx].get();
        Vertex *v = poly->getVertex(scene.selectedVertexIdx);
        if (v)
        {
            ImGui::Separator();
            ImGui::Text("Wybrany wierzcholek: Wielokat #%d, Wierzcholek #%d",
                        scene.selectedPolygonIdx, scene.selectedVertexIdx);
            ImGui::Text("Ciaglosc w wierzcholku:");
            std::vector<Continuity> allowed = ContinuityPolicy::allowed(*v);
            if (allowed.size() <= 1)
            {
                ImGui::TextDisabled("G0 (brak krzywej przy wierzcholku)");
            }
            else
            {
                Continuity current = ContinuityPolicy::effective(*v);
                for (size_t i = 0; i < allowed.size(); ++i)
                {
                    if (ImGui::RadioButton(continuityName(allowed[i]), current == allowed[i]))
                    {
                        editor.setContinuity(*v, allowed[i]);
                    }
                    if (i + 1 < allowed.size())
                        ImGui::SameLine();
                }
                if (!v->hasExplicitContinuity)
                    ImGui::TextDisabled("(domyslna)");
            }

            if (ImGui::Button("Usun wierzcholek"))
            {
                editor.removeVertex(*poly, scene.selectedVertexIdx);
                scene.selectedVertexIdx = -1;
            }
        }
    }
    ImGui::End();

    if (showDocs)
    {
        ImGui::SetNextWindowSize(ImVec2(550.0f, 450.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Dokumentacja projektu", &showDocs);

        if (ImGui::CollapsingHeader("1. Instrukcja obslugi", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BulletText("LPM (przeciaganie): Przesuwanie wierzcholka.");
            ImGui::BulletText("Wybrany wierzcholek: w panelu mozna wybrac ciaglosc G0 / G1 / C1 (tylko dozwolone).");
            ImGui::BulletText("F1: Wlaczenie / wylaczenie okna dokumentacji.");
        }
        ImGui::End();
    }
}