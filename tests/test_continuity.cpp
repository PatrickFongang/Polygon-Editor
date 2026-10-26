// Unit tests of the continuity math: strategies (pure vector math) and the tangent interface of edges.
// No window or OpenGL context is created.
#include "test_common.hpp"
#include "model/continuity_strategy.hpp"
#include "model/vertex.hpp"
#include "model/edge/line_edge.hpp"
#include "model/edge/bezier_edge.hpp"
#include "model/edge/arc_edge.hpp"
#include "model/polygon.hpp"
#include "builder/polygon_builder.hpp"
#include "editing/polygon_editor.hpp"
#include "editing/continuity_enforcer.hpp"
#include "model/continuity_policy.hpp"
#include "model/constraint/constraint_system.hpp"
#include <algorithm>

static void testG0() {
    const auto& g0 = ContinuityStrategy::get(Continuity::G0);
    CHECK(g0.getType() == Continuity::G0);
    CHECK(!g0.desiredFollower({1, 0}, {0, 5}).has_value());
    CHECK(g0.isSatisfied({1, 0}, {-1, 0}, 1e-3f));
}

static void testG1() {
    const auto& g1 = ContinuityStrategy::get(Continuity::G1);
    CHECK(g1.getType() == Continuity::G1);

    // follower takes the direction of the driver but keeps its own length
    auto d = g1.desiredFollower({3, 4}, {0, -10});
    CHECK(d.has_value() && near(*d, V(6, 8)));

    // degenerate follower borrows the driver's length
    d = g1.desiredFollower({0, 2}, {0, 0});
    CHECK(d.has_value() && near(*d, V(0, 2)));

    // a zero driver gives no direction to follow
    CHECK(!g1.desiredFollower({0, 0}, {1, 1}).has_value());

    CHECK(g1.isSatisfied({1, 0}, {5, 0}, 1e-3f));
    CHECK(!g1.isSatisfied({1, 0}, {-5, 0}, 1e-3f)); // opposite direction is a cusp, not G1
    CHECK(!g1.isSatisfied({1, 0}, {1, 1}, 1e-3f));
    CHECK(g1.isSatisfied({1, 0}, {0, 0}, 1e-3f));

    // the result of desiredFollower always satisfies the relation
    V driver(2, -7), follower(-3, 1);
    CHECK(g1.isSatisfied(driver, *g1.desiredFollower(driver, follower), 1e-4f));
}

static void testC1() {
    const auto& c1 = ContinuityStrategy::get(Continuity::C1);
    CHECK(c1.getType() == Continuity::C1);

    auto d = c1.desiredFollower({3, 4}, {0, -10});
    CHECK(d.has_value() && near(*d, V(3, 4)));

    CHECK(c1.isSatisfied({1, 2}, {1, 2}, 1e-3f));
    CHECK(!c1.isSatisfied({1, 2}, {2, 4}, 1e-3f)); // same direction, different length: only G1
}

static void testLineTangent() {
    Vertex a({10, 10}), b({40, 50});
    LineEdge line(&a, &b);
    CHECK(near(line.tangentAt(EdgeEnd::Start), V(30, 40)));
    CHECK(near(line.tangentAt(EdgeEnd::End), V(30, 40)));

    // adapting at the start keeps the start vertex and moves the end
    auto moved = line.adaptTangent(EdgeEnd::Start, {0, 20});
    CHECK(moved.size() == 1 && moved[0] == &b);
    CHECK(near(a.getPosition(), V(10, 10)));
    CHECK(near(b.getPosition(), V(10, 30)));

    // adapting at the end keeps the end vertex and moves the start
    moved = line.adaptTangent(EdgeEnd::End, {5, 0});
    CHECK(moved.size() == 1 && moved[0] == &a);
    CHECK(near(b.getPosition(), V(10, 30)));
    CHECK(near(a.getPosition(), V(5, 30)));
    CHECK(near(line.tangentAt(EdgeEnd::End), V(5, 0)));
}

