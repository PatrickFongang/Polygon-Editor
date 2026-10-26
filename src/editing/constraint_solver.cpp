#include "constraint_solver.hpp"
#include <algorithm>
#include "../model/constraint/constraint_system.hpp"
#include "../model/vertex.hpp"

std::vector<Vertex*> ConstraintSolver::apply(const EditEvent& event)
{
    PinnedSet pinned(event.pinned.begin(), event.pinned.end());
    std::vector<Vertex*> allMoved;

    for (int pass = 0; pass < MAX_PASSES; ++pass)
    {
        bool moved = false;
        for (const auto& constraint : system.all())
        {
            for (Vertex* vertex : constraint->project(pinned))
            {
                moved = true;
                if (std::find(allMoved.begin(), allMoved.end(), vertex) == allMoved.end())
                    allMoved.push_back(vertex);
            }
        }
        if (!moved)
            break;
    }
    return allMoved;
}
