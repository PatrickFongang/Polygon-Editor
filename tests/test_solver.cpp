// Tests of the cooperation of the constraint solver with the continuity enforcer in the PolygonEditor.
// No window or OpenGL context is created.
#include "test_common.hpp"
#include "model/polygon.hpp"
#include "model/vertex.hpp"
#include "model/edge/bezier_edge.hpp"
#include "model/continuity_policy.hpp"
#include "model/constraint/constraint_system.hpp"
#include "model/constraint/length_constraint.hpp"
#include "model/constraint/parallel_constraint.hpp"
#include "builder/polygon_builder.hpp"
#include "editing/polygon_editor.hpp"
#include "editing/continuity_enforcer.hpp"
#include "editing/constraint_solver.hpp"

static PolygonEditor makeEditor(ConstraintSystem& system) {
    PolygonEditor editor;
    editor.addRule(std::make_unique<ContinuityEnforcer>());
    editor.addRule(std::make_unique<ConstraintSolver>(system));
    return editor;
}

static std::unique_ptr<Polygon> makeSquare(float x = 0.0f, float y = 0.0f) {
    return PolygonBuilder()
        .addVertex(x, y)
        .addVertex(x + 100, y)
        .addVertex(x + 100, y + 100)
        .addVertex(x, y + 100)
        .closePolygon()
        .build();
}

// A - B (Bezier) - C (line) - D (Bezier) - A (line), G1 at B and C
static std::unique_ptr<Polygon> makeChain() {
    return PolygonBuilder()
        .addVertex(0, 0, Continuity::G0)
        .addVertex(100, 0, Continuity::G1)
        .withLastEdgeAsBezier()
        .addVertex(100, 100, Continuity::G1)
        .addVertex(0, 100, Continuity::G0)
        .withLastEdgeAsBezier()
        .closePolygon()
        .build();
}

static bool continuousAt(const Vertex& v) {
    return v.getContinuityStrategy().isSatisfied(
        v.getPrevEdge()->tangentAt(EdgeEnd::End), v.getNextEdge()->tangentAt(EdgeEnd::Start), 1e-3f);
}

static void testEnforceAllSolvesConstraints() {
    auto square = makeSquare();
    ConstraintSystem system;
    system.add(std::make_unique<LengthConstraint>(square->getEdge(0), 50.0f));
    PolygonEditor editor = makeEditor(system);

    editor.enforceAll(*square);
    CHECK(near(square->getEdge(0)->getLength(), 50.0f));
}

static void testDraggedVertexFollowsCursor() {
    auto square = makeSquare();
    ConstraintSystem system;
    system.add(std::make_unique<LengthConstraint>(square->getEdge(0), 100.0f)); // A - B
    PolygonEditor editor = makeEditor(system);

    editor.moveVertex(*square, 1, {160, 20});
    CHECK(near(square->getVertex(1)->getPosition(), V(160, 20))); // B is under the cursor
    CHECK(near(square->getEdge(0)->getLength(), 100.0f));          // A has been pulled
    CHECK(near(square->getVertex(2)->getPosition(), V(100, 100))); // untouched vertices stay
}

static void testChainOfLengthsConverges() {
    auto square = makeSquare();
    ConstraintSystem system;
    system.add(std::make_unique<LengthConstraint>(square->getEdge(0), 100.0f)); // A - B
    system.add(std::make_unique<LengthConstraint>(square->getEdge(1), 100.0f)); // B - C
    PolygonEditor editor = makeEditor(system);

    editor.moveVertex(*square, 0, {-150, -40});
    CHECK(near(square->getVertex(0)->getPosition(), V(-150, -40)));
    CHECK(near(square->getEdge(0)->getLength(), 100.0f, 0.05f));
    CHECK(near(square->getEdge(1)->getLength(), 100.0f, 0.05f));
}

static void testParallelBetweenPolygons() {
    auto first = makeSquare(0, 0);
    auto second = makeSquare(300, 0);
    ConstraintSystem system;
    system.add(std::make_unique<ParallelConstraint>(first->getEdge(0), second->getEdge(0)));
    PolygonEditor editor = makeEditor(system);

    editor.moveVertex(*first, 1, {100, 40}); // B of the first square: its edge A - B turns
    CHECK(near(first->getVertex(1)->getPosition(), V(100, 40)));
    CHECK(system.all()[0]->isSatisfied(1e-3f)); // the other square followed
    CHECK(!near(second->getVertex(0)->getPosition(), V(300, 0)));

    // lengths are preserved by rotations
    CHECK(near(second->getEdge(0)->getLength(), 100.0f));
}