static void testBezierTangent() {
    Vertex a({0, 0}), b({90, 0});
    BezierEdge bez(&a, &b);
    bez.getControlPoint1()->setPosition({30, 30});
    bez.getControlPoint2()->setPosition({60, 30});

    CHECK(near(bez.tangentAt(EdgeEnd::Start), V(90, 90)));
    CHECK(near(bez.tangentAt(EdgeEnd::End), V(90, -90)));

    CHECK(bez.adaptTangent(EdgeEnd::Start, {0, 30}).empty());
    CHECK(near(bez.getControlPoint1()->getPosition(), V(0, 10)));
    CHECK(near(bez.tangentAt(EdgeEnd::Start), V(0, 30)));

    CHECK(bez.adaptTangent(EdgeEnd::End, {30, 0}).empty());
    CHECK(near(bez.getControlPoint2()->getPosition(), V(80, 0)));
    CHECK(near(bez.tangentAt(EdgeEnd::End), V(30, 0)));
    CHECK(near(b.getPosition(), V(90, 0))); // vertices stay in place
}

static void testArcTangent() {
    Vertex a({0, 0}), b({100, 0});
    ArcEdge arc(&a, &b);

    // semicircle with the radius 50 and the center (50, 0); compare with a finite difference of the arc
    float r = 50.0f;
    V center(50, 0);
    float a0 = std::atan2(a.getPosition().y - center.y, a.getPosition().x - center.x);
    auto point = [&](float t) {
        float ang = a0 + static_cast<float>(M_PI) * t;
        return V(center.x + r * std::cos(ang), center.y + r * std::sin(ang));
    };
    const float h = 1e-3f;
    V numStart = (point(h) - point(0.0f)) / h;
    V numEnd = (point(1.0f) - point(1.0f - h)) / h;
    CHECK(near(arc.tangentAt(EdgeEnd::Start), numStart, 0.5f));
    CHECK(near(arc.tangentAt(EdgeEnd::End), numEnd, 0.5f));

    CHECK(arc.supports(Continuity::G0));
    CHECK(arc.supports(Continuity::G1));
    CHECK(!arc.supports(Continuity::C1));
}

// Whole round trip of the intended use: a driver tangent produces the follower tangent through the strategy
static void testStrategyWithEdges() {
    // two Bezier edges meeting at m, user dragged the control point of the first one
    Vertex p({0, 0}), m({100, 0}), q({200, 0});
    BezierEdge first(&p, &m), second(&m, &q);
    first.getControlPoint2()->setPosition({80, 40});
    second.getControlPoint1()->setPosition({130, 0});

    for (Continuity c : {Continuity::G1, Continuity::C1}) {
        const auto& strategy = ContinuityStrategy::get(c);
        V driver = first.tangentAt(EdgeEnd::End);
        V follower = second.tangentAt(EdgeEnd::Start);
        if (auto desired = strategy.desiredFollower(driver, follower))
            second.adaptTangent(EdgeEnd::Start, *desired);
        CHECK(strategy.isSatisfied(driver, second.tangentAt(EdgeEnd::Start), 1e-4f));
    }
    CHECK(near(second.tangentAt(EdgeEnd::Start), first.tangentAt(EdgeEnd::End))); // C1 ran last
}

static void testVertexMoveWithEdges() {
    Vertex a({0, 0}), b({90, 0}), c({90, 90});
    BezierEdge bez(&a, &b);
    LineEdge line(&b, &c);
    a.setNextEdge(&bez);
    b.setPrevEdge(&bez);
    b.setNextEdge(&line);
    c.setPrevEdge(&line);

    V cp2Before = bez.getControlPoint2()->getPosition();
    V cp1Before = bez.getControlPoint1()->getPosition();
    b.moveWithEdges({100, 20});
    CHECK(near(b.getPosition(), V(100, 20)));
    CHECK(near(bez.getControlPoint2()->getPosition(), cp2Before + V(10, 20))); // follows its endpoint
    CHECK(near(bez.getControlPoint1()->getPosition(), cp1Before));
}


// A - B (Bezier) - C (line) - D (Bezier) - A (line), G1 at B and C
static std::unique_ptr<Polygon> makeChain(Continuity atB, Continuity atC) {
    return PolygonBuilder()
        .addVertex(0, 0, Continuity::G0)
        .addVertex(100, 0, atB)
        .withLastEdgeAsBezier()
        .addVertex(100, 100, atC)
        .addVertex(0, 100, Continuity::G0)
        .withLastEdgeAsBezier()
        .closePolygon()
        .build();
}

