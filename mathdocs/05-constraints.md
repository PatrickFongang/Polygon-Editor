# 5. Geometric constraints

Code: [length_constraint.cpp](../src/model/constraint/length_constraint.cpp),
[parallel_constraint.cpp](../src/model/constraint/parallel_constraint.cpp),
[constraint_solver.cpp](../src/editing/constraint_solver.cpp),
[polygon_editor.cpp](../src/editing/polygon_editor.cpp).

## Idea: projection

A constraint is an equation $C(\mathbf{x}) = 0$ on the positions $\mathbf{x}$ of the vertices. After an edit it is
usually violated, and the solver moves the vertices as little as possible to satisfy it again: it **projects** the
current state onto the set $C = 0$. Several constraints are handled one after another, each projection possibly
disturbing the previous ones, until all hold (the *Gauss-Seidel* iteration, the same scheme as in position-based
dynamics).

**Pinned vertices.** The vertex the user holds must stay under the cursor. Every vertex $i$ has a weight $w_i$ (an
inverse mass): $w_i = 1$ for a free vertex and $w_i = 0$ for a pinned one. A projection distributes the correction in
proportion to the weights, so a pinned vertex never moves and the rest of the system adapts to it.

## Fixed length

Constraint on an edge $AB$ with the required length $L$:

$$
C = \Vert B - A\Vert - L .
$$

With $d = \Vert B - A\Vert$ and $\hat{\mathbf{u}} = (B - A)/d$ the projection moves the ends along the edge:

$$
\Delta A = +\frac{w_A}{w_A + w_B}\,(d - L)\,\hat{\mathbf{u}}, \qquad
\Delta B = -\frac{w_B}{w_A + w_B}\,(d - L)\,\hat{\mathbf{u}} .
$$

Then $B' - A' = (B - A) - (d - L)\hat{\mathbf{u}}\,\frac{w_A + w_B}{w_A + w_B} = L\,\hat{\mathbf{u}}$, so the constraint holds exactly after
one step, and the displacement is the smallest possible for the given weights (the correction is shared in proportion
to the weights and the edge keeps its direction).

* Both ends free: each moves by half of $(d - L)$.
* One end pinned: the other end slides along the edge.
* Both pinned ($w_A + w_B = 0$): nothing can move, the constraint stays unsatisfied.
* $d = 0$ (the direction is undefined): the direction $(1, 0)$ is used.

Example: $A = (0, 0)$ pinned, $B = (30, 40)$, $L = 100$: $d = 50$, $\hat{\mathbf{u}} = (0.6, 0.8)$,
$\Delta B = -(50 - 100)(0.6, 0.8) = (30, 40)$, so $B' = (60, 80)$.

## Parallelism

Two edges with the directions $\mathbf{d}_1$ and $\mathbf{d}_2$ lie on parallel lines when the angle between the *lines*
is zero. The angle from $\mathbf{d}_1$ to $\mathbf{d}_2$ is

$$
\theta = \mathrm{atan2}(\mathbf{d}_1\times\mathbf{d}_2,\ \mathbf{d}_1\cdot\mathbf{d}_2) \in (-\pi, \pi],
$$

and since opposite directions are parallel as well, it is wrapped modulo $\pi$ to the smallest equivalent rotation:

$$
\theta \leftarrow \begin{cases} \theta - \pi & \theta > \pi/2, \\ \theta + \pi & \theta \le -\pi/2, \\ \theta & \text{otherwise,} \end{cases}
\qquad \theta \in (-\pi/2, \pi/2] .
$$

($C = \theta$ is the constraint function; for a zero-length edge there is no direction and $\theta = 0$.)

**Distributing the rotation.** Rotating $\mathbf{d}_1$ by $\varphi_1$ and $\mathbf{d}_2$ by $\varphi_2$ changes the angle between them to
$\theta + \varphi_2 - \varphi_1$. Each edge gets a weight $w_k = 0$ if **both** its ends are pinned (a rigid edge) and
$w_k = 1$ otherwise, and the shares $s_k = w_k / (w_1 + w_2)$. Choosing

$$
\varphi_1 = +s_1\,\theta, \qquad \varphi_2 = -s_2\,\theta
$$

makes the new angle $\theta - s_2\theta - s_1\theta = 0$. If both edges are free they turn by $\theta/2$ each, if one is rigid
the other turns by the whole $\theta$, and if both are rigid nothing can be done.

**Pivot.** An edge $PQ$ is rotated by $\varphi$ around a pivot $O$: $X' = O + R(\varphi)(X - O)$ for every end that is
not pinned ($R$ from [01-conventions.md](01-conventions.md)). The pivot is

* the **pinned end**, if exactly one end is pinned (that end must stay in place, the other one swings),
* the **midpoint** $(P + Q)/2$ otherwise - a rotation around the centre keeps the midpoint and the length and moves the
  ends the least.

A rotation preserves the length of an edge, so parallelism does not break a length constraint on the same edge (up to the
shared-vertex case below).

Example (as in the tests): $e_1 = (0,0)\to(100,0)$, $e_2 = (0,50)\to(100,80)$. $\theta = \mathrm{atan2}(30, 100) \approx 16.7^\circ$
and each edge turns by $\approx 8.35^\circ$ around its midpoint.

**Edges sharing a vertex.** The two rotations are applied one after another, so the second one already sees the
moved shared vertex. The result is slightly off the exact solution, which is why the iteration is repeated.

## The solver

`ConstraintSolver::apply` runs at most $50$ passes; one pass projects every constraint once. It stops early when a pass
moves nothing. For consistent constraints the error typically decreases geometrically with the number of passes. When the
constraints contradict each other (or the pinned vertices), no iteration converges, and the state after the last pass
stays: a residual error, no failure. A vertex is always moved with `Vertex::moveWithEdges`, which carries the control points of
the neighbouring Béziers along, so the constraints do not distort the curves attached to the moved vertices.

## Together with continuity: the fixed-point loop

A constraint can turn a segment, which breaks the continuity at its ends, and the continuity can turn a segment, which
breaks a constraint. `PolygonEditor::notify` therefore repeats both rules until nothing moves:

```text
repeat at most 20 rounds:
    moved := {}
    moved += ContinuityEnforcer.apply(event)
    moved += ConstraintSolver.apply(event)
    if moved is empty: stop
    event.seeds := moved and their neighbours
```

The **pinned** vertices and the **driver** edge stay in the event during all the rounds: whatever the two rules do, the
element held by the user keeps following the cursor.

For ordinary edits the loop settles after one or two rounds. A contradictory case is a C1 vertex $V$ between a Bézier
(control point $P_2$ next to $V$) and a segment $VW$ with a fixed length $L$: C1 requires $W - V = 3(V - P_2)$, so
$\Vert VW\Vert = 3\Vert V - P_2\Vert$, while the constraint requires $L$. When the user drags $P_2$ so that
$3\Vert V - P_2\Vert \ne L$, the two rules keep undoing each other until the limit of the rounds is reached; the dragged
control point stays where the user holds it.

## Constraint lifetime

A constraint refers to edges, not to vertices. Edges that disappear (turned into a curve, split, or merged when a
vertex is removed) take their constraints with them, the others keep theirs. This is bookkeeping rather than
mathematics and is described in the in-program documentation.
