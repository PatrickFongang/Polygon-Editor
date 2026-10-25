#pragma once

#include <vector>
#include <memory>
#include "vertex.hpp"
#include "edge/edge.hpp"

class Polygon
{
public:
    bool isSelected = false;
    bool isHovered = false;

    Polygon() = default;

    void splitEdge(size_t edgeIndex, const sf::Vector2f &newVertexPosition, const sf::Color &newVertexColor = sf::Color::White, Continuity newVertexContinuity = Continuity::G0);
    void splitEdgeAtMidpoint(size_t edgeIndex);
    bool removeVertex(size_t vertexIndex);
    void replaceLastEdge(std::unique_ptr<Edge> newEdge);
    void replaceEdge(size_t edgeIndex, std::unique_ptr<Edge> newEdge);

    void moveVertex(size_t vertexIndex, const sf::Vector2f &newPosition);
    void translate(const sf::Vector2f &delta);

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
        updateVertexEdgeLinks();
    }

    const std::vector<std::unique_ptr<Vertex>> &getVertices() const { return vertices; }
    const std::vector<std::unique_ptr<Edge>> &getEdges() const { return edges; }

    Vertex *getVertex(size_t index) const
    {
        return (index < vertices.size()) ? vertices[index].get() : nullptr;
    }

    Edge *getEdge(size_t index) const
    {
        return (index < edges.size()) ? edges[index].get() : nullptr;
    }

    size_t getVertexCount() const { return vertices.size(); }
    size_t getEdgeCount() const { return edges.size(); }

private:
    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<Edge>> edges;

    std::pair<Edge *, Edge *> findEdgePair(Vertex *middleVertex);
    void updateVertexEdgeLinks();
};