static bool satisfiedAt(const Vertex& v, float eps = 1e-3f) {
    return v.getContinuityStrategy().isSatisfied(
        v.getPrevEdge()->tangentAt(EdgeEnd::End), v.getNextEdge()->tangentAt(EdgeEnd::Start), eps);
}

static PolygonEditor makeEditor() {
    static ConstraintSystem noConstraints;
    PolygonEditor editor(noConstraints);
    editor.addRule(std::make_unique<ContinuityEnforcer>());
    return editor;
}

static void testEnforceAll() {
    auto poly = makeChain(Continuity::G1, Continuity::G1);
    auto* bezier = static_cast<BezierEdge*>(poly->getEdge(0));
    CHECK(!satisfiedAt(*poly->getVertex(1))); // default control points are not tangent to the line

    V lineBefore = poly->getEdge(1)->tangentAt(EdgeEnd::Start);
    makeEditor().enforceAll(*poly);

    CHECK(satisfiedAt(*poly->getVertex(1)));
    CHECK(satisfiedAt(*poly->getVertex(2)));
    // the Bezier adapted, the line stayed where it was
    CHECK(near(poly->getEdge(1)->tangentAt(EdgeEnd::Start), lineBefore));
    CHECK(near(bezier->getControlPoint2()->getPosition().x, 100.0f)); // on the vertical line through B
}

static void testMoveVertexKeepsContinuity() {
    for (Continuity c : {Continuity::G1, Continuity::C1}) {
        auto poly = makeChain(c, c);
        PolygonEditor editor = makeEditor();
        editor.enforceAll(*poly);

        editor.moveVertex(*poly, 1, {130, 20});
        CHECK(near(poly->getVertex(1)->getPosition(), V(130, 20)));
        CHECK(satisfiedAt(*poly->getVertex(1)));
        CHECK(satisfiedAt(*poly->getVertex(2)));

        // moving the far end of the line changes its direction, the Bezier at the other end follows
        editor.moveVertex(*poly, 2, {60, 160});
        CHECK(near(poly->getVertex(2)->getPosition(), V(60, 160)));
        CHECK(satisfiedAt(*poly->getVertex(1)));
        CHECK(satisfiedAt(*poly->getVertex(2)));
    }
}

static void testDriverControlPoint() {
    auto poly = makeChain(Continuity::G1, Continuity::G1);
    PolygonEditor editor = makeEditor();
    editor.enforceAll(*poly);

    Vertex* b = poly->getVertex(1);
    Vertex* c = poly->getVertex(2);
    auto* bezier = static_cast<BezierEdge*>(poly->getEdge(0));
    float lineLength = poly->getEdge(1)->getLength();

    // user grabs the control point of A-B next to B and drags it, the line B-C has to turn around B
    auto handle = bezier->hitTestControlPoint(bezier->getControlPoint2()->getPosition(), 5.0f);
    CHECK(handle.has_value());
    CHECK(handle->edge == bezier && handle->end == EdgeEnd::End && handle->point == bezier->getControlPoint2());
    editor.moveControlPoint(*handle, {80, 30});

    CHECK(near(bezier->getControlPoint2()->getPosition(), V(80, 30))); // the dragged point follows the cursor
    CHECK(near(b->getPosition(), V(100, 0)));
    CHECK(near(poly->getEdge(1)->getLength(), lineLength));
    CHECK(satisfiedAt(*b));
    CHECK(!near(c->getPosition(), V(100, 100)));       // C had to move
    CHECK(satisfiedAt(*c));                              // and the Bezier C-D followed the line
    CHECK(near(poly->getVertex(0)->getPosition(), V(0, 0)));
    CHECK(near(poly->getVertex(3)->getPosition(), V(0, 100)));
}

