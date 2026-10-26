#include "input_handler.hpp"
#include <cmath>

static bool isPointInPolygon(const sf::Vector2f &pt, const std::vector<std::unique_ptr<Vertex>> &vertices)
{
    bool inside = false;
    size_t n = vertices.size();

    for (size_t i = 0, j = n - 1; i < n; j = i++)
    {
        sf::Vector2f vi = vertices[i]->getPosition();
        sf::Vector2f vj = vertices[j]->getPosition();

        if (((vi.y > pt.y) != (vj.y > pt.y)) &&
            (pt.x < (vj.x - vi.x) * (pt.y - vi.y) / (vj.y - vi.y) + vi.x))
        {
            inside = !inside;
        }
    }
    return inside;
}

// the nearest edge within the hit radius; returns false if there is none
static bool findEdgeAt(Scene &scene, const sf::Vector2f &pos, float radius, int &polygonIdx, int &edgeIdx)
{
    auto &polygons = scene.getPolygons();
    float bestDist = radius;
    bool found = false;

    for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx)
    {
        const auto &edges = polygons[pIdx]->getEdges();
        for (size_t eIdx = 0; eIdx < edges.size(); ++eIdx)
        {
            float d = edges[eIdx]->distanceToPoint(pos);
            if (d <= bestDist)
            {
                bestDist = d;
                polygonIdx = static_cast<int>(pIdx);
                edgeIdx = static_cast<int>(eIdx);
                found = true;
            }
        }
    }
    return found;
}

float InputHandler::distance(const sf::Vector2f &a, const sf::Vector2f &b)
{
    return std::hypot(a.x - b.x, a.y - b.y);
}

