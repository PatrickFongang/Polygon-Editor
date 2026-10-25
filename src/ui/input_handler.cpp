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
                    polygons[scene.selectedPolygonIdx]->removeVertex(scene.selectedVertexIdx);
                    scene.selectedVertexIdx = -1;
                    scene.primarySelectedEdge.invalidate();
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
        draggedControlPoint = nullptr;

        auto &polygons = scene.getPolygons();
        bool shiftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

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
            scene.primarySelectedEdge.invalidate();

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
                if (Vertex *cp = edges[eIdx]->hitTestControlPoint(mousePos, HIT_RADIUS))
                {
                    scene.selectedPolygonIdx = static_cast<int>(pIdx);
                    scene.selectedVertexIdx = -1;
                    draggedControlPoint = cp;
                    isDraggingControlPoint = true;
                    return;
                }
            }
        }

        // edge detection - only if no vertex or control point was hit
        bestDist = HIT_RADIUS;
        int hitEdgeIdx = -1;
        hitPolyIdx = -1;

        for (size_t pIdx = 0; pIdx < polygons.size(); ++pIdx)
        {
            const auto &edges = polygons[pIdx]->getEdges();
            for (size_t eIdx = 0; eIdx < edges.size(); ++eIdx)
            {
                float d = edges[eIdx]->distanceToPoint(mousePos);
                if (d <= bestDist)
                {
                    bestDist = d;
                    hitPolyIdx = static_cast<int>(pIdx);
                    hitEdgeIdx = static_cast<int>(eIdx);
                }
            }
        }

        if (hitPolyIdx != -1 && hitEdgeIdx != -1)
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
                scene.primarySelectedEdge.invalidate();
                isDraggingPolygon = true;
                return;
            }
        }

        // clicked on empty space - deselect everything
        scene.selectedPolygonIdx = -1;
        scene.selectedVertexIdx = -1;
        scene.primarySelectedEdge.invalidate();
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
                    polygons[pIdx]->removeVertex(vIdx);
                    scene.selectedVertexIdx = -1;
                    scene.primarySelectedEdge.invalidate();
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
                    polygons[pIdx]->splitEdgeAtMidpoint(eIdx);
                    scene.selectedVertexIdx = -1;
                    scene.primarySelectedEdge.invalidate();
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
        draggedControlPoint = nullptr;
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
                polygons[scene.selectedPolygonIdx]->moveVertex(scene.selectedVertexIdx, mousePos);
            }
        }
        else if (isDraggingControlPoint && draggedControlPoint != nullptr)
        {
            // control point follows the mouse cursor directly
            draggedControlPoint->setPosition(mousePos);
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