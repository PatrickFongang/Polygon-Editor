# 2. Cubic Bézier segments

Code: [bezier_edge.cpp](../src/model/edge/bezier_edge.cpp).

## Definition

A cubic Bézier segment is given by four points: the endpoints $P_0, P_3$ (vertices of the polygon) and two
control points $P_1, P_2$. In the Bernstein basis

$$
B(t) = (1-t)^3 P_0 + 3(1-t)^2 t\,P_1 + 3(1-t)t^2 P_2 + t^3 P_3, \qquad t \in [0, 1].
$$

The polyline $P_0P_1P_2P_3$ is the *control polygon* (drawn dashed in the program).

## Power basis

Expanding the Bernstein form and collecting the powers of $t$ gives

$$
B(t) = \mathbf{a}\,t^3 + \mathbf{b}\,t^2 + \mathbf{c}\,t + \mathbf{d},
$$

$$
\mathbf{a} = -P_0 + 3P_1 - 3P_2 + P_3, \qquad
\mathbf{b} = 3P_0 - 6P_1 + 3P_2, \qquad
\mathbf{c} = -3P_0 + 3P_1, \qquad
\mathbf{d} = P_0 .
$$

These are exactly the four vectors computed in `BezierEdge::draw`.

## Derivative and end tangents

$$
B'(t) = 3\mathbf{a}\,t^2 + 2\mathbf{b}\,t + \mathbf{c}.
$$

At the ends

$$
B'(0) = \mathbf{c} = 3(P_1 - P_0), \qquad
B'(1) = 3\mathbf{a} + 2\mathbf{b} + \mathbf{c} = 3(P_3 - P_2).
$$

(For $B'(1)$: the coefficient of $P_0$ is $-3 + 6 - 3 = 0$, of $P_1$ is $9 - 12 + 3 = 0$, of $P_2$ is $-9 + 6 = -3$,
of $P_3$ is $3$.) These are the tangents used by the continuity algorithm
(`BezierEdge::tangentAt`), and it follows that the control point next to an end completely determines the
tangent at that end:

$$
P_1 = P_0 + \frac{1}{3}B'(0), \qquad P_2 = P_3 - \frac{1}{3}B'(1)
$$

(`BezierEdge::adaptTangent`).

## Incremental drawing with forward differences

Evaluating $B(t)$ at $N+1$ equally spaced parameters $t_k = kh$, $h = 1/N$, with the power form would need
multiplications and powers for every point. The **forward differences** reduce each step to three vector
additions.

Define the differences of the sequence $B_k = B(kh)$:

$$
\Delta_1(t) = B(t+h) - B(t), \qquad
\Delta_2(t) = \Delta_1(t+h) - \Delta_1(t), \qquad
\Delta_3(t) = \Delta_2(t+h) - \Delta_2(t).
$$

For a cubic polynomial:

$$
\Delta_1(t) = \mathbf{a}(3t^2h + 3th^2 + h^3) + \mathbf{b}(2th + h^2) + \mathbf{c}h,
$$

$$
\Delta_2(t) = \mathbf{a}(6th^2 + 6h^3) + 2\mathbf{b}h^2,
$$

$$
\Delta_3(t) = 6\mathbf{a}h^3 \quad\text{(constant)}.
$$

At $t = 0$ the initial values are

$$
B_0 = \mathbf{d}, \qquad
\Delta_1(0) = \mathbf{a}h^3 + \mathbf{b}h^2 + \mathbf{c}h, \qquad
\Delta_2(0) = 6\mathbf{a}h^3 + 2\mathbf{b}h^2, \qquad
\Delta_3 = 6\mathbf{a}h^3 .
$$

Each step then is

$$
B_{k+1} = B_k + \Delta_1,\qquad \Delta_1 \leftarrow \Delta_1 + \Delta_2,\qquad \Delta_2 \leftarrow \Delta_2 + \Delta_3 ,
$$

which is the loop in `BezierEdge::draw` (`currentPos += delta1; delta1 += delta2; delta2 += delta3;`).
The program uses $N = 50$ steps and draws a segment of the polyline between consecutive points with the selected
line algorithm ([06-drawing-and-hit-testing.md](06-drawing-and-hit-testing.md)).

Remarks:

* The method accumulates rounding errors over the $N$ steps, which is negligible for $N = 50$ in `float`.
* The last point should be $P_3$: after $N$ steps $B_N = B(1) = P_3$ up to rounding.

## Default control points

A segment turned into a Bézier curve gets control points at one third and two thirds of the chord, shifted by
$40$ px along the normal of the chord:

$$
P_1 = P_0 + \frac13(P_3 - P_0) + 40\,\hat{\mathbf{n}}, \qquad
P_2 = P_0 + \frac23(P_3 - P_0) + 40\,\hat{\mathbf{n}}, \qquad
\hat{\mathbf{n}} = \hat{\mathrm{perp}(P_3 - P_0)} .
$$

(The continuity algorithm then immediately corrects them if the end vertices require G1 or C1.)
