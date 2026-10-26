// Unit tests of the constraint model: the projection of each constraint and the ConstraintSystem.
// No window or OpenGL context is created.
#include "test_common.hpp"
#include "model/vertex.hpp"
#include "model/edge/line_edge.hpp"
#include "model/edge/bezier_edge.hpp"
#include "model/constraint/constraint_system.hpp"
#include "model/constraint/length_constraint.hpp"
#include "model/constraint/parallel_constraint.hpp"

static float length(const Edge& e) { return e.getLength(); }

// angle of the edge as a line, in [0, pi)
static float lineAngle(const Edge& e) {
    V d = e.getVertex2()->getPosition() - e.getVertex1()->getPosition();
    float angle = std::atan2(d.y, d.x);
    return angle < 0.0f ? angle + static_cast<float>(M_PI) : angle;
}

static void testLengthFreeEnds() {
    Vertex a({0, 0}), b({30, 40});
    LineEdge edge(&a, &b);
    LengthConstraint constraint(&edge, 100.0f);

    CHECK(!constraint.isSatisfied(1e-3f));
    auto moved = constraint.project({});
    CHECK(moved.size() == 2);
    CHECK(near(length(edge), 100.0f));
    CHECK(near((a.getPosition() + b.getPosition()) * 0.5f, V(15, 20))); // both ends share the correction
    CHECK(constraint.isSatisfied(1e-3f));
    CHECK(constraint.project({}).empty()); // already satisfied
}

static void testLengthPinnedEnd() {
    Vertex a({0, 0}), b({30, 40});
    LineEdge edge(&a, &b);
    LengthConstraint constraint(&edge, 100.0f);

    auto moved = constraint.project({&a});
    CHECK(moved.size() == 1 && moved[0] == &b);
    CHECK(near(a.getPosition(), V(0, 0)));
    CHECK(near(b.getPosition(), V(60, 80))); // the free end slides along the edge

    // shortening works the same way, the other end pinned
    LengthConstraint shorter(&edge, 50.0f);
    shorter.project({&b});
    CHECK(near(b.getPosition(), V(60, 80)));
    CHECK(near(length(edge), 50.0f));
}

static void testLengthBothPinnedOrDegenerate() {
    Vertex a({0, 0}), b({30, 40});
    LineEdge edge(&a, &b);
    LengthConstraint constraint(&edge, 100.0f);
    CHECK(constraint.project({&a, &b}).empty());
    CHECK(near(b.getPosition(), V(30, 40)));

    Vertex c({5, 5}), d({5, 5}); // zero length: any direction will do
    LineEdge point(&c, &d);
    LengthConstraint grow(&point, 10.0f);
    grow.project({});
    CHECK(near(length(point), 10.0f));
}

static void testLengthMovesBezierControlPointsWithEndpoint() {
    Vertex a({0, 0}), b({30, 40}), c({100, 100});
    LineEdge line(&a, &b);
    BezierEdge bezier(&b, &c);
    a.setNextEdge(&line);
    b.setPrevEdge(&line);
    b.setNextEdge(&bezier);
    c.setPrevEdge(&bezier);

    V cp1Before = bezier.getControlPoint1()->getPosition();
    LengthConstraint constraint(&line, 100.0f);
    constraint.project({&a});
    // b moved by (30, 40), and the control point next to it with it
    CHECK(near(bezier.getControlPoint1()->getPosition(), cp1Before + V(30, 40)));
}

static void testParallelFreeEdges() {
    Vertex a({0, 0}), b({100, 0}), c({0, 50}), d({100, 80});
    LineEdge e1(&a, &b), e2(&c, &d);
    ParallelConstraint constraint(&e1, &e2);

    CHECK(!constraint.isSatisfied(1e-3f));
    float len1 = length(e1), len2 = length(e2);
    V mid1 = (a.getPosition() + b.getPosition()) * 0.5f;
    V mid2 = (c.getPosition() + d.getPosition()) * 0.5f;

    auto moved = constraint.project({});
    CHECK(moved.size() == 4);
    CHECK(constraint.isSatisfied(1e-4f));
    CHECK(near(length(e1), len1) && near(length(e2), len2)); // rotation only
    CHECK(near((a.getPosition() + b.getPosition()) * 0.5f, mid1)); // both turn around their midpoints
    CHECK(near((c.getPosition() + d.getPosition()) * 0.5f, mid2));
    // each turned by half of the angle between them (atan2(30, 100) / 2)
    float half = std::atan2(30.0f, 100.0f) / 2.0f;
    CHECK(near(lineAngle(e1), half, 1e-4f));
    CHECK(near(lineAngle(e2), std::atan2(30.0f, 100.0f) - half, 1e-4f));
    CHECK(constraint.project({}).empty());
}

static void testParallelPinnedEnd() {
    Vertex a({0, 0}), b({100, 0}), c({0, 50}), d({100, 80});
    LineEdge e1(&a, &b), e2(&c, &d);
    ParallelConstraint constraint(&e1, &e2);

    // c is pinned: the second edge turns around it, the first one around its midpoint
    constraint.project({&c});
    CHECK(near(c.getPosition(), V(0, 50)));
    CHECK(constraint.isSatisfied(1e-4f));
    CHECK(near(length(e2), std::hypot(100.0f, 30.0f)));
    CHECK(near((a.getPosition() + b.getPosition()) * 0.5f, V(50, 0)));
}

