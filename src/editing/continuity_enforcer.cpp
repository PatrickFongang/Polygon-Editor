#include "continuity_enforcer.hpp"
#include <deque>
#include "../model/vertex.hpp"
#include "../model/continuity_policy.hpp"
#include "../model/edge/edge.hpp"

std::vector<Vertex*> ContinuityEnforcer::apply(const EditEvent& event)
{
    std::vector<Vertex*> allMoved;
    struct Item
    {
        Vertex* vertex;
        Edge* driver;
    };

    std::deque<Item> work;
    for (Vertex* v : event.seeds)
        work.push_back({v, event.driver});

    for (int steps = 0; !work.empty() && steps < MAX_STEPS; ++steps)
    {
        Item item = work.front();
        work.pop_front();

        for (Vertex* moved : enforceAt(*item.vertex, item.driver))
        {
            allMoved.push_back(moved);
            // a moved vertex changes the tangents of the lines around it
            work.push_back({moved, nullptr});
            for (Vertex* adjacent : moved->adjacentVertices())
                work.push_back({adjacent, nullptr});
        }
    }
    return allMoved;
}

std::vector<Vertex*> ContinuityEnforcer::enforceAt(Vertex& vertex, Edge* driver)
{
    Edge* prev = vertex.getPrevEdge();
    Edge* next = vertex.getNextEdge();
    if (!prev || !next)
        return {};

    Continuity continuity = ContinuityPolicy::effective(vertex);
    if (continuity == Continuity::G0)
        return {};

    // both tangents are oriented along the traversal of the polygon
    sf::Vector2f incoming = prev->tangentAt(EdgeEnd::End);
    sf::Vector2f outgoing = next->tangentAt(EdgeEnd::Start);

    const ContinuityStrategy& strategy = ContinuityStrategy::get(continuity);
    if (strategy.isSatisfied(incoming, outgoing, TOLERANCE))
        return {};

    bool prevFollows;
    if (driver == prev)
        prevFollows = false;
    else if (driver == next)
        prevFollows = true;
    else
        prevFollows = prev->adaptCost() < next->adaptCost();

    if (prevFollows)
    {
        if (auto desired = strategy.desiredFollower(outgoing, incoming))
            return prev->adaptTangent(EdgeEnd::End, *desired);
    }
    else
    {
        if (auto desired = strategy.desiredFollower(incoming, outgoing))
            return next->adaptTangent(EdgeEnd::Start, *desired);
    }
    return {};
}
