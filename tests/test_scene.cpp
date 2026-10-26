// Tests of the predefined scene: it has to show the main features and be consistent once the rules are applied.
// No window or OpenGL context is created (the scene gets no drawer).
#include "test_common.hpp"
#include "scene/scene.hpp"
#include "model/continuity_policy.hpp"
#include "model/constraint/length_constraint.hpp"
#include "model/constraint/parallel_constraint.hpp"
#include "editing/polygon_editor.hpp"
#include "editing/continuity_enforcer.hpp"
#include "editing/constraint_solver.hpp"

struct DemoScene {
    Scene scene{nullptr};
    PolygonEditor editor{scene.getConstraints()};

    DemoScene() {
        scene.initPredefinedScene();
        editor.addRule(std::make_unique<ContinuityEnforcer>());
        editor.addRule(std::make_unique<ConstraintSolver>(scene.getConstraints()));
        for (auto& polygon : scene.getPolygons())
            editor.enforceAll(*polygon);
    }
};

static bool continuousAt(const Vertex& v) {
    return v.getContinuityStrategy().isSatisfied(
        v.getPrevEdge()->tangentAt(EdgeEnd::End), v.getNextEdge()->tangentAt(EdgeEnd::Start), 1e-3f);
}

static void testSceneContents() {
    DemoScene demo;
    auto& polygons = demo.scene.getPolygons();
    CHECK(polygons.size() >= 2);

    bool hasBezier = false, hasArc = false;
    for (auto& polygon : polygons) {
        for (auto& edge : polygon->getEdges()) {
            hasBezier |= edge->type() == EdgeType::Bezier;
            hasArc |= edge->type() == EdgeType::Arc;
        }
    }
    CHECK(hasBezier && hasArc);

    auto& constraints = demo.scene.getConstraints();
    CHECK(constraints.size() >= 2);
    bool hasLength = false, hasParallel = false, betweenPolygons = false, severalOnOneEdge = false;
    for (const auto& constraint : constraints.all()) {
        hasLength |= dynamic_cast<LengthConstraint*>(constraint.get()) != nullptr;
        hasParallel |= dynamic_cast<ParallelConstraint*>(constraint.get()) != nullptr;
        auto edges = constraint->edges();
        if (edges.size() == 2) {
            bool inFirst = false, inSecond = false;
            for (const auto& edge : polygons[0]->getEdges()) inFirst |= edge.get() == edges[0] || edge.get() == edges[1];
            for (const auto& edge : polygons[1]->getEdges()) inSecond |= edge.get() == edges[0] || edge.get() == edges[1];
            betweenPolygons |= inFirst && inSecond;
        }
        for (Edge* edge : edges)
            severalOnOneEdge |= constraints.constraintsOf(edge).size() > 1;
        for (Edge* edge : edges)
            CHECK(!edge->isCurved()); // constraints concern segments
    }
    CHECK(hasLength && hasParallel && betweenPolygons && severalOnOneEdge);
}

static void testSceneIsConsistentAtStart() {
    DemoScene demo;
    for (const auto& constraint : demo.scene.getConstraints().all())
        CHECK(constraint->isSatisfied(1e-2f));

    for (auto& polygon : demo.scene.getPolygons()) {
        for (auto& vertex : polygon->getVertices()) {
            if (ContinuityPolicy::matters(*vertex))
                CHECK(continuousAt(*vertex));
        }
    }
}

static void testSceneSurvivesEditsAndReset() {
    DemoScene demo;
    auto& first = *demo.scene.getPolygons()[0];
    auto& second = *demo.scene.getPolygons()[1];

    demo.editor.moveVertex(first, 2, {230, 260});
    demo.editor.moveVertex(second, 3, {520, 150});
    for (const auto& constraint : demo.scene.getConstraints().all())
        CHECK(constraint->isSatisfied(1e-2f));

    // after a reset the old constraints are gone with the old polygons, new ones are in place
    demo.scene.initPredefinedScene();
    for (auto& polygon : demo.scene.getPolygons())
        demo.editor.enforceAll(*polygon);
    CHECK(demo.scene.getConstraints().size() == 4);
    for (const auto& constraint : demo.scene.getConstraints().all())
        CHECK(constraint->isSatisfied(1e-2f));
}

int main() {
    testSceneContents();
    testSceneIsConsistentAtStart();
    testSceneSurvivesEditsAndReset();

    if (failures == 0)
        std::printf("all scene tests passed\n");
    return failures == 0 ? 0 : 1;
}