static void testDriverControlPointOfStartEnd() {
    // the control point next to the START of the edge: the previous edge (a line D-A) adapts
    auto poly = makeChain(Continuity::G1, Continuity::G1);
    poly->getVertex(0)->setContinuity(Continuity::G1);
    PolygonEditor editor = makeEditor();
    editor.enforceAll(*poly);

    auto* bezier = static_cast<BezierEdge*>(poly->getEdge(0));
    auto handle = bezier->hitTestControlPoint(bezier->getControlPoint1()->getPosition(), 5.0f);
    CHECK(handle.has_value() && handle->end == EdgeEnd::Start);
    editor.moveControlPoint(*handle, {20, -40});

    CHECK(near(bezier->getControlPoint1()->getPosition(), V(20, -40)));
    CHECK(near(poly->getVertex(0)->getPosition(), V(0, 0)));
    CHECK(satisfiedAt(*poly->getVertex(0)));
    CHECK(!near(poly->getVertex(3)->getPosition(), V(0, 100))); // D moved: the line D-A rotated around A
}

static void testHitTestMiss() {
    Vertex a({0, 0}), b({90, 0});
    BezierEdge bez(&a, &b);
    CHECK(!bez.hitTestControlPoint({1000, 1000}, 5.0f).has_value());
    LineEdge line(&a, &b);
    CHECK(!line.hitTestControlPoint({0, 0}, 5.0f).has_value());
}

static void testDriverBezierBezierC1() {
    auto poly = PolygonBuilder()
        .addVertex(0, 0, Continuity::G0)
        .addVertex(100, 0, Continuity::C1)
        .withLastEdgeAsBezier()
        .addVertex(200, 0, Continuity::G0)
        .withLastEdgeAsBezier()
        .addVertex(100, 100, Continuity::G0)
        .closePolygon()
        .build();
    PolygonEditor editor = makeEditor();
    editor.enforceAll(*poly);

    auto* first = static_cast<BezierEdge*>(poly->getEdge(0));
    auto* second = static_cast<BezierEdge*>(poly->getEdge(1));
    editor.moveControlPoint(*first->hitTestControlPoint(first->getControlPoint2()->getPosition(), 5.0f), {90, -25});

    CHECK(near(second->getControlPoint1()->getPosition(), V(110, 25))); // reflection of the dragged point
    CHECK(near(first->tangentAt(EdgeEnd::End), second->tangentAt(EdgeEnd::Start)));
}

static void testG0IsLeftAlone() {
    auto poly = makeChain(Continuity::G0, Continuity::G0);
    V cpBefore = static_cast<BezierEdge*>(poly->getEdge(0))->getControlPoint2()->getPosition();
    makeEditor().enforceAll(*poly);
    CHECK(near(static_cast<BezierEdge*>(poly->getEdge(0))->getControlPoint2()->getPosition(), cpBefore));
}

static bool contains(const std::vector<Continuity>& list, Continuity c) {
    return std::find(list.begin(), list.end(), c) != list.end();
}

static std::unique_ptr<Polygon> makeSquare() {
    return PolygonBuilder()
        .addVertex(0, 0)
        .addVertex(100, 0)
        .addVertex(100, 100)
        .addVertex(0, 100)
        .closePolygon()
        .build();
}

static void testArcAnchoredAtStart() {
    Vertex a({0, 0}), b({100, 0});
    ArcEdge arc(&a, &b);
    CHECK(near(arc.geometry().radius, 50.0f)); // no anchor: semicircle

    arc.adaptTangent(EdgeEnd::Start, {1, 1});
    auto g = arc.geometry();
    CHECK(!g.straight);
    CHECK(near(g.radius, 100.0f / (2.0f * std::sin(M_PI / 4)), 0.01f));
    CHECK(near(arc.pointAt(g, 0.0f), a.getPosition()));
    CHECK(near(arc.pointAt(g, 1.0f), b.getPosition()));
    CHECK(near(vec::cross(vec::normalized(arc.tangentAt(EdgeEnd::Start)), vec::normalized(V(1, 1))), 0.0f, 1e-4f));
    CHECK(vec::dot(arc.tangentAt(EdgeEnd::Start), V(1, 1)) > 0.0f);

    // the arc leaves a in the direction (1, 1) and curves to the right (towards the chord)
    CHECK(arc.pointAt(g, 0.01f).y > 0.0f);

    arc.releaseTangent(EdgeEnd::End); // another end: nothing happens
    CHECK(near(arc.geometry().radius, g.radius));
    arc.releaseTangent(EdgeEnd::Start);
    CHECK(near(arc.geometry().radius, 50.0f));
}

