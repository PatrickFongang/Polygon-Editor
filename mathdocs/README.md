# Mathematics behind the editor

Short derivations of the formulas used in the program. Every document points to the source files that implement
the described algorithm, and the symbols match the code, so the formulas can be compared with it directly.

| File | Contents |
|---|---|
| [01-conventions.md](01-conventions.md) | vectors, dot and cross product, perpendicular vector, rotations, edge orientation and tangents |
| [02-bezier.md](02-bezier.md) | cubic Bézier curves: Bernstein and power basis, end tangents, incremental drawing with forward differences |
| [03-arc.md](03-arc.md) | circular arcs: the arc through two points with a given tangent, the sweep angle, the semicircle, degenerate cases |
| [04-continuity.md](04-continuity.md) | G0 / G1 / C1, how the follower edge is repaired, propagation, rules of the continuity policy |
| [05-constraints.md](05-constraints.md) | fixed length and parallelism as projections, Gauss-Seidel, pinned vertices, cooperation with continuity |
| [06-drawing-and-hit-testing.md](06-drawing-and-hit-testing.md) | Bresenham's algorithm, distance to a segment and a curve, point in polygon |
| [07-numerical-parameters.md](07-numerical-parameters.md) | all the tolerances and limits |

The formulas use LaTeX (`$...$`), which GitHub renders.
