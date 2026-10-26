#pragma once
#include <vector>
#include "../enum/continuity.hpp"

class Vertex;

// Rules about which continuity classes make sense at a vertex, and the defaults used when the edges around
// the vertex change. The continuity chosen by the user (Vertex::hasExplicitContinuity) is never overwritten
// by a default, only downgraded when it stops being possible.
class ContinuityPolicy {
public:
    // class given to a vertex that gets a curved neighbour without an explicit choice of the user
    // (G1 for surnames A-K, C1 for surnames L-Z)
    static constexpr Continuity DEFAULT = Continuity::G1;

    // can the vertex have this continuity? Rules:
    //  - continuity concerns only vertices with at least one curved edge, the others are G0,
    //  - both edges must support the class (an arc has no C1),
    //  - an arc follows a tangent at only one end, so G1 is refused if the other end of an adjacent arc is G1,
    //    or if both adjacent edges are arcs (they would wait for each other).
    static bool isAllowed(const Vertex& vertex, Continuity continuity);

    // all classes the vertex can have, for the UI
    static std::vector<Continuity> allowed(const Vertex& vertex);

    // default for the vertex: DEFAULT if possible, otherwise the closest weaker class
    static Continuity defaultFor(const Vertex& vertex);

    // the class that is really enforced: the stored one, or G0 when it is not possible (right now)
    static Continuity effective(const Vertex& vertex);

    // brings the continuity of the vertex in line with its current edges (after the edges were changed):
    // non-explicit vertices get the default, explicit ones are only downgraded if needed.
    // Tangents imposed on the adjacent arcs are released, the continuity enforcer anchors them again.
    static void sanitize(Vertex& vertex);

    // the same for several vertices; non-explicit ones first become G0. Conflicts over the only G1 end of an arc
    // are won by explicit choices, then by the earlier vertices of the list
    static void sanitize(const std::vector<Vertex*>& vertices);
};