static void testParallelRigidEdge() {
    Vertex a({0, 0}), b({100, 0}), c({0, 50}), d({100, 80});
    LineEdge e1(&a, &b), e2(&c, &d);
    ParallelConstraint constraint(&e1, &e2);

    // the second edge has both ends pinned: the first one has to turn by the whole angle
    auto moved = constraint.project({&c, &d});
    CHECK(near(c.getPosition(), V(0, 50)) && near(d.getPosition(), V(100, 80)));
    CHECK(moved.size() == 2);
    CHECK(constraint.isSatisfied(1e-4f));
    CHECK(near(lineAngle(e1), std::atan2(30.0f, 100.0f), 1e-4f));
    CHECK(near((a.getPosition() + b.getPosition()) * 0.5f, V(50, 0)));

    // nothing can be done if both are rigid
    Vertex p({0, 0}), q({100, 0}), r({0, 50}), s({100, 80});
    LineEdge f1(&p, &q), f2(&r, &s);
    CHECK(ParallelConstraint(&f1, &f2).project({&p, &q, &r, &s}).empty());
}

static void testParallelOppositeDirections() {
    // the second edge points the other way and is slightly off: parallel lines, no half turn needed
    Vertex a({0, 0}), b({100, 0}), c({100, 50}), d({0, 60});
    LineEdge e1(&a, &b), e2(&c, &d);
    ParallelConstraint constraint(&e1, &e2);

    CHECK(std::abs(constraint.angleError()) < 0.2f);
    constraint.project({});
    CHECK(constraint.isSatisfied(1e-4f));
    CHECK(std::abs(lineAngle(e1)) < 0.2f || std::abs(lineAngle(e1) - static_cast<float>(M_PI)) < 0.2f);

    // already parallel (also when opposite): nothing moves
    Vertex p({0, 0}), q({100, 0}), r({100, 10}), s({0, 10});
    LineEdge f1(&p, &q), f2(&r, &s);
    CHECK(ParallelConstraint(&f1, &f2).project({}).empty());
}

static void testParallelAdjacentEdgesConverge() {
    // two edges sharing a vertex: the sequential rotation is approximate, a few passes are enough
    Vertex a({0, 0}), b({100, 0}), c({150, 70});
    LineEdge e1(&a, &b), e2(&b, &c);
    ParallelConstraint constraint(&e1, &e2);

    int passes = 0;
    while (!constraint.isSatisfied(1e-3f) && passes < 50) {
        constraint.project({});
        ++passes;
    }
    CHECK(constraint.isSatisfied(1e-3f));
}

static void testParallelDegenerateEdge() {
    Vertex a({0, 0}), b({100, 0}), c({5, 5}), d({5, 5});
    LineEdge e1(&a, &b), e2(&c, &d);
    ParallelConstraint constraint(&e1, &e2);
    CHECK(constraint.isSatisfied(1e-4f)); // no direction, nothing to satisfy
    CHECK(constraint.project({}).empty());
}

static void testConstraintSystem() {
    Vertex a({0, 0}), b({100, 0}), c({0, 50}), d({100, 80}), e({50, 200});
    LineEdge e1(&a, &b), e2(&c, &d), e3(&d, &e);

    ConstraintSystem system;
    auto* length1 = system.add(std::make_unique<LengthConstraint>(&e1, 100.0f));
    auto* length2 = system.add(std::make_unique<LengthConstraint>(&e2, 50.0f));
    auto* parallel = system.add(std::make_unique<ParallelConstraint>(&e1, &e2));
    CHECK(system.size() == 3);

    CHECK(length1->involves(&e1) && !length1->involves(&e2));
    CHECK(parallel->involves(&e1) && parallel->involves(&e2) && !parallel->involves(&e3));
    CHECK(system.constraintsOf(&e1).size() == 2); // several constraints on one edge
    CHECK(system.constraintsOf(&e2).size() == 2);
    CHECK(system.constraintsOf(&e3).empty());

    CHECK(system.remove(length2));
    CHECK(!system.remove(length2)); // gone already
    CHECK(system.size() == 2);

    // removing one edge takes the constraints with it, also the ones that join it with other edges
    system.removeInvolving(&e2);
    CHECK(system.size() == 1);
    CHECK(system.all()[0].get() == length1);
    CHECK(system.constraintsOf(&e2).empty());

    system.clear();
    CHECK(system.size() == 0);
}

static void testLabels() {
    Vertex a({0, 0}), b({100, 0});
    LineEdge e(&a, &b);
    CHECK(LengthConstraint(&e, 123.4f).label() == "L=123");
    CHECK(ParallelConstraint(&e, &e).label() == "||");
}

int main() {
    testLengthFreeEnds();
    testLengthPinnedEnd();
    testLengthBothPinnedOrDegenerate();
    testLengthMovesBezierControlPointsWithEndpoint();
    testParallelFreeEdges();
    testParallelPinnedEnd();
    testParallelRigidEdge();
    testParallelOppositeDirections();
    testParallelAdjacentEdgesConverge();
    testParallelDegenerateEdge();
    testConstraintSystem();
    testLabels();

    if (failures == 0)
        std::printf("all constraint tests passed\n");
    return failures == 0 ? 0 : 1;
}
