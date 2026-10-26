# 3. Circular arcs

Code: [arc_edge.cpp](../src/model/edge/arc_edge.cpp).

An arc edge joins $P$ (start) and $Q$ (end). Write $\mathbf{c} = Q - P$ for the chord and $\ell = \Vert\mathbf{c}\Vert$.

A circular arc through two points is not unique, so the program fixes it with the continuity requirements:

* **no tangent required** (G0 at both ends): the arc is a **semicircle** over the chord (radius $\ell/2$),
* **tangent required at one end** (G1 there): the arc is the only circular arc through $P$ and $Q$ that has
  the given tangent at that end.

(Only one end of an arc can be G1: with tangents prescribed at both ends the arc would be over-determined.)

Every point of the arc is described with the centre $C$, the radius $r > 0$, the start angle $\theta_0$ and the
signed **sweep** $s$ (central angle; $s > 0$ means the angle grows along the arc):

$$
X(t) = C + r\,(\cos(\theta_0 + s\,t),\ \sin(\theta_0 + s\,t)), \qquad t \in [0, 1].
$$

## Arc with a tangent at the start

Given the tangent direction $\hat{\mathbf{t}}$ at $P$ (the unit vector pointing along the traversal).

**Centre.** The centre lies on the normal of the tangent at $P$, so $C = P + R\,\mathbf{n}$ with
$\mathbf{n} = \mathrm{perp}(\hat{\mathbf{t}})$ and a signed distance $R$. The condition $\Vert C - Q\Vert = |R|$ gives

$$
\Vert R\mathbf{n} - \mathbf{c}\Vert^2 = R^2
\;\Rightarrow\;
\ell^2 - 2R\,\mathbf{n}\cdot\mathbf{c} = 0
\;\Rightarrow\;
R = \frac{\ell^2}{2\,\mathbf{n}\cdot\mathbf{c}} = \frac{\ell^2}{2\,(\hat{\mathbf{t}}\times\mathbf{c})} .
$$

With $\sin\alpha = \frac{\hat{\mathbf{t}}\times\mathbf{c}}{\ell}$ this is

$$
R = \frac{\ell}{2\sin\alpha}, \qquad r = |R|, \qquad C = P + R\,\mathrm{perp}(\hat{\mathbf{t}}).
$$

**Sweep.** Let $\alpha$ be the angle **from the tangent to the chord**,
$\alpha = \mathrm{atan2}(\hat{\mathbf{t}}\times\mathbf{c},\ \hat{\mathbf{t}}\cdot\mathbf{c}) \in (-\pi, \pi]$.
By the tangent-chord theorem the angle between the tangent and the chord equals **half of the central angle**
subtending the chord, therefore

$$
s = 2\alpha.
$$

For $|\alpha| > \pi/2$ the tangent points away from $Q$ and the arc is longer than a semicircle; the formula covers
this case without any special treatment. The sign of $\alpha$ tells on which side of the tangent the arc curves,
and it is the sign of $R$ and of $s$ as well.

**Start angle.** $\theta_0 = \mathrm{atan2}(P_y - C_y,\ P_x - C_x)$.

**Check.** The derivative is

$$
X'(t) = r\,s\,(-\sin(\theta_0 + st),\ \cos(\theta_0 + st)),
$$

so $X'(0) = r\,s\,\mathrm{perp}\,(\hat{P - C})$, which is perpendicular to the radius $P - C$
(a tangent) and has the direction of $\hat{\mathbf{t}}$ (the signs of $s$ and of $R$ cancel). Its length
$r\,|s|$ is the **length of the arc**.

### Worked example

$P = (0, 0)$, $Q = (100, 0)$, tangent $\hat{\mathbf{t}} = (1, 1)/\sqrt2$. Then $\mathbf{c} = (100, 0)$, $\ell = 100$,

$$
\hat{\mathbf{t}}\times\mathbf{c} = \frac{1}{\sqrt2}(1\cdot 0 - 1\cdot 100) = -\frac{100}{\sqrt2}, \qquad
\sin\alpha = -\frac{1}{\sqrt2}, \qquad \alpha = -45^\circ,
$$

$$
R = \frac{100}{2\cdot(-1/\sqrt2)} = -50\sqrt2 \approx -70.7, \qquad s = -90^\circ,
$$

$$
C = P + R\,\mathrm{perp}(\hat{\mathbf{t}}) = (0,0) + (-70.7)\cdot\frac{1}{\sqrt2}(-1, 1) = (50, -50).
$$

Indeed $\Vert C - P\Vert = \Vert C - Q\Vert = 50\sqrt2$ and $(P - C)\cdot\hat{\mathbf{t}} = 0$.

## Arc with a tangent at the end

If the tangent $\mathbf{T}$ is prescribed at the **end** $Q$, build the arc **travelled backwards**: from $Q$ to $P$,
leaving $Q$ along $-\hat{\mathbf{T}}$ (reversing the direction of travel reverses the tangent). This is the case above
with the roles of $P$ and $Q$ swapped and gives $C$, $r$, $\theta_0'$ and $s'$. The same circle travelled forwards
has

$$
\theta_0 = \theta_0' + s', \qquad s = -s' .
$$

(`arcFromStartTangent` is used for both ends; `ArcEdge::geometry` reverses the result in the second case.)

## Arc without a tangent (semicircle)

$$
C = \frac{P + Q}{2}, \qquad r = \frac{\ell}{2}, \qquad s = +\pi, \qquad \theta_0 = \mathrm{atan2}(P - C).
$$

This is the general case with $\alpha = +90^\circ$ (the tangent is perpendicular to the chord).

## Degenerate arcs

If $\sin\alpha \to 0$ the tangent is parallel to the chord and $R \to \infty$: the "arc" is a straight segment.
The program treats $|\sin\alpha| < 10^{-4}$ (and $\ell = 0$) as a **segment**: it draws $P + t\,\mathbf{c}$ and reports
the tangent $\mathbf{c}$. For $\alpha$ close to $\pm\pi$ (the tangent pointing exactly away from $Q$) the arc would
be a full circle and the same fallback applies.

## Tangent at the ends

$$
X'(0) = r\,s\,(-\sin\theta_0,\ \cos\theta_0), \qquad
X'(1) = r\,s\,(-\sin(\theta_0 + s),\ \cos(\theta_0 + s)).
$$

These are the values of `ArcEdge::tangentAt`; for an anchored arc the tangent at the anchored end has exactly the
anchor direction.

## Drawing

The arc is drawn as a polyline of $\mathrm{clamp}(\lfloor 30\,|s|/\pi \rfloor, 8, 90)$ segments between the points
$X(k/m)$.
