#include "continuity_policy.hpp"
#include "vertex.hpp"
#include "edge/edge.hpp"

namespace
{
    Vertex* oppositeVertex(const Edge& edge, const Vertex& vertex)
    {
        return edge.getVertex1() == &vertex ? edge.getVertex2() : edge.getVertex1();
    }
}

bool ContinuityPolicy::matters(const Vertex& vertex)
{
    const Edge* prev = vertex.getPrevEdge();
    const Edge* next = vertex.getNextEdge();
    return prev && next && (prev->isCurved() || next->isCurved());
}

bool ContinuityPolicy::isAllowed(const Vertex& vertex, Continuity continuity)
{
    if (continuity == Continuity::G0)
        return true;

    const Edge* prev = vertex.getPrevEdge();
    const Edge* next = vertex.getNextEdge();
    if (!prev || !next)
        return false;

    if (!prev->isCurved() && !next->isCurved())
        return false;

    if (!prev->supports(continuity) || !next->supports(continuity))
        return false;

    if (continuity == Continuity::G1)
    {
        if (prev->derivesTangent() && next->derivesTangent())
            return false;

        for (const Edge* edge : {prev, next})
        {
            if (edge->derivesTangent() && oppositeVertex(*edge, vertex)->getContinuity() == Continuity::G1)
                return false;
        }
    }
    return true;
}

std::vector<Continuity> ContinuityPolicy::allowed(const Vertex& vertex)
{
    std::vector<Continuity> result;
    for (Continuity c : {Continuity::G0, Continuity::G1, Continuity::C1})
    {
        if (isAllowed(vertex, c))
            result.push_back(c);
    }
    return result;
}

Continuity ContinuityPolicy::defaultFor(const Vertex& vertex)
{
    for (Continuity c : {DEFAULT, Continuity::G1})
    {
        if (isAllowed(vertex, c))
            return c;
    }
    return Continuity::G0;
}

Continuity ContinuityPolicy::effective(const Vertex& vertex)
{
    Continuity stored = vertex.getContinuity();
    return isAllowed(vertex, stored) ? stored : Continuity::G0;
}

void ContinuityPolicy::sanitize(Vertex& vertex)
{
    Edge* prev = vertex.getPrevEdge();
    Edge* next = vertex.getNextEdge();
    if (!prev || !next)
        return;

    prev->releaseTangent(EdgeEnd::End);
    next->releaseTangent(EdgeEnd::Start);

    // between two segments there is nothing to be continuous, the stored choice is kept for later
    if (!prev->isCurved() && !next->isCurved())
        return;

    if (!vertex.hasExplicitContinuity)
        vertex.setContinuity(defaultFor(vertex));
    else if (!isAllowed(vertex, vertex.getContinuity()))
        vertex.setContinuity(isAllowed(vertex, Continuity::G1) ? Continuity::G1 : Continuity::G0);
}

void ContinuityPolicy::sanitize(const std::vector<Vertex*>& vertices)
{
    for (Vertex* v : vertices)
    {
        if (!v->hasExplicitContinuity)
            v->setContinuity(Continuity::G0);
    }
    // explicit choices of the user first, so that they win over defaults
    for (Vertex* v : vertices)
    {
        if (v->hasExplicitContinuity)
            sanitize(*v);
    }
    for (Vertex* v : vertices)
    {
        if (!v->hasExplicitContinuity)
            sanitize(*v);
    }
}
