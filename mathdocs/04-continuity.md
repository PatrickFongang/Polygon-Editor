# 4. Continuity at the vertices

Code: [continuity_strategy.hpp](../src/model/continuity_strategy.hpp),
[continuity_policy.cpp](../src/model/continuity_policy.cpp),
[continuity_enforcer.cpp](../src/editing/continuity_enforcer.cpp) and the `tangentAt` / `adaptTangent` methods of the
edges.

## Definitions

Let $V$ be a vertex, $\mathbf{T}_{\mathrm{in}} = E_{\mathrm{prev}}'(1)$ the tangent of the edge arriving at $V$ and
$\mathbf{T}_{\mathrm{out}} = E_{\mathrm{next}}'(0)$ the tangent of the edge leaving it (both oriented along the
traversal, see [01-conventions.md](01-conventions.md)).

| Class | Meaning | Condition |
|---|---|---|
| **G0** | continuity of position | the edges share the vertex, so it always holds; **no** condition on the tangents |
| **G1** | continuity of the unit tangent (direction) | $\hat{\mathbf{T}}_{\mathrm{in}} = \hat{\mathbf{T}}_{\mathrm{out}}$, i.e. $\mathbf{T}_{\mathrm{in}}\times\mathbf{T}_{\mathrm{out}} = 0$ and $\mathbf{T}_{\mathrm{in}}\cdot\mathbf{T}_{\mathrm{out}} > 0$ |
| **C1** | continuity of the tangent vector | $\mathbf{T}_{\mathrm{in}} = \mathbf{T}_{\mathrm{out}}$ |

Each class implies the weaker ones: $C^1 \Rightarrow G^1 \Rightarrow G^0$ (for non-zero tangents). So when a vertex
changes from G1 to G0 nothing has to be repaired on the vertex: a G1 vertex already satisfies G0.

The opposite direction, $\mathbf{T}_{\mathrm{in}} = -\lambda\mathbf{T}_{\mathrm{out}}$ with $\lambda>0$, is a **cusp**
(the curve turns back); it is parallel but **not** G1, which is why the test includes the sign condition.

C1 depends on the parametrisation. Here every edge is parametrised on $[0, 1]$ regardless of its length, so C1 between
a long and a short edge forces their *derivative vectors* to be equal, not their speeds in length units.

The enforcer checks the relations with a tolerance $\varepsilon = 10^{-3}$
(`ContinuityStrategy::isSatisfied`): for G1, $|\hat{\mathbf{T}}_{\mathrm{in}}\times\hat{\mathbf{T}}_{\mathrm{out}}| \le \varepsilon$ (the sine of the angle between the
tangents, about $0.06^\circ$); for C1, $\Vert\mathbf{T}_{\mathrm{in}} - \mathbf{T}_{\mathrm{out}}\Vert \le \varepsilon$.

## Repairing a broken vertex: the follower

When the relation is broken, one of the two edges - the **follower** - is changed so that its tangent at $V$ becomes

$$
\mathbf{T}_{\mathrm{follower}}' = f(\mathbf{T}_{\mathrm{driver}}, \mathbf{T}_{\mathrm{follower}})
$$

where $f$ is defined by the class (`ContinuityStrategy::desiredFollower`):

$$
f_{G0} = \text{(no change)}, \qquad
f_{G1} = \Vert\mathbf{T}_{\mathrm{follower}}\Vert\,\hat{\mathbf{T}}_{\mathrm{driver}}, \qquad
f_{C1} = \mathbf{T}_{\mathrm{driver}} .
$$

For G1 the follower keeps its own length (so the "shape" of the follower changes as little as possible). If the
follower's tangent is zero it borrows $\Vert\mathbf{T}_{\mathrm{driver}}\Vert$; if the driver's tangent is zero there is no
direction to follow and nothing is done.

Both tangents are oriented along the traversal, so the same formula works whether the follower is the next edge
(adapted at its start) or the previous edge (adapted at its end). How a follower realises $\mathbf{T}'$ depends on its
type:

