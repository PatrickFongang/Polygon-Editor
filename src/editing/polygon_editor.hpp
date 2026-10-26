#pragma once
#include <memory>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "post_edit_rule.hpp"

class Polygon;
struct ControlHandle;

// Entry point for edits of polygons. Applies an edit to the model and then lets the registered rules
// (continuity, later geometric constraints) restore their invariants.
class PolygonEditor {
public:
    void addRule(std::unique_ptr<PostEditRule> rule);

    // the vertex follows the position, its control points move with it
    void moveVertex(Polygon& polygon, size_t vertexIndex, const sf::Vector2f& position);

    // the control point follows the position, the edge it belongs to is the driver of the continuity at its vertex
    // (the neighbouring edge adapts, which may require moving the vertices of a neighbouring line)
    void moveControlPoint(const ControlHandle& handle, const sf::Vector2f& position);

    // checks the rules at all vertices, e.g. after a polygon has been built
    void enforceAll(Polygon& polygon);

private:
    std::vector<std::unique_ptr<PostEditRule>> rules;

    void notify(const EditEvent& event);
};
