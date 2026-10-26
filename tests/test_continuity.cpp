// Unit tests of the continuity math: strategies (pure vector math) and the tangent interface of edges.
// No window or OpenGL context is created.
#include <cmath>
#include <cstdio>
#include "model/continuity_strategy.hpp"
#include "model/vertex.hpp"
#include "model/edge/line_edge.hpp"
#include "model/edge/bezier_edge.hpp"
#include "model/edge/arc_edge.hpp"
#include "model/polygon.hpp"
#include "builder/polygon_builder.hpp"
#include "editing/polygon_editor.hpp"
#include "editing/continuity_enforcer.hpp"

static int failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);      \
            ++failures;                                                      \
        }                                                                    \
    } while (0)

static bool near(sf::Vector2f a, sf::Vector2f b, float eps = 1e-3f) { return vec::length(a - b) <= eps; }
static bool near(float a, float b, float eps = 1e-3f) { return std::abs(a - b) <= eps; }

using V = sf::Vector2f;

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
    PolygonEditor editor;
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

    if (failures == 0)
        std::printf("all continuity tests passed\n");
    return failures == 0 ? 0 : 1;
}
