#pragma once
#include "post_edit_rule.hpp"

class ConstraintSystem;

// Restores the geometric constraints (lengths, parallelism...) after an edit.
//
// Every constraint is projected in turn (Gauss-Seidel): each projection fixes its own relation, which may break
// the neighbouring ones, so the passes are repeated until all of them hold or the limit is reached. The vertices
// pinned by the edit (the one under the cursor) never move, the rest of the system adapts to them.
// If the constraints contradict each other (or the pinned vertices), the last state stays, without an error.
class ConstraintSolver : public PostEditRule {
public:
    explicit ConstraintSolver(ConstraintSystem& constraintSystem) : system(constraintSystem) {}

    std::vector<Vertex*> apply(const EditEvent& event) override;

private:
    static constexpr int MAX_PASSES = 50;

    ConstraintSystem& system;
};
