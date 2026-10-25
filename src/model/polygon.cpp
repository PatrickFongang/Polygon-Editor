#include "polygon.hpp"
#include "edge/line_edge.hpp"
#include <algorithm>

void Polygon::updateVertexEdgeLinks()
{
    for (auto &v : vertices)
    {
        v->setPrevEdge(nullptr);
        v->setNextEdge(nullptr);
    }
    for (auto &e : edges)
    {
        if (e->getVertex1()) e->getVertex1()->setNextEdge(e.get());
        if (e->getVertex2()) e->getVertex2()->setPrevEdge(e.get());
    }
}

void Polygon::splitEdge(size_t edgeIndex, const sf::Vector2f &newVertexPosition, const sf::Color &newVertexColor, Continuity newVertexContinuity)
{
    if (edgeIndex >= edges.size())
    {
        return;
    }

    Edge *edgeToSplit = edges[edgeIndex].get();
    Vertex *v1 = edgeToSplit->getVertex1();
    Vertex *v2 = edgeToSplit->getVertex2();

    auto splitVertex = std::make_unique<Vertex>(newVertexPosition, newVertexColor, newVertexContinuity);
    Vertex *splitVertexPtr = splitVertex.get();

    // put vertex in the correct position in the vertices vector
    auto v1It = std::find_if(vertices.begin(), vertices.end(),
                             [v1](const std::unique_ptr<Vertex> &v) { return v.get() == v1; });
    if (v1It != vertices.end())
    {
        vertices.insert(v1It + 1, std::move(splitVertex));
    }
    else
    {
        vertices.push_back(std::move(splitVertex));
    }

    // replace the edge to split with two new line edges
    edges[edgeIndex] = std::make_unique<LineEdge>(v1, splitVertexPtr);
    auto newEdge = std::make_unique<LineEdge>(splitVertexPtr, v2);
    edges.insert(edges.begin() + edgeIndex + 1, std::move(newEdge));

    updateVertexEdgeLinks();
}

void Polygon::splitEdgeAtMidpoint(size_t edgeIndex)
{
    if (edgeIndex >= edges.size()) return;
    sf::Vector2f mid = edges[edgeIndex]->getMidpoint();
    splitEdge(edgeIndex, mid, sf::Color::White, Continuity::G0);
}

bool Polygon::removeVertex(size_t vertexIndex)
{
    // the polygon must have at least 3 vertices to remain valid after removal
    if (vertexIndex >= vertices.size() || vertices.size() <= 3)
    {
        return false;
    }

    Vertex *vertexToRemove = vertices[vertexIndex].get();
    auto [edge1, edge2] = findEdgePair(vertexToRemove);

    if (edge1 && edge2)
    {
        Vertex *prevV = edge1->getVertex1();
        Vertex *nextV = edge2->getVertex2();

        // change the first edge to connect prevV and nextV
        auto edge1It = std::find_if(edges.begin(), edges.end(),
                                    [edge1](const std::unique_ptr<Edge> &e) { return e.get() == edge1; });
        if (edge1It != edges.end())
        {
            *edge1It = std::make_unique<LineEdge>(prevV, nextV);
        }

        // delete the second edge and the vertex
        std::erase_if(edges, [edge2](const std::unique_ptr<Edge> &e)
                      { return e.get() == edge2; });
        vertices.erase(vertices.begin() + vertexIndex);

        updateVertexEdgeLinks();
        return true;
    }
    return false;
}

void Polygon::replaceLastEdge(std::unique_ptr<Edge> newEdge)
{
    if (!edges.empty())
    {
        edges.back() = std::move(newEdge);
        updateVertexEdgeLinks();
    }
}

void Polygon::replaceEdge(size_t edgeIndex, std::unique_ptr<Edge> newEdge)
{
    if (edgeIndex < edges.size())
    {
        edges[edgeIndex] = std::move(newEdge);
        updateVertexEdgeLinks();
    }
}

void Polygon::moveVertex(size_t vertexIndex, const sf::Vector2f &newPosition)
{
    if (vertexIndex >= vertices.size()) return;

    Vertex *v = vertices[vertexIndex].get();
    sf::Vector2f delta = newPosition - v->getPosition();
    v->setPosition(newPosition);

    if (v->getPrevEdge()) v->getPrevEdge()->onEndpointMoved(v, delta);
    if (v->getNextEdge()) v->getNextEdge()->onEndpointMoved(v, delta);
}

void Polygon::translate(const sf::Vector2f &delta)
{
    for (auto &v : vertices)
    {
        v->move(delta);
    }
    for (auto &e : edges)
    {
        e->translateControlPoints(delta);
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