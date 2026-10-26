#pragma once
#include <memory>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "post_edit_rule.hpp"
#include "../enum/edge_type.hpp"
#include "../enum/continuity.hpp"

class Polygon;
class Vertex;
class Edge;
class Constraint;
class ConstraintSystem;
class LengthConstraint;
class ParallelConstraint;
struct ControlHandle;

// Entry point for edits of polygons. Applies an edit to the model and then lets the registered rules
// (continuity, geometric constraints) restore their invariants. It also keeps the constraints consistent with the
// topology: a constraint disappears together with an edge that is split, removed or turned into a curve.
class PolygonEditor {
public:
    explicit PolygonEditor(ConstraintSystem& constraintSystem) : constraints(constraintSystem) {}

    void addRule(std::unique_ptr<PostEditRule> rule);

    // the vertex follows the position, its control points move with it
    void moveVertex(Polygon& polygon, size_t vertexIndex, const sf::Vector2f& position);

    // the control point follows the position, the edge it belongs to is the driver of the continuity at its vertex
    // (the neighbouring edge adapts, which may require moving the vertices of a neighbouring line)
    void moveControlPoint(const ControlHandle& handle, const sf::Vector2f& position);

    // changes the type of the edge (segment, Bezier curve, arc); the vertices at its ends get the default
    // continuity unless the user chose one, and the neighbouring edges adapt to it
    void toggleEdgeType(Polygon& polygon, size_t edgeIndex, EdgeType type);

    // adds a vertex in the middle of the edge; the edge is replaced by two segments
    void splitEdgeAtMidpoint(Polygon& polygon, size_t edgeIndex);

    // removes the vertex (the polygon keeps at least 3); returns false if it was not possible
    bool removeVertex(Polygon& polygon, size_t vertexIndex);

    // sets the continuity chosen by the user at the vertex; refused (false) if the policy does not allow it.
    // The choice is marked as explicit, so defaults no longer overwrite it, and tangents imposed on the
    // adjacent arcs are released and anchored again by the rules if the new class needs it.
    bool setContinuity(Vertex& vertex, Continuity continuity);

    // Gives the segment the length (a second request for the same edge changes the value of the first one).
    // Only segments can be constrained; returns nullptr if the request is refused.
    LengthConstraint* setEdgeLength(Edge& edge, float length);

    // Makes two different segments parallel; returns nullptr if the request is refused
    // (a curve, the same edge twice, or the edges are already constrained that way).
    ParallelConstraint* addParallelConstraint(Edge& first, Edge& second);

    bool removeConstraint(const Constraint* constraint);

    // checks the rules at all vertices, e.g. after a polygon has been built
    void enforceAll(Polygon& polygon);

private:
    ConstraintSystem& constraints;
    std::vector<std::unique_ptr<PostEditRule>> rules;

    // all the constraints involving any of the edges
    std::vector<Constraint*> constraintsOf(const std::vector<Edge*>& edges) const;
    void removeConstraints(const std::vector<Constraint*>& doomed);
    // runs the rules around the ends of the edges (after a constraint was added)
    void notifyAround(const std::vector<Edge*>& edges);

    // Lets the rules restore their invariants. A rule can break what another one has fixed (a constraint turns a
    // segment, so the continuity at its ends has to follow, and the other way round), therefore the rules are
    // repeated, around the vertices that were moved, until nothing moves any more or the limit is reached.
    void notify(EditEvent event);

    static constexpr int MAX_ROUNDS = 20;
    // restores the continuity around vertices whose edges have just been changed
    void sanitizeAndNotify(const std::vector<Vertex*>& vertices);
};