void InputHandler::handleEvent(const sf::Event &event, const sf::RenderWindow &window, Scene &scene, bool &showDocs)
{

    // keyboard events
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::F1)
        {
            showDocs = !showDocs;
        }
        // deleting a vertex with Delete or Backspace
        else if (event.key.code == sf::Keyboard::Delete || event.key.code == sf::Keyboard::BackSpace)
        {
            if (scene.selectedPolygonIdx != -1 && scene.selectedVertexIdx != -1)
            {
                auto &polygons = scene.getPolygons();
                if (scene.selectedPolygonIdx < static_cast<int>(polygons.size()))
                {
                    editor.removeVertex(*polygons[scene.selectedPolygonIdx], scene.selectedVertexIdx);
                    scene.selectedVertexIdx = -1;
                    scene.clearEdgeSelection();
                }
            }
        }
    }

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    // left moude button clicked - hitbox detection for vertex, control point, or edge
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        lastMousePos = mousePos;
        isDraggingVertex = false;
        isDraggingControlPoint = false;
        isDraggingPolygon = false;
        draggedHandle.reset();

        auto &polygons = scene.getPolygons();
        bool shiftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

        bool ctrlHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
        if (ctrlHeld)
        {
            // Ctrl + click picks the second edge for a relation between two edges (it can be in another polygon)
            int polyIdx = -1;
            int edgeIdx = -1;
            if (findEdgeAt(scene, mousePos, HIT_RADIUS, polyIdx, edgeIdx))
                scene.secondarySelectedEdge = {polyIdx, edgeIdx};
            else
                scene.secondarySelectedEdge.invalidate();
            return;
        }

        // vertex detection
        float bestDist = HIT_RADIUS;
        int hitPolyIdx = -1;
        int hitVertIdx = -1;

        for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx)
        {
            const auto &vertices = polygons[pIdx]->getVertices();
            for (size_t vIdx = 0; vIdx < vertices.size(); ++vIdx)
            {
                float d = distance(mousePos, vertices[vIdx]->getPosition());
                if (d <= bestDist)
                {
                    bestDist = d;
                    hitPolyIdx = static_cast<int>(pIdx);
                    hitVertIdx = static_cast<int>(vIdx);
                }
            }
        }

        if (hitPolyIdx != -1 && hitVertIdx != -1)
        {
            scene.selectedPolygonIdx = hitPolyIdx;
            scene.selectedVertexIdx = hitVertIdx;
            scene.clearEdgeSelection();

            if (shiftHeld)
            {
                isDraggingPolygon = true;
            }
            else
            {
                isDraggingVertex = true;
            }
            return;
        }

        // control point detection (for Bezier edges) - only if no vertex was hit
        for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx)
        {
            const auto &edges = polygons[pIdx]->getEdges();
            for (size_t eIdx = 0; eIdx < edges.size(); ++eIdx)
            {
                if (auto handle = edges[eIdx]->hitTestControlPoint(mousePos, HIT_RADIUS))
                {
                    scene.selectedPolygonIdx = static_cast<int>(pIdx);
                    scene.selectedVertexIdx = -1;
                    draggedHandle = handle;
                    isDraggingControlPoint = true;
                    return;
                }
            }
        }

        // edge detection - only if no vertex or control point was hit
        int hitEdgeIdx = -1;
        hitPolyIdx = -1;

        if (findEdgeAt(scene, mousePos, HIT_RADIUS, hitPolyIdx, hitEdgeIdx))
        {
            scene.selectedPolygonIdx = hitPolyIdx;
            scene.selectedVertexIdx = -1;
            scene.primarySelectedEdge = {hitPolyIdx, hitEdgeIdx};
            // if edge is hit, we can start dragging the whole polygon
            isDraggingPolygon = true;
            return;
        }

        // polygon interior detection
        for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx)
        {
            if (isPointInPolygon(mousePos, polygons[pIdx]->getVertices()))
            {
                scene.selectedPolygonIdx = static_cast<int>(pIdx);
                scene.selectedVertexIdx = -1;
                scene.clearEdgeSelection();
                isDraggingPolygon = true;
                return;
            }
        }

        // clicked on empty space - deselect everything
        scene.selectedPolygonIdx = -1;
        scene.selectedVertexIdx = -1;
        scene.clearEdgeSelection();
    }

    // right mouse button clicked - add or remove vertex
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
    {
        auto &polygons = scene.getPolygons();

        // if right-clicked on a vertex, remove it
        for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx)
        {
            const auto &vertices = polygons[pIdx]->getVertices();
            for (size_t vIdx = 0; vIdx < vertices.size(); ++vIdx)
            {
                if (distance(mousePos, vertices[vIdx]->getPosition()) <= HIT_RADIUS)
                {
                    editor.removeVertex(*polygons[pIdx], vIdx);
                    scene.selectedVertexIdx = -1;
                    scene.clearEdgeSelection();
                    return;
                }
            }
        }

        // if right-clicked on an edge, split it in the midpoint
        for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx)
        {
            const auto &edges = polygons[pIdx]->getEdges();
            for (size_t eIdx = 0; eIdx < edges.size(); ++eIdx)
            {
                if (edges[eIdx]->distanceToPoint(mousePos) <= HIT_RADIUS)
                {
                    editor.splitEdgeAtMidpoint(*polygons[pIdx], eIdx);
                    scene.selectedVertexIdx = -1;
                    scene.clearEdgeSelection();
                    return;
                }
            }
        }
    }

    // left mouse button released - stop dragging
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        isDraggingVertex = false;
        isDraggingControlPoint = false;
        isDraggingPolygon = false;
        draggedHandle.reset();
    }

    // mouse moved - handle dragging of vertex, control point, or polygon
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f delta = mousePos - lastMousePos;
        auto &polygons = scene.getPolygons();

        if (isDraggingVertex && scene.selectedPolygonIdx != -1 && scene.selectedVertexIdx != -1)
        {
            if (scene.selectedPolygonIdx < static_cast<int>(polygons.size()))
            {
                // vertex follows the mouse cursor directly
                editor.moveVertex(*polygons[scene.selectedPolygonIdx], scene.selectedVertexIdx, mousePos);
            }
        }
        else if (isDraggingControlPoint && draggedHandle)
        {
            // control point follows the mouse cursor directly, the neighbouring edge adapts to keep the continuity
            editor.moveControlPoint(*draggedHandle, mousePos);
        }
        else if (isDraggingPolygon && scene.selectedPolygonIdx != -1)
        {
            if (scene.selectedPolygonIdx < static_cast<int>(polygons.size()))
            {
                // translate the entire polygon by the mouse delta
                polygons[scene.selectedPolygonIdx]->translate(delta);
            }
        }

        lastMousePos = mousePos;
    }
}