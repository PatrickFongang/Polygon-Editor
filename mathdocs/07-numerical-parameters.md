# 7. Numerical parameters

All the thresholds and limits used by the algorithms in one place.

| Parameter | Value | Where | Meaning |
|---|---|---|---|
| hit radius | $8$ px | [input_handler.hpp](../src/ui/input_handler.hpp) | distance within which the cursor hits a vertex, control point or edge |
| Bézier drawing steps $N$ | $50$ | [bezier_edge.cpp](../src/model/edge/bezier_edge.cpp) | segments of the polyline of a curve |
| Bézier / arc distance samples | $100$ | `distanceToPoint` | segments used to measure the distance to a curve |
| arc drawing segments | $\operatorname{clamp}(\lfloor 30\lvert s\rvert/\pi\rfloor,\ 8,\ 90)$ | [arc_edge.cpp](../src/model/edge/arc_edge.cpp) | $s$ is the sweep; 30 segments per half turn |
| arc degeneracy | $\lvert\sin\alpha\rvert < 10^{-4}$ | arc_edge.cpp | below this the arc is a straight segment |
| default Bézier offset | $40$ px | bezier_edge.cpp | normal shift of the initial control points |
| continuity tolerance | $10^{-3}$ | [continuity_enforcer.hpp](../src/editing/continuity_enforcer.hpp) | sine of the angle (G1) or length (C1) |
| continuity work queue | $1000$ steps | continuity_enforcer.hpp | limit of the propagation |
| length constraint tolerance | $10^{-4}$ px | [length_constraint.cpp](../src/model/constraint/length_constraint.cpp) | below this no correction is made |
| parallel constraint tolerance | $10^{-5}$ rad | [parallel_constraint.cpp](../src/model/constraint/parallel_constraint.cpp) | below this no rotation is made |
| solver passes | $50$ | [constraint_solver.hpp](../src/editing/constraint_solver.hpp) | Gauss-Seidel passes per round |
| editing rounds | $20$ | [polygon_editor.hpp](../src/editing/polygon_editor.hpp) | rounds of the fixed-point loop (continuity + constraints) |

Positions are `float`, so lengths of the order of $10^2$ px are represented with a relative error of about
$10^{-7}$; the tolerances above are several orders of magnitude larger than the rounding errors and several orders of
magnitude smaller than a pixel.
