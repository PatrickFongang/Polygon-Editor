#pragma once
#include <memory>
#include <vector>
#include "constraint.hpp"

// All constraints of the scene. It lives next to the polygons (a constraint can join edges of different polygons).
class ConstraintSystem {
public:
    // takes the ownership, returns the added constraint
    Constraint* add(std::unique_ptr<Constraint> constraint);

    // returns false if there is no such constraint
    bool remove(const Constraint* constraint);

    // constraints imposed on the edge (several of them can be)
    std::vector<Constraint*> constraintsOf(const Edge* edge) const;

    // removes all constraints that involve the edge, to be called before the edge disappears
    void removeInvolving(const Edge* edge);

    void clear() { constraints.clear(); }

    const std::vector<std::unique_ptr<Constraint>>& all() const { return constraints; }
    size_t size() const { return constraints.size(); }

private:
    std::vector<std::unique_ptr<Constraint>> constraints;
};