| Follower | Realisation of the new tangent $\mathbf{T}'$ at the vertex $V$ |
|---|---|
| **Bézier**, next edge | $P_1 = V + \frac13\mathbf{T}'$ (control point next to $V$) |
| **Bézier**, previous edge | $P_2 = V - \frac13\mathbf{T}'$ |
| **segment** $VW$, next edge | $W' = V + \mathbf{T}'$: the other end moves; the segment **turns around $V$** (for G1 the length is unchanged) |
| **segment** $UV$, previous edge | $U' = V - \mathbf{T}'$ |
| **arc** | remembers the **anchor** direction $\hat{\mathbf{T}}'$ at that end; the arc is recomputed from it ([03-arc.md](03-arc.md)) |

Examples:

* **G1, Bézier follows a segment.** Segment $UV$ is the driver, the Bézier starts at $V$:
  $\mathbf{T}_{\mathrm{in}} = V - U$ and $P_1' = V + \Vert P_1 - V\Vert\,\hat{V - U}$ - the control point is moved onto the
  line of the segment, keeping its distance from $V$.
* **C1 between two Béziers.** With $B_1'(1) = 3(V - P_2)$ and $B_2'(0) = 3(P_1 - V)$ the condition
  $\mathbf{T}_{\mathrm{in}} = \mathbf{T}_{\mathrm{out}}$ gives $P_1 = 2V - P_2$: the control points are mirror images of each other through $V$.
* **C1, Bézier and segment.** $P_1 = V + \frac13(W - V)$: the control point lies one third along the segment.

Note that **an arc cannot take part in C1**. The length of its tangent is the arc length $r|s|$, which is determined by
the geometry, so only the *direction* can be imposed. This is why the arc supports only G0 and G1
(`Edge::supports`).

## Who follows

* The edge the user is dragging (a Bézier control point) is always the **driver**.
* Otherwise the edge with the **lower `adaptCost`** follows:

$$
\text{arc } (0) \;<\; \text{Bézier } (1) \;<\; \text{segment } (2) .
$$

An arc is the cheapest follower (only its anchor changes, no vertex moves), a Bézier moves one control point and a
segment is the last resort because turning it moves a vertex, which may break other relations. If the costs are equal
the next edge follows.

Dragging a vertex needs no driver: the control points travel with the vertex (`Vertex::moveWithEdges`), so only the
neighbours that really changed direction (a segment rotated by the move) have to be followed, and the Bézier or arc next
to such a segment gives way.

## Propagation

An adaptation can move a vertex (a turned segment), which changes the tangents of the other edges at that vertex, so
the check has to be repeated there. The algorithm is a work queue (`ContinuityEnforcer::apply`):

```text
queue := seed vertices (the moved vertex and its neighbours)
while queue is not empty and steps < 1000:
    V := pop(queue)
    if the relation at V holds: continue
    choose the follower; adapt it to the driver
    for each vertex M moved by the adaptation:
        push M and the neighbours of M
```

It terminates quickly because the tangent of an edge depends on few vertices:

* a **Bézier** tangent at one end depends only on its control point there, so moving the other end changes nothing;
* a **segment** tangent depends on both ends, so a turned segment really changes the neighbouring vertex - this is
  the only way the repair propagates, along chains of segments that are followers;
* an **arc** has a prescribed tangent (the anchor) at one end only. Moving a vertex does change the tangent at its
  other end, but that end is G0 by the rules below, so nothing is required of it and nothing has to be repaired.

A hard limit of 1000 steps protects against situations that have no solution.

## Which classes are allowed (`ContinuityPolicy`)

* Continuity matters only where **at least one adjacent edge is curved**. Between two segments the vertex is G0 (the
  stored choice is kept for later).
* The class must be supported by **both** edges: an arc has no C1.
* An arc has **one** prescribed tangent. G1 is therefore refused at a vertex if the opposite end of an adjacent arc is
  already G1. Two arcs meeting at a vertex cannot be G1 either: each would take its only tangent from the other, a
  circular definition with a free parameter (the common tangent direction).
* A vertex whose arc loses its only G1 end (G1 $\to$ G0) makes the arc a **semicircle** again, as the specification
  requires for an arc with G0 at both ends. This is the one case where lowering the class changes geometry.

Defaults (`ContinuityPolicy::sanitize`): after an edge changes, vertices without an explicit user choice get the default
class (G1), the explicit ones are only lowered when they became impossible, and the first of the vertices (or an
explicit one) wins when only one end of an arc may be G1.
