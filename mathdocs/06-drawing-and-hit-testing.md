# 6. Line drawing and hit testing

Code: [bresenham_drawer.cpp](../src/utils/bresenham_drawer.cpp), [edge.cpp](../src/model/edge/edge.cpp),
[input_handler.cpp](../src/ui/input_handler.cpp).

## Bresenham's line algorithm

The line from $(x_0, y_0)$ to $(x_1, y_1)$ is drawn on the integer pixel grid. Let

$$
d_x = |x_1 - x_0|, \qquad d_y = |y_1 - y_0|, \qquad s_x = \operatorname{sgn}(x_1 - x_0), \qquad s_y = \operatorname{sgn}(y_1 - y_0) .
$$

The program uses the variant that works in **all octants** with one loop:

```text
err = dx - dy
loop:
    plot(x, y);  if (x, y) = (x1, y1) then stop
    e2 = 2 * err
    if e2 > -dy:  err -= dy;  x += sx        # step in x
    if e2 <  dx:  err += dx;  y += sy        # step in y   (both: a diagonal step)
```

Only integer additions and comparisons are needed.

**Why it works.** Let $u$ and $v$ be the numbers of steps already made in $x$ and in $y$ (so the pixel is
$(x_0 + s_x u,\ y_0 + s_y v)$). The ideal line satisfies $v\,d_x = u\,d_y$. The loop maintains

$$
\mathrm{err} = (d_x - d_y) + D(u, v), \qquad D(u, v) = v\,d_x - u\,d_y ,
$$

because a step in $x$ changes $\mathrm{err}$ by $-d_y$ (i.e. $u \to u + 1$) and a step in $y$ by $+d_x$ ($v \to v + 1$).
$D$ is $d_x$ times the vertical distance of the pixel row from the ideal line at column $u$. With $e_2 = 2\,\mathrm{err}$ the two tests are
equivalent to

$$
\text{step in } y \iff 2D < 2d_y - d_x \iff y_{\mathrm{ideal}}(u + 1) > v + \tfrac12,
$$

$$
\text{step in } x \iff 2D > d_y - 2d_x \iff x_{\mathrm{ideal}}(v + 1) > u + \tfrac12,
$$

where $y_{\mathrm{ideal}}(u) = u\,d_y/d_x$ and $x_{\mathrm{ideal}}(v) = v\,d_x/d_y$ (in step units). In words:

* step in $y$ if the ideal line is more than half a pixel above the current row at the **next column**,
* step in $x$ if the ideal line is more than half a pixel beyond the current column at the **next row**,
* both conditions together give a diagonal step.

For a shallow line ($d_x \ge d_y$) the test for $x$ always succeeds when the pixels follow the line, so $x$ advances every
iteration and $y$ only when the line has drifted half a pixel. The roles are exchanged for a steep line. The loop
makes $\max(d_x, d_y)$ iterations, one per pixel column (or row) of the longer axis.

The strict inequalities decide the ties (a line passing exactly between two pixels); the textbook version with
$\ge$ and $\le$ may draw the other of the two pixels there.

Every polyline segment of a Bézier curve or an arc is drawn with the same algorithm, because curves are approximated by
polylines (see [02-bezier.md](02-bezier.md), [03-arc.md](03-arc.md)).

## Distance from a point to a segment

For the segment $AB$ and the point $p$, let $\mathbf{ab} = B - A$ and $\mathbf{ap} = p - A$. The projection parameter,
clamped to the segment, is

$$
t = \operatorname{clamp}\!\left(\frac{\mathbf{ap}\cdot\mathbf{ab}}{\lVert\mathbf{ab}\rVert^2},\ 0,\ 1\right), \qquad
\operatorname{dist}(p, AB) = \lVert p - (A + t\,\mathbf{ab})\rVert .
$$

If $A = B$ the distance is $\lVert p - A\rVert$. (`Edge::distanceToPoint`.)

## Distance to a curve or an arc

For Bézier curves and arcs the distance is the minimum of the segment distances over a polyline with $N = 100$ points
$X(k/N)$. This is simple and robust; the error is at most the sagitta of one polyline segment, far below the hit radius
for the sizes used here.

## Choosing the object under the cursor

The cursor hits an element within a radius of $8$ px. The elements are tested in this priority order and the first
group with a hit wins: **vertex** (the nearest one), **Bézier control point**, **edge** (the nearest one), **interior of
a polygon**.

## Point in polygon (interior)

The test casts a horizontal ray from the point $p$ to the right and counts the crossings with the edges (the
**even-odd rule**). For the vertices $V_i$ and $V_j$ ($j = i - 1$, cyclically) the edge $V_jV_i$ is crossed when

$$
(V_{i,y} > p_y) \neq (V_{j,y} > p_y) \quad\text{and}\quad
p_x < V_{i,x} + \frac{(V_{j,x} - V_{i,x})(p_y - V_{i,y})}{V_{j,y} - V_{i,y}} .
$$

The first condition says that the edge straddles the horizontal line through $p$; the second that the crossing
point lies to the right of $p$ (the right-hand side is the $x$ coordinate of the intersection of the edge with
$y = p_y$). An odd number of crossings means the point is inside; with self-intersections the even-odd rule makes
the doubly covered regions "outside". The test uses only the **vertices**, so for polygons with curved edges the
interior is that of the polygon of the vertices, an approximation.
