#pragma once
#include "edit_event.hpp"

// A rule applied after every edit of a polygon (continuity now, geometric constraints later).
class PostEditRule {
public:
    virtual ~PostEditRule() = default;
    virtual void apply(const EditEvent& event) = 0;
};
