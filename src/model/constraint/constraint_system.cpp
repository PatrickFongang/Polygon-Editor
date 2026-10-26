#include "constraint_system.hpp"
#include <algorithm>

Constraint* ConstraintSystem::add(std::unique_ptr<Constraint> constraint)
{
    constraints.push_back(std::move(constraint));
    return constraints.back().get();
}

bool ConstraintSystem::remove(const Constraint* constraint)
{
    return std::erase_if(constraints, [constraint](const std::unique_ptr<Constraint>& c) { return c.get() == constraint; }) > 0;
}

std::vector<Constraint*> ConstraintSystem::constraintsOf(const Edge* edge) const
{
    std::vector<Constraint*> result;
    for (const auto& constraint : constraints)
    {
        if (constraint->involves(edge))
            result.push_back(constraint.get());
    }
    return result;
}

void ConstraintSystem::removeInvolving(const Edge* edge)
{
    std::erase_if(constraints, [edge](const std::unique_ptr<Constraint>& c) { return c->involves(edge); });
}
