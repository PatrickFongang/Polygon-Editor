// Tests of the constraint API of the PolygonEditor and of the cleanup of constraints when the topology changes.
// No window or OpenGL context is created.
#include "test_common.hpp"
#include "model/polygon.hpp"
#include "model/vertex.hpp"
#include "model/edge/bezier_edge.hpp"
#include "model/constraint/constraint_system.hpp"
#include "model/constraint/length_constraint.hpp"
#include "model/constraint/parallel_constraint.hpp"
#include "builder/polygon_builder.hpp"
#include "editing/polygon_editor.hpp"
#include "editing/continuity_enforcer.hpp"
#include "editing/constraint_solver.hpp"

struct Fixture {
    ConstraintSystem system;
    PolygonEditor editor{system};

    Fixture() {
        editor.addRule(std::make_unique<ContinuityEnforcer>());
        editor.addRule(std::make_unique<ConstraintSolver>(system));
    }
};

static std::unique_ptr<Polygon> makeSquare(float x = 0.0f, float y = 0.0f) {
    return PolygonBuilder()
        .addVertex(x, y)
        .addVertex(x + 100, y)
        .addVertex(x + 100, y + 100)
        .addVertex(x, y + 100)
        .closePolygon()
        .build();
}

static void testEdgeTypes() {
    auto poly = PolygonBuilder()
        .addVertex(0, 0).addVertex(100, 0).withLastEdgeAsBezier()
        .addVertex(100, 100).withLastEdgeAsArc()
        .addVertex(0, 100).closePolygon().build();
    CHECK(poly->getEdge(0)->type() == EdgeType::Bezier);
    CHECK(poly->getEdge(1)->type() == EdgeType::Arc);
    CHECK(poly->getEdge(2)->type() == EdgeType::Line);
}

static void testSetEdgeLength() {
    Fixture f;
    auto square = makeSquare();

    auto* length = f.editor.setEdgeLength(*square->getEdge(0), 60.0f);
    CHECK(length != nullptr);
    CHECK(near(square->getEdge(0)->getLength(), 60.0f)); // enforced at once
    CHECK(f.system.size() == 1);

    // another request for the same edge changes the value instead of adding a second constraint
    auto* again = f.editor.setEdgeLength(*square->getEdge(0), 80.0f);
    CHECK(again == length && f.system.size() == 1);
    CHECK(near(square->getEdge(0)->getLength(), 80.0f));

    // refused: not positive, not a number
    CHECK(f.editor.setEdgeLength(*square->getEdge(1), 0.0f) == nullptr);
    CHECK(f.editor.setEdgeLength(*square->getEdge(1), -5.0f) == nullptr);
    CHECK(f.editor.setEdgeLength(*square->getEdge(1), std::nanf("")) == nullptr);
    CHECK(f.system.size() == 1);

    // refused: a curve
    f.editor.toggleEdgeType(*square, 2, EdgeType::Bezier);
    CHECK(f.editor.setEdgeLength(*square->getEdge(2), 50.0f) == nullptr);
    CHECK(f.system.size() == 1);
}

static void testAddParallel() {
    Fixture f;
    auto square = makeSquare();
    auto other = makeSquare(300, 50);

    auto* parallel = f.editor.addParallelConstraint(*square->getEdge(0), *other->getEdge(0));
    CHECK(parallel != nullptr);
    CHECK(parallel->isSatisfied(1e-4f));

    // turn a vertex: the relation is still kept
    f.editor.moveVertex(*square, 1, {100, 30});
    CHECK(parallel->isSatisfied(1e-3f));

    // refused: the same edge, a duplicate (also in the other order), a curve
    CHECK(f.editor.addParallelConstraint(*square->getEdge(1), *square->getEdge(1)) == nullptr);
    CHECK(f.editor.addParallelConstraint(*square->getEdge(0), *other->getEdge(0)) == nullptr);
    CHECK(f.editor.addParallelConstraint(*other->getEdge(0), *square->getEdge(0)) == nullptr);
    f.editor.toggleEdgeType(*other, 1, EdgeType::Arc);
    CHECK(f.editor.addParallelConstraint(*square->getEdge(0), *other->getEdge(1)) == nullptr);
    CHECK(f.system.size() == 1);
}

static void testSeveralConstraintsOnOneEdgeAndRemoval() {
    Fixture f;
    auto square = makeSquare();
    auto other = makeSquare(300, 0);
    Edge* edge = square->getEdge(0);

    auto* length = f.editor.setEdgeLength(*edge, 70.0f);
    auto* parallel = f.editor.addParallelConstraint(*edge, *other->getEdge(0));
    auto* parallel2 = f.editor.addParallelConstraint(*edge, *other->getEdge(2));
    CHECK(f.system.constraintsOf(edge).size() == 3);

    CHECK(f.editor.removeConstraint(parallel));
    CHECK(!f.editor.removeConstraint(parallel));
    CHECK(f.system.constraintsOf(edge).size() == 2);
    CHECK(f.system.constraintsOf(other->getEdge(0)).empty());
    CHECK(length != nullptr && parallel2 != nullptr);
}

