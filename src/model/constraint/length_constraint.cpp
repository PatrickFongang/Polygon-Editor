#include "length_constraint.hpp"
#include <cmath>
#include <cstdio>
#include "../edge/edge.hpp"
#include "../vertex.hpp"
#include "../../utils/vec_math.hpp"

bool LengthConstraint::isSatisfied(float eps) const
{
    return std::abs(edge->getLength() - length) <= eps;
}

std::vector<Vertex*> LengthConstraint::project(const PinnedSet& pinned)
{
    Vertex* a = edge->getVertex1();
    Vertex* b = edge->getVertex2();

    float current = edge->getLength();
    if (std::abs(current - length) <= 1e-4f)
        return {};

    // an end that is pinned does not take part in the correction
    float wa = pinned.count(a) ? 0.0f : 1.0f;
    float wb = pinned.count(b) ? 0.0f : 1.0f;
    if (wa + wb == 0.0f)
        return {};

    // direction from a to b, an arbitrary one if both ends coincide
    sf::Vector2f direction = current > 0.0f ? (b->getPosition() - a->getPosition()) / current : sf::Vector2f(1.f, 0.f);
    sf::Vector2f correction = direction * (current - length);

    std::vector<Vertex*> moved;
    if (wa > 0.0f)
    {
        a->moveWithEdges(a->getPosition() + correction * (wa / (wa + wb)));
        moved.push_back(a);
    }
    if (wb > 0.0f)
    {
        b->moveWithEdges(b->getPosition() - correction * (wb / (wa + wb)));
        moved.push_back(b);
    }
    return moved;
}

std::string LengthConstraint::label() const
{
    char text[32];
    std::snprintf(text, sizeof(text), "L=%.0f", length);
    return text;
}