static void testArcAnchoredAtEnd() {
    Vertex a({0, 0}), b({100, 0});
    ArcEdge arc(&a, &b);

    arc.adaptTangent(EdgeEnd::End, {1, -1});
    auto g = arc.geometry();
    CHECK(!g.straight);
    CHECK(near(arc.pointAt(g, 0.0f), a.getPosition()));
    CHECK(near(arc.pointAt(g, 1.0f), b.getPosition()));
    CHECK(near(vec::cross(vec::normalized(arc.tangentAt(EdgeEnd::End)), vec::normalized(V(1, -1))), 0.0f, 1e-4f));
    CHECK(vec::dot(arc.tangentAt(EdgeEnd::End), V(1, -1)) > 0.0f);

    // the arc keeps the tangent given at the end also when the other vertex moves
    a.setPosition({-40, 30});
    g = arc.geometry();
    CHECK(near(arc.pointAt(g, 0.0f), a.getPosition()));
    CHECK(near(arc.pointAt(g, 1.0f), b.getPosition()));
    CHECK(near(vec::cross(vec::normalized(arc.tangentAt(EdgeEnd::End)), vec::normalized(V(1, -1))), 0.0f, 1e-4f));
}

static void testArcTangentMatchesFiniteDifference() {
    for (EdgeEnd anchorEnd : {EdgeEnd::Start, EdgeEnd::End}) {
        Vertex a({10, 20}), b({120, 60});
        ArcEdge arc(&a, &b);
        arc.adaptTangent(anchorEnd, {3, -2});
        auto g = arc.geometry();
        const float h = 1e-3f;
        V numStart = (arc.pointAt(g, h) - arc.pointAt(g, 0.0f)) / h;
        V numEnd = (arc.pointAt(g, 1.0f) - arc.pointAt(g, 1.0f - h)) / h;
        CHECK(near(arc.tangentAt(EdgeEnd::Start), numStart, 0.5f));
        CHECK(near(arc.tangentAt(EdgeEnd::End), numEnd, 0.5f));
    }
}

static void testArcStraightWhenTangentAlongChord() {
    Vertex a({0, 0}), b({100, 0});
    ArcEdge arc(&a, &b);
    arc.adaptTangent(EdgeEnd::Start, {5, 0});
    CHECK(arc.geometry().straight);
    CHECK(near(arc.tangentAt(EdgeEnd::Start), V(100, 0)));
    CHECK(near(arc.distanceToPoint({50, 7}), 7.0f, 0.01f));
}

static void testArcDistanceToPoint() {
    Vertex a({0, 0}), b({100, 0});
    ArcEdge arc(&a, &b);
    // the semicircle bulges to the -y side: (50, -50) lies on it
    CHECK(near(arc.distanceToPoint({50, -50}), 0.0f, 0.5f));
    CHECK(near(arc.distanceToPoint({50, -60}), 10.0f, 0.5f));
}

static void testPolicyAllowed() {
    auto square = makeSquare();
    Vertex* v = square->getVertex(1);
    CHECK(ContinuityPolicy::allowed(*v) == std::vector<Continuity>{Continuity::G0}); // between two segments

    auto chain = makeChain(Continuity::G1, Continuity::G1);
    CHECK(ContinuityPolicy::allowed(*chain->getVertex(1)) ==
          (std::vector<Continuity>{Continuity::G0, Continuity::G1, Continuity::C1})); // Bezier - segment
}

static void testPolicyWithArcs() {
    auto square = makeSquare();
    PolygonEditor editor = makeEditor();
    editor.toggleEdgeType(*square, 0, EdgeType::Arc); // A - B
    Vertex* a = square->getVertex(0);
    Vertex* b = square->getVertex(1);

    CHECK(a->getContinuity() == Continuity::G1);  // the first end of an arc gets the default
    CHECK(b->getContinuity() == Continuity::G0);  // the second one cannot be G1 as well
    CHECK(!contains(ContinuityPolicy::allowed(*b), Continuity::G1));
    CHECK(!contains(ContinuityPolicy::allowed(*a), Continuity::C1)); // arcs have no C1
    CHECK(satisfiedAt(*a));

    // a second arc next to the first one: the vertex between two arcs cannot be G1
    editor.toggleEdgeType(*square, 1, EdgeType::Arc); // B - C
    CHECK(ContinuityPolicy::allowed(*b) == std::vector<Continuity>{Continuity::G0});
    CHECK(square->getVertex(2)->getContinuity() == Continuity::G1); // the free end of the second arc
}

