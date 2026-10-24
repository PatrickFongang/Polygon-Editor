#include "polygon.hpp"
#include "edge/line_edge.hpp"

void Polygon::splitEdge(size_t edgeIndex, const sf::Vector2f &newVertexPosition, const sf::Color &newVertexColor, Continuity newVertexContinuity)
{
    if (edgeIndex >= edges.size())
    {
        return;
    }

    Edge *edgeToSplit = edges[edgeIndex].get();
    Vertex *v2 = edgeToSplit->getVertex2();

    auto splitVertex = std::make_unique<Vertex>(newVertexPosition, newVertexColor, newVertexContinuity);
    edgeToSplit->setVertex2(splitVertex.get());

    auto newEdge = std::make_unique<LineEdge>(splitVertex.get(), v2);
    edges.insert(edges.begin() + edgeIndex + 1, std::move(newEdge));
    vertices.push_back(std::move(splitVertex));
}

void Polygon::removeVertex(size_t vertexIndex)
{
    if (vertexIndex >= vertices.size())
    {
        return;
    }

    Vertex *vertexToRemove = vertices[vertexIndex].get();
    auto [edge1, edge2] = findEdgePair(vertexToRemove);

    if (edge1 && edge2)
    {
        edge1->setVertex2(edge2->getVertex2());
        std::erase_if(edges, [edge2](const std::unique_ptr<Edge> &e)
                      { return e.get() == edge2; });
        vertices.erase(vertices.begin() + vertexIndex);
    }
}

void Polygon::replaceLastEdge(std::unique_ptr<Edge> newEdge) {
    if (!edges.empty()) {
        edges.back() = std::move(newEdge);
    }
}

std::pair<Edge *, Edge *> Polygon::findEdgePair(Vertex *middleVertex)
{
    Edge *edge1 = nullptr;
    Edge *edge2 = nullptr;

    for (const auto &edge : edges)
    {
        if (edge->getVertex2() == middleVertex)
        {
            edge1 = edge.get();
        }
        else if (edge->getVertex1() == middleVertex)
        {
            edge2 = edge.get();
        }
    }

    return {edge1, edge2};
}