static void testConstraintTurnsSegmentAndCurvesFollow() {
    auto poly = makeChain();
    ConstraintSystem system;
    // B - C is parallel to D - A, and the user drags D
    system.add(std::make_unique<ParallelConstraint>(poly->getEdge(1), poly->getEdge(3)));
    PolygonEditor editor = makeEditor(system);
    editor.enforceAll(*poly);

    editor.moveVertex(*poly, 3, {-60, 100});
    CHECK(near(poly->getVertex(3)->getPosition(), V(-60, 100)));
    CHECK(system.all()[0]->isSatisfied(1e-3f));
    // B - C has turned, so the Beziers next to it had to adapt to keep G1
    CHECK(!near(poly->getVertex(2)->getPosition(), V(100, 100)));
    CHECK(continuousAt(*poly->getVertex(1)));
    CHECK(continuousAt(*poly->getVertex(2)));
}

static void testControlPointDragTurnsSegmentWithLengthConstraint() {
    auto poly = makeChain();
    ConstraintSystem system;
    system.add(std::make_unique<LengthConstraint>(poly->getEdge(1), 100.0f)); // B - C
    PolygonEditor editor = makeEditor(system);
    editor.enforceAll(*poly);

    // the control point of A - B next to B is dragged, so the segment B - C turns around B (keeping its length)
    auto* bezier = static_cast<BezierEdge*>(poly->getEdge(0));
    auto handle = bezier->hitTestControlPoint(bezier->getControlPoint2()->getPosition(), 5.0f);
    CHECK(handle.has_value());
    editor.moveControlPoint(*handle, {80, 30});

    CHECK(near(bezier->getControlPoint2()->getPosition(), V(80, 30)));
    CHECK(near(poly->getEdge(1)->getLength(), 100.0f, 0.05f));
    CHECK(continuousAt(*poly->getVertex(1)));
    CHECK(continuousAt(*poly->getVertex(2)));
}

static void testContradictingRulesTerminate() {
    // C1 forces the length of B - C to follow the control point (3 |cp - B|), the constraint wants 100
    auto poly = makeChain();
    poly->getVertex(1)->setContinuity(Continuity::C1);
    ConstraintSystem system;
    system.add(std::make_unique<LengthConstraint>(poly->getEdge(1), 100.0f));
    PolygonEditor editor = makeEditor(system);
    editor.enforceAll(*poly);

    auto* bezier = static_cast<BezierEdge*>(poly->getEdge(0));
    auto handle = bezier->hitTestControlPoint(bezier->getControlPoint2()->getPosition(), 5.0f);
    CHECK(handle.has_value());
    editor.moveControlPoint(*handle, {60, 70}); // must return, no endless loop
    CHECK(near(bezier->getControlPoint2()->getPosition(), V(60, 70))); // the dragged point still follows the cursor
}

static void testNoRulesNoMoves() {
    auto square = makeSquare();
    ConstraintSystem system;
    PolygonEditor editor = makeEditor(system);
    editor.moveVertex(*square, 2, {130, 140});
    CHECK(near(square->getVertex(2)->getPosition(), V(130, 140)));
    CHECK(near(square->getVertex(1)->getPosition(), V(100, 0)));
    CHECK(near(square->getVertex(3)->getPosition(), V(0, 100)));
}

static void testRulesReportMovedVertices() {
    auto square = makeSquare();
    ConstraintSystem system;
    system.add(std::make_unique<LengthConstraint>(square->getEdge(0), 50.0f));
    ConstraintSolver solver(system);

    EditEvent event;
    event.pinned = {square->getVertex(0)};
    auto moved = solver.apply(event);
    CHECK(moved.size() == 1 && moved[0] == square->getVertex(1));
    CHECK(solver.apply(event).empty()); // nothing left to do

    // the continuity enforcer reports the vertices it moved: a dragged control point turns a segment
    auto poly = makeChain();
    ConstraintSystem empty;
    makeEditor(empty).enforceAll(*poly);
    auto* bezier = static_cast<BezierEdge*>(poly->getEdge(0));
    bezier->getControlPoint2()->setPosition({80, 30});
    EditEvent drag;
    drag.seeds = {poly->getVertex(1)};
    drag.driver = bezier;
    auto movedByContinuity = ContinuityEnforcer().apply(drag);
    CHECK(!movedByContinuity.empty());
    CHECK(movedByContinuity[0] == poly->getVertex(2));
}

int main() {
    testEnforceAllSolvesConstraints();
    testDraggedVertexFollowsCursor();
    testChainOfLengthsConverges();
    testParallelBetweenPolygons();
    testConstraintTurnsSegmentAndCurvesFollow();
    testControlPointDragTurnsSegmentWithLengthConstraint();
    testContradictingRulesTerminate();
    testNoRulesNoMoves();
    testRulesReportMovedVertices();

    if (failures == 0)
        std::printf("all solver tests passed\n");
    return failures == 0 ? 0 : 1;
}