static void testSanitizeKeepsExplicitChoice() {
    auto square = makeSquare();
    Vertex* b = square->getVertex(1);
    b->setContinuity(Continuity::C1);
    b->hasExplicitContinuity = true;
    PolygonEditor editor = makeEditor();

    // C1 is fine next to a Bezier curve and survives
    editor.toggleEdgeType(*square, 0, EdgeType::Bezier);
    CHECK(b->getContinuity() == Continuity::C1);
    CHECK(satisfiedAt(*b));

    // an arc does not have C1, the explicit choice is downgraded to G1 and it wins the G1 end over the default of A
    editor.toggleEdgeType(*square, 0, EdgeType::Arc);
    CHECK(b->getContinuity() == Continuity::G1);
    CHECK(square->getVertex(0)->getContinuity() == Continuity::G0);
    CHECK(satisfiedAt(*b));

    // back to segments: nothing to be continuous, the vertex remembers the choice
    editor.toggleEdgeType(*square, 0, EdgeType::Line);
    CHECK(b->getContinuity() == Continuity::G1);
    CHECK(ContinuityPolicy::effective(*b) == Continuity::G0);
}

static void testToggleToBezierAlignsControlPoints() {
    auto square = makeSquare();
    PolygonEditor editor = makeEditor();
    editor.toggleEdgeType(*square, 0, EdgeType::Bezier);

    CHECK(square->getVertex(0)->getContinuity() == Continuity::G1);
    CHECK(square->getVertex(1)->getContinuity() == Continuity::G1);
    CHECK(satisfiedAt(*square->getVertex(0)));
    CHECK(satisfiedAt(*square->getVertex(1)));
    // the segments around did not move, the new curve adapted
    CHECK(near(square->getVertex(3)->getPosition(), V(0, 100)));
    CHECK(near(square->getVertex(2)->getPosition(), V(100, 100)));
}

static void testArcFollowsNeighbourWhenMoved() {
    auto square = makeSquare();
    PolygonEditor editor = makeEditor();
    editor.toggleEdgeType(*square, 0, EdgeType::Arc);
    Vertex* a = square->getVertex(0);

    editor.moveVertex(*square, 3, {-60, 120}); // turns the segment D - A, which drives the arc at A
    CHECK(satisfiedAt(*a));
    editor.moveVertex(*square, 1, {140, 30});  // the free end of the arc
    CHECK(satisfiedAt(*a));
    editor.moveVertex(*square, 0, {10, -20});  // the anchored vertex itself
    CHECK(satisfiedAt(*a));
}

static void testSplitAndRemove() {
    auto poly = makeChain(Continuity::G1, Continuity::G1); // A - B (Bezier) - C - D (Bezier) - A
    PolygonEditor editor = makeEditor();
    editor.enforceAll(*poly);

    editor.splitEdgeAtMidpoint(*poly, 0); // splits the Bezier edge A - B
    CHECK(poly->getVertexCount() == 5);
    CHECK(poly->getEdgeCount() == 5);
    Vertex* middle = poly->getVertex(1);
    CHECK(near(middle->getPosition(), V(50, 0)));
    CHECK(ContinuityPolicy::effective(*middle) == Continuity::G0); // between two segments
    // B is now between a segment and the segment B - C: nothing to enforce, and nothing crashes
    CHECK(!poly->getEdge(0)->isCurved() && !poly->getEdge(1)->isCurved());

    // removing a vertex joins its neighbours with a segment
    CHECK(editor.removeVertex(*poly, 1));
    CHECK(poly->getVertexCount() == 4);
    CHECK(poly->getEdgeCount() == 4);
    CHECK(satisfiedAt(*poly->getVertex(2)));

    // a triangle cannot lose a vertex
    auto triangle = PolygonBuilder().addVertex(0, 0).addVertex(10, 0).addVertex(0, 10).closePolygon().build();
    CHECK(!editor.removeVertex(*triangle, 0));
    CHECK(triangle->getVertexCount() == 3);
}

