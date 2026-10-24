#pragma once

#include <vector>
#include <memory>
#include "vertex.hpp"
#include "edge/edge.hpp"

class Polygon
{
public:
    bool isSelected = false; // Flag to indicate if the polygon is selected
    bool isHovered = false;  // Flag to indicate if the polygon is hovered over

    void splitEdge(size_t edgeIndex, const sf::Vector2f &newVertexPosition, const sf::Color &newVertexColor, Continuity newVertexContinuity = Continuity::G0);
    void removeVertex(size_t vertexIndex);
    void replaceLastEdge(std::unique_ptr<Edge> newEdge);

    Polygon() = default;

    void addVertex(std::unique_ptr<Vertex> vertex)
    {
        vertices.push_back(std::move(vertex));
    }

    void addVertex(float x, float y)
    {
        vertices.push_back(std::make_unique<Vertex>(sf::Vector2f(x, y)));
    }

    void addEdge(std::unique_ptr<Edge> edge)
    {
        edges.push_back(std::move(edge));
    }

    const std::vector<std::unique_ptr<Vertex>> &getVertices() const
    {
        return vertices;
    }

    const std::vector<std::unique_ptr<Edge>> &getEdges() const
    {
        return edges;
    }

    Vertex *getVertex(size_t index) const
    {
        if (index < vertices.size())
        {
            return vertices[index].get();
        }
        return nullptr;
    }

    Edge *getEdge(size_t index) const
    {
        if (index < edges.size())
        {
            return edges[index].get();
        }
        return nullptr;
    }

    const size_t getVertexCount() const
    {
        return vertices.size();
    }
    const size_t getEdgeCount() const
    {
        return edges.size();
    }

private:
    std::vector<std::unique_ptr<Vertex>> vertices; // List of vertices in the polygon
    std::vector<std::unique_ptr<Edge>> edges;      // List of edges in the polygon

    std::pair<Edge *, Edge *> findEdgePair(Vertex *middleVertex);
};