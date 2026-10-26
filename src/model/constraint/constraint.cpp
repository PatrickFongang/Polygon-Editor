#include "constraint.hpp"
#include <algorithm>

bool Constraint::involves(const Edge* edge) const
{
    std::vector<Edge*> constrained = edges();
    return std::find(constrained.begin(), constrained.end(), edge) != constrained.end();
}
