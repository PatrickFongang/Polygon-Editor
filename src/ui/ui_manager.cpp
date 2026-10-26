#include "ui_manager.hpp"
#include "imgui.h"
#include "documentation.hpp"
#include "../model/continuity_policy.hpp"
#include "../model/constraint/constraint_system.hpp"
#include "../model/constraint/length_constraint.hpp"
#include <algorithm>
#include <string>

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

void UIManager::drawContinuityLabels(Scene &scene, sf::RenderWindow &window)
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();
    for (auto &polygon : scene.getPolygons())
    {
        for (const auto &vertex : polygon->getVertices())
        {
            if (!ContinuityPolicy::matters(*vertex))
                continue;

            sf::Vector2i pixel = window.mapCoordsToPixel(vertex->getPosition());
            const char *name = continuityName(ContinuityPolicy::effective(*vertex));
            drawList->AddText(ImVec2(pixel.x + 9.0f, pixel.y - 20.0f), IM_COL32(255, 220, 120, 255), name);
        }
    }
}

// "W#1 K#2": the polygon and the edge, as in the panel
static std::string edgeName(Scene &scene, const Edge *edge)
{
    auto &polygons = scene.getPolygons();
    for (size_t p = 0; p < polygons.size(); ++p)
    {
        const auto &edges = polygons[p]->getEdges();
        for (size_t e = 0; e < edges.size(); ++e)
        {
            if (edges[e].get() == edge)
                return "W#" + std::to_string(p) + " K#" + std::to_string(e);
        }
    }
    return "?";
}

void UIManager::drawConstraintLabels(Scene &scene, sf::RenderWindow &window)
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    // how many labels an edge has already got, so that they do not overlap
    std::vector<std::pair<const Edge *, int>> used;
    int relationNumber = 0;

    for (const auto &constraint : scene.getConstraints().all())
    {
        std::vector<Edge *> edges = constraint->edges();
        std::string text = constraint->label();
        if (edges.size() > 1)
            text += std::to_string(++relationNumber); // the same number marks both edges of a relation

        for (const Edge *edge : edges)
        {
            auto it = std::find_if(used.begin(), used.end(), [edge](const auto &u) { return u.first == edge; });
            if (it == used.end())
            {
                used.push_back({edge, 0});
                it = used.end() - 1;
            }
            int slot = it->second++;

            sf::Vector2i pixel = window.mapCoordsToPixel(edge->getMidpoint());
            ImVec2 size = ImGui::CalcTextSize(text.c_str());
            ImVec2 pos(pixel.x + 6.0f, pixel.y - 8.0f - slot * (size.y + 4.0f));
            drawList->AddRectFilled(ImVec2(pos.x - 2.0f, pos.y - 1.0f), ImVec2(pos.x + size.x + 2.0f, pos.y + size.y + 1.0f),
                                    IM_COL32(20, 40, 70, 220), 3.0f);
            drawList->AddText(pos, IM_COL32(140, 210, 255, 255), text.c_str());
        }
    }
}

void UIManager::drawConstraintPanel(Scene &scene)
{
    auto &polygons = scene.getPolygons();
    auto edgeAt = [&](const EdgeSelection &selection) -> Edge * {
        if (!selection.isValid() || selection.polygonIdx >= static_cast<int>(polygons.size()))
            return nullptr;
        return polygons[selection.polygonIdx]->getEdge(selection.edgeIdx);
    };

    Edge *first = edgeAt(scene.primarySelectedEdge);
    Edge *second = edgeAt(scene.secondarySelectedEdge);
    if (!first)
    {
        lengthInputEdge = nullptr;
        return;
    }

    ImGui::Separator();
    ImGui::Text("Ograniczenia:");
    ImGui::TextDisabled("Ctrl+LPM: wybor drugiej krawedzi (np. do rownoleglosci)");
    if (second)
    {
        ImGui::Text("Druga krawedz: Wielokat #%d, Krawedz #%d",
                    scene.secondarySelectedEdge.polygonIdx, scene.secondarySelectedEdge.edgeIdx);
    }

    if (first->isCurved())
    {
        ImGui::TextDisabled("Ograniczenia dotycza tylko odcinkow.");
    }
    else
    {
        // the field starts with the length now, or with the length that is already imposed
        if (first != lengthInputEdge)
        {
            lengthInputEdge = first;
            lengthInput = first->getLength();
            for (Constraint *constraint : scene.getConstraints().constraintsOf(first))
            {
                if (auto *length = dynamic_cast<LengthConstraint *>(constraint))
                    lengthInput = length->getLength();
            }
        }

        ImGui::SetNextItemWidth(110.0f);
        ImGui::InputFloat("Dlugosc", &lengthInput, 1.0f, 10.0f, "%.1f");
        ImGui::SameLine();
        ImGui::BeginDisabled(!(lengthInput > 0.0f));
        if (ImGui::Button("Ustaw dlugosc"))
        {
            editor.setEdgeLength(*first, lengthInput);
        }
        ImGui::EndDisabled();

        bool canBeParallel = second && second != first && !second->isCurved();
        ImGui::BeginDisabled(!canBeParallel);
        if (ImGui::Button("Rownolegle do drugiej krawedzi"))
        {
            editor.addParallelConstraint(*first, *second);
        }
        ImGui::EndDisabled();
        if (second && second->isCurved())
            ImGui::TextDisabled("Druga krawedz nie jest odcinkiem.");
    }

    // the constraints of both selected edges, each with the button that removes it
    std::vector<Constraint *> listed = scene.getConstraints().constraintsOf(first);
    if (second)
    {
        for (Constraint *constraint : scene.getConstraints().constraintsOf(second))
        {
            if (std::find(listed.begin(), listed.end(), constraint) == listed.end())
                listed.push_back(constraint);
        }
    }

    if (listed.empty())
    {
        ImGui::TextDisabled("Brak ograniczen na zaznaczonych krawedziach.");
        return;
    }

    ImGui::Text("Aktywne ograniczenia:");
    for (Constraint *constraint : listed)
    {
        std::string description = constraint->label() + "  [";
        std::vector<Edge *> edges = constraint->edges();
        for (size_t i = 0; i < edges.size(); ++i)
            description += (i ? ", " : "") + edgeName(scene, edges[i]);
        description += "]";

        ImGui::PushID(constraint);
        ImGui::BulletText("%s", description.c_str());
        ImGui::SameLine();
        bool removed = ImGui::SmallButton("Usun");
        ImGui::PopID();

        if (removed)
        {
            editor.removeConstraint(constraint);
            break; // the list is no longer valid
        }
    }
}

void UIManager::renderUI(Scene &scene, sf::RenderWindow &window, bool &showDocs)
{
    drawContinuityLabels(scene, window);
    drawConstraintLabels(scene, window);

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
            scene.clearEdgeSelection();
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
    drawConstraintPanel(scene);
    if (scene.selectedPolygonIdx != -1 && scene.selectedVertexIdx != -1)
    {
        Polygon *poly = scene.getPolygons()[scene.selectedPolygonIdx].get();
        Vertex *v = poly->getVertex(scene.selectedVertexIdx);
        if (v)
        {
            ImGui::Separator();
            ImGui::Text("Wybrany wierzcholek: Wielokat #%d, Wierzcholek #%d",
                        scene.selectedPolygonIdx, scene.selectedVertexIdx);
            ImGui::Text("Ciaglosc w wierzcholku: %s", continuityName(ContinuityPolicy::effective(*v)));
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

    drawDocumentation(showDocs);
}