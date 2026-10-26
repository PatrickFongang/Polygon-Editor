#include "parallel_constraint.hpp"
#include <algorithm>
#include <cmath>
#include "../edge/edge.hpp"
#include "../vertex.hpp"
#include "../../utils/vec_math.hpp"

namespace
{
    constexpr float PI = 3.14159265358979323846f;

    void addUnique(std::vector<Vertex*>& list, Vertex* vertex)
    {
        if (std::find(list.begin(), list.end(), vertex) == list.end())
            list.push_back(vertex);
    }

    // turns the edge by the angle phi; only the ends which are not pinned move
    void rotateEdge(Edge& edge, float phi, const PinnedSet& pinned, std::vector<Vertex*>& moved)
    {
        Vertex* a = edge.getVertex1();
        Vertex* b = edge.getVertex2();
        bool aPinned = pinned.count(a) > 0;
        bool bPinned = pinned.count(b) > 0;

        sf::Vector2f pivot = (a->getPosition() + b->getPosition()) * 0.5f;
        if (aPinned != bPinned)
            pivot = aPinned ? a->getPosition() : b->getPosition();

        float c = std::cos(phi);
        float s = std::sin(phi);
        auto rotated = [&](const sf::Vector2f& p) {
            sf::Vector2f r = p - pivot;
            return pivot + sf::Vector2f(c * r.x - s * r.y, s * r.x + c * r.y);
        };

        // both new positions are computed before any vertex moves
        sf::Vector2f newA = rotated(a->getPosition());
        sf::Vector2f newB = rotated(b->getPosition());
        if (!aPinned)
        {
            a->moveWithEdges(newA);
            addUnique(moved, a);
        }
        if (!bPinned)
        {
            b->moveWithEdges(newB);
            addUnique(moved, b);
        }
    }
}

float ParallelConstraint::angleError() const
{
    sf::Vector2f d1 = first->getVertex2()->getPosition() - first->getVertex1()->getPosition();
    sf::Vector2f d2 = second->getVertex2()->getPosition() - second->getVertex1()->getPosition();
    if (vec::length(d1) <= 0.0f || vec::length(d2) <= 0.0f)
        return 0.0f; // no direction to compare

    float theta = std::atan2(vec::cross(d1, d2), vec::dot(d1, d2));
    // parallel lines: the opposite direction is as good as the same one
    if (theta > PI / 2.0f)
        theta -= PI;
    else if (theta <= -PI / 2.0f)
        theta += PI;
    return theta;
}

bool ParallelConstraint::isSatisfied(float eps) const
{
    return std::abs(angleError()) <= eps;
}

std::vector<Vertex*> ParallelConstraint::project(const PinnedSet& pinned)
{
    float theta = angleError();
    if (std::abs(theta) <= 1e-5f)
        return {};

    auto weight = [&](const Edge* edge) {
        return pinned.count(edge->getVertex1()) && pinned.count(edge->getVertex2()) ? 0.0f : 1.0f;
    };
    float w1 = weight(first);
    float w2 = weight(second);
    if (w1 + w2 == 0.0f)
        return {};

    // the second direction is the first one turned by theta, so the first edge turns by +theta and the second
    // by -theta, each by its share
    std::vector<Vertex*> moved;
    if (w1 > 0.0f)
        rotateEdge(*first, theta * w1 / (w1 + w2), pinned, moved);
    if (w2 > 0.0f)
        rotateEdge(*second, -theta * w2 / (w1 + w2), pinned, moved);
    return moved;
}