static void testRemoveVertexNextToCurve() {
    auto poly = makeChain(Continuity::G1, Continuity::G1); // C is between a segment and the Bezier C - D
    PolygonEditor editor = makeEditor();
    editor.enforceAll(*poly);

    CHECK(editor.removeVertex(*poly, 2)); // removes C: B - D becomes a segment, B and D are re-checked
    Vertex* b = poly->getVertex(1);
    CHECK(!poly->getEdge(1)->isCurved());
    CHECK(satisfiedAt(*b));
    CHECK(ContinuityPolicy::effective(*poly->getVertex(2)) == Continuity::G0); // D: segment, segment
}

static void testSetContinuity() {
    auto square = makeSquare();
    PolygonEditor editor = makeEditor();
    Vertex* a = square->getVertex(0);
    Vertex* b = square->getVertex(1);

    // between two segments there is nothing to choose
    CHECK(!editor.setContinuity(*b, Continuity::G1));
    CHECK(!b->hasExplicitContinuity);

    // Bezier curve: C1 is allowed, the choice is explicit and gets enforced right away
    editor.toggleEdgeType(*square, 0, EdgeType::Bezier);
    CHECK(editor.setContinuity(*b, Continuity::C1));
    CHECK(b->getContinuity() == Continuity::C1 && b->hasExplicitContinuity);
    CHECK(near(static_cast<BezierEdge*>(square->getEdge(0))->tangentAt(EdgeEnd::End), square->getEdge(1)->tangentAt(EdgeEnd::Start)));

    // G0 lets the curve go
    CHECK(editor.setContinuity(*b, Continuity::G0));
    static_cast<BezierEdge*>(square->getEdge(0))->getControlPoint2()->setPosition({60, 40});
    editor.enforceAll(*square);
    CHECK(near(static_cast<BezierEdge*>(square->getEdge(0))->getControlPoint2()->getPosition(), V(60, 40)));

    // arc: G1 at A is anchored, G0 releases the anchor and the arc is a semicircle again
    // (D is moved first, because the tangent of the square is perpendicular to the chord and gives a semicircle anyway)
    editor.moveVertex(*square, 3, {-40, 100});
    editor.toggleEdgeType(*square, 0, EdgeType::Arc);
    auto* arc = static_cast<ArcEdge*>(square->getEdge(0));
    CHECK(a->getContinuity() == Continuity::G1);
    CHECK(!near(arc->geometry().radius, 50.0f));
    CHECK(editor.setContinuity(*a, Continuity::G0));
    CHECK(near(arc->geometry().radius, 50.0f));
    CHECK(a->hasExplicitContinuity);

    // now B may take over the only G1 end of the arc, and A may not any more
    CHECK(editor.setContinuity(*b, Continuity::G1));
    CHECK(!editor.setContinuity(*a, Continuity::G1));
    CHECK(satisfiedAt(*b));
    CHECK(!editor.setContinuity(*b, Continuity::C1)); // no C1 on arcs
    CHECK(b->getContinuity() == Continuity::G1);
}

int main() {
    testG0();
    testG1();
    testC1();
    testLineTangent();
    testBezierTangent();
    testArcTangent();
    testStrategyWithEdges();
    testVertexMoveWithEdges();
    testEnforceAll();
    testMoveVertexKeepsContinuity();
    testDriverControlPoint();
    testDriverControlPointOfStartEnd();
    testHitTestMiss();
    testDriverBezierBezierC1();
    testG0IsLeftAlone();
    testArcAnchoredAtStart();
    testArcAnchoredAtEnd();
    testArcTangentMatchesFiniteDifference();
    testArcStraightWhenTangentAlongChord();
    testArcDistanceToPoint();
    testPolicyAllowed();
    testPolicyWithArcs();
    testSanitizeKeepsExplicitChoice();
    testToggleToBezierAlignsControlPoints();
    testArcFollowsNeighbourWhenMoved();
    testSplitAndRemove();
    testRemoveVertexNextToCurve();
    testSetContinuity();

    if (failures == 0)
        std::printf("all continuity tests passed\n");
    return failures == 0 ? 0 : 1;
}
