# 1. Conventions

All formulas in this folder use the notation below. It matches the code, so every formula can be checked
against the source file it refers to.

## Vectors

Points and vectors are 2D, with components $(x, y)$. The screen has the $y$ axis pointing **down**, but the
formulas never depend on that: they only use $\sin$ and $\cos$ of angles measured from the $x$ axis towards the
$y$ axis. (On the screen that direction looks clockwise; in the formulas it is called "positive".)

| Symbol | Meaning | Code ([vec_math.hpp](../src/utils/vec_math.hpp)) |
|---|---|---|
| $\mathbf{a}\cdot\mathbf{b} = a_x b_x + a_y b_y$ | dot product | `vec::dot` |
| $\mathbf{a}\times\mathbf{b} = a_x b_y - a_y b_x$ | 2D cross product (a scalar) | `vec::cross` |
| $\Vert\mathbf{a}\Vert$ | length | `vec::length` |
| $\hat{\mathbf{a}} = \mathbf{a} / \Vert\mathbf{a}\Vert$ | unit vector ($\hat{\mathbf{0}} = \mathbf{0}$) | `vec::normalized` |
| $\mathrm{perp}(\mathbf{a}) = (-a_y, a_x)$ | $\mathbf{a}$ turned by $+90^\circ$ | `vec::perp` |

Useful identities, used throughout:

$$
\mathbf{a}\cdot\mathbf{b} = \Vert\mathbf{a}\Vert\Vert\mathbf{b}\Vert\cos\theta, \qquad
\mathbf{a}\times\mathbf{b} = \Vert\mathbf{a}\Vert\Vert\mathbf{b}\Vert\sin\theta, \qquad
\mathbf{a}\cdot\mathrm{perp}(\mathbf{b}) = \mathbf{b}\times\mathbf{a}
$$

where $\theta$ is the angle **from $\mathbf{a}$ to $\mathbf{b}$**, so that

$$
\theta = \mathrm{atan2}(\mathbf{a}\times\mathbf{b},\ \mathbf{a}\cdot\mathbf{b}) \in (-\pi, \pi].
$$

Rotation of a vector by the angle $\varphi$:

$$
R(\varphi)\,\mathbf{r} = \begin{pmatrix} \cos\varphi & -\sin\varphi \\ \sin\varphi & \cos\varphi \end{pmatrix}\mathbf{r}.
$$

## Polygons, edges and traversal order

A polygon has vertices $V_0, \dots, V_{n-1}$. The edge $i$ leads **from $V_i$ to $V_{(i+1) \text{ mod } n}$**. The first
endpoint of an edge is called its *start* (`EdgeEnd::Start`, vertex `v1`) and the second one its *end*
(`EdgeEnd::End`, vertex `v2`).

At a vertex $V$ the edge arriving at $V$ is the **previous** edge (for which $V$ is the end) and the edge leaving
$V$ is the **next** edge (for which $V$ is the start).

## Parametrisation and tangents

Every edge is a curve $E(t)$, $t \in [0, 1]$, running from its start to its end. The **tangent vector** at an end
is the derivative $E'(t)$ there, so it always points **along the traversal of the polygon**:

$$
\mathbf{T}_{\text{start}} = E'(0), \qquad \mathbf{T}_{\text{end}} = E'(1).
$$

Because both tangents at a vertex are oriented the same way, a smooth join has the *incoming* tangent
$E_{\text{prev}}'(1)$ and the *outgoing* tangent $E_{\text{next}}'(0)$ pointing in the **same** direction. This is
the convention behind every continuity formula in [04-continuity.md](04-continuity.md).

| Edge type | $E(t)$ | $E'(0)$ | $E'(1)$ |
|---|---|---|---|
| segment $AB$ | $A + t(B - A)$ | $B - A$ | $B - A$ |
| cubic Bézier $P_0P_1P_2P_3$ | see [02-bezier.md](02-bezier.md) | $3(P_1 - P_0)$ | $3(P_3 - P_2)$ |
| circular arc | see [03-arc.md](03-arc.md) | along the circle, length $=$ arc length | same |

Code: `Edge::tangentAt(EdgeEnd)` in [edge.hpp](../src/model/edge/edge.hpp).