static void testToggleEdgeTypeRemovesConstraintsOfThatEdge() {
    Fixture f;
    auto square = makeSquare();
    auto other = makeSquare(300, 0);
    f.editor.setEdgeLength(*square->getEdge(0), 70.0f);
    f.editor.addParallelConstraint(*square->getEdge(0), *other->getEdge(0));
    f.editor.setEdgeLength(*square->getEdge(2), 90.0f);
    f.editor.addParallelConstraint(*square->getEdge(1), *other->getEdge(1));
    CHECK(f.system.size() == 4);

    // the same type: nothing happens
    f.editor.toggleEdgeType(*square, 0, EdgeType::Line);
    CHECK(f.system.size() == 4);

    // a curve cannot carry the constraints; the ones of the other edges stay
    f.editor.toggleEdgeType(*square, 0, EdgeType::Bezier);
    CHECK(f.system.size() == 2);
    CHECK(f.system.constraintsOf(other->getEdge(0)).empty());
    CHECK(f.system.constraintsOf(square->getEdge(2)).size() == 1);
    CHECK(f.system.constraintsOf(square->getEdge(1)).size() == 1);

    // and they still work (no dangling pointers)
    f.editor.moveVertex(*square, 2, {120, 140});
    CHECK(near(square->getEdge(2)->getLength(), 90.0f, 0.05f));
}

static void testSplitRemovesConstraintsOfThatEdge() {
    Fixture f;
    auto square = makeSquare();
    auto other = makeSquare(300, 0);
    f.editor.setEdgeLength(*square->getEdge(0), 70.0f);
    f.editor.addParallelConstraint(*square->getEdge(0), *other->getEdge(0)); // joins two polygons
    f.editor.setEdgeLength(*square->getEdge(1), 90.0f);
    f.editor.addParallelConstraint(*square->getEdge(1), *other->getEdge(1));
    CHECK(f.system.size() == 4);

    f.editor.splitEdgeAtMidpoint(*square, 0);
    CHECK(square->getEdgeCount() == 5);
    CHECK(f.system.size() == 2);
    CHECK(f.system.constraintsOf(other->getEdge(0)).empty());
    CHECK(f.system.constraintsOf(square->getEdge(2)).size() == 2); // the old edge 1 is the third one now

    f.editor.moveVertex(*other, 2, {420, 130}); // runs the solver over the remaining constraints
    CHECK(f.system.all()[1]->isSatisfied(1e-3f));
}

static void testRemoveVertexRemovesConstraintsOfBothEdges() {
    Fixture f;
    auto square = makeSquare();
    auto other = makeSquare(300, 0);
    f.editor.setEdgeLength(*square->getEdge(0), 70.0f);                      // A - B
    f.editor.addParallelConstraint(*square->getEdge(1), *other->getEdge(1)); // B - C
    f.editor.setEdgeLength(*square->getEdge(2), 90.0f);                      // C - D
    f.editor.addParallelConstraint(*square->getEdge(3), *other->getEdge(3)); // D - A
    CHECK(f.system.size() == 4);

    CHECK(f.editor.removeVertex(*square, 2)); // C: B - C and C - D are gone
    CHECK(square->getVertexCount() == 3);
    CHECK(f.system.size() == 2);
    CHECK(f.system.constraintsOf(square->getEdge(0)).size() == 1);
    CHECK(f.system.constraintsOf(square->getEdge(2)).size() == 1);
    CHECK(f.system.constraintsOf(other->getEdge(1)).empty());
    f.editor.moveVertex(*square, 0, {-20, -30}); // no dangling pointer in the solver
    CHECK(near(square->getVertex(0)->getPosition(), V(-20, -30)));
}

static void testFailedRemovalKeepsConstraints() {
    Fixture f;
    auto triangle = PolygonBuilder().addVertex(0, 0).addVertex(100, 0).addVertex(0, 100).closePolygon().build();
    f.editor.setEdgeLength(*triangle->getEdge(0), 70.0f);
    CHECK(!f.editor.removeVertex(*triangle, 1));
    CHECK(f.system.size() == 1);
    CHECK(triangle->getVertexCount() == 3);
}

int main() {
    testEdgeTypes();
    testSetEdgeLength();
    testAddParallel();
    testSeveralConstraintsOnOneEdgeAndRemoval();
    testToggleEdgeTypeRemovesConstraintsOfThatEdge();
    testSplitRemovesConstraintsOfThatEdge();
    testRemoveVertexRemovesConstraintsOfBothEdges();
    testFailedRemovalKeepsConstraints();

    if (failures == 0)
        std::printf("all editor constraint tests passed\n");
    return failures == 0 ? 0 : 1;
}
