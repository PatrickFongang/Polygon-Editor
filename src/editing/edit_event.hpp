#pragma once
#include <vector>

class Vertex;
class Edge;

// Describes what has just been changed by an edit, so that rules can restore their invariants.
struct EditEvent {
    // vertices around which the geometry changed (rules check their relations there)
    std::vector<Vertex*> seeds;

    // vertices that the user holds (e.g. the one being dragged): rules must leave them where they are
    std::vector<Vertex*> pinned;

    // edge the user is manipulating directly (e.g. through its control point), it must not be adapted
    Edge* driver = nullptr;
};
