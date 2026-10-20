# 2D Polygon & Curvilinear Polygon Editor

An interactive 2D geometry editor built in C++ using **SFML 3.0** and **Dear ImGui**. The application allows users to manipulate polygons, convert edges into cubic Bézier curves or circular arcs, and enforce geometric constraints and continuity classes ($G^0$, $G^1$, $C^1$) in real time.

---

## Features

* **Polygon Manipulation:**
  * Support for multiple polygons (including self-intersecting polygons) loaded from a predefined demonstration scene.
  * Interactive dragging of individual vertices, Bézier control points, or entire polygons using the mouse.
  * Dynamic topology editing: adding new vertices at the midpoint of an edge and deleting existing vertices.
* **Geometric Constraints (Relations):**
  * **Fixed Edge Length:** Lock an edge to a user-defined length.
  * **Parallelism / Perpendicularity:** Enforce relational constraints between pairs of edges (within the same polygon or across different polygons).
  * Visual constraint indicators rendered near edge midpoints.
* **Cubic Bézier Curves & Circular Arcs:**
  * Seamless toggling of any edge between a straight line segment, a cubic Bézier curve, and a circular arc.
  * Interactive control polygon rendering (dashed lines) and draggable control points.
  * Enforceable vertex continuity classes: **$G^0$** (positional), **$G^1$** (unit tangent vector direction), and **$C^1$** (tangent vector continuity).
* **Custom Rendering Algorithms:**
  * Switchable line-drawing backend: standard library rendering vs. a custom **Bresenham's line algorithm** implementation.
  * Fast incremental forward-differencing algorithm (power basis) for rendering cubic Bézier segments.

---

## Tech Stack & Requirements

* **Language:** C++17
* **Compiler:** MinGW-w64 (GCC 14.2.0 32-bit DW2)
* **Libraries:**
  * [SFML 3.0.0](https://www.sfml-dev.org/) (Graphics, Window, System)
  * [Dear ImGui](https://github.com/ocornut/imgui) + [ImGui-SFML](https://github.com/SFML/imgui-sfml) (included in the `imgui/` directory)
* **Build System:** Make (`mingw32-make`)

---

## Project Structure

```text
.
├── Makefile          # Build configuration for Windows (MinGW)
├── README.md         # Project documentation
├── src/              # Application source code and geometry engine
│   └── main.cpp
├── imgui/            # Dear ImGui & ImGui-SFML source files
└── SFML/             # Local SFML 3.0 library files (ignored by Git)
    ├── include/
    ├── lib/
    └── bin/

```

---

## How to Build and Run (Windows)

### 1. Prerequisites

Ensure you have **MinGW (GCC 14.2.0 32-bit)** installed and added to your system `PATH` (e.g., `C:\mingw32\bin`).

### 2. Setup SFML

Since binary libraries are excluded from the repository, download **SFML 3.0.0 for GCC 14.2.0 MinGW (DW2) 32-bit**:

1. Extract the `include`, `lib`, and `bin` folders into the `./SFML/` directory in the project root.
2. Copy all `.dll` files from `./SFML/bin/` directly into the project root directory (next to the `Makefile`):
```powershell
Copy-Item SFML/bin/*.dll .

```



### 3. Compile the Project

Open PowerShell or Command Prompt in the project root directory and run:

```powershell
mingw32-make

```

To recompile from scratch (clean build):

```powershell
mingw32-make clean
mingw32-make

```

### 4. Run the Application

Once compiled, launch the executable:

```powershell
.\app.exe

```