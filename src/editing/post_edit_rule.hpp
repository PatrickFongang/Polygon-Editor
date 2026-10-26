#pragma once
#include <vector>
#include "edit_event.hpp"

// A rule applied after every edit of a polygon (continuity now, geometric constraints later).
class PostEditRule {
public:
    virtual ~PostEditRule() = default;
    // Restores the invariants of the rule after the edit. Returns the vertices the rule had to move: their
    // neighbourhood has changed, so the other rules have to check it again.
    virtual std::vector<Vertex*> apply(const EditEvent& event) = 0;
};
