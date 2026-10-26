#pragma once
#include <string>
#include <unordered_set>
#include <vector>

class Edge;
class Vertex;

// Vertices that must not be moved by a constraint (e.g. the vertex the user drags: it follows the cursor).
using PinnedSet = std::unordered_set<const Vertex*>;

// A geometric relation imposed on one edge or on a pair of edges (also from different polygons).
// A constraint does not own the edges; ConstraintSystem removes it when one of its edges is going away.
class Constraint {
public:
    virtual ~Constraint() = default;

    // the edges the constraint is imposed on
    virtual std::vector<Edge*> edges() const = 0;

    bool involves(const Edge* edge) const;

    // Is the relation satisfied? eps is a tolerance in the unit natural to the constraint
    // (pixels for a length, radians for an angle).
    virtual bool isSatisfied(float eps) const = 0;

    // Moves the vertices which are not pinned so that the relation holds (one Gauss-Seidel step, a single
    // constraint at a time - others may need another pass). Returns the vertices that were moved.
    virtual std::vector<Vertex*> project(const PinnedSet& pinned) = 0;

    // short text shown next to the edge on the scene
    virtual std::string label() const = 0;
};
