# Edytor wielokątów i wielokątów krzywoliniowych

Interaktywny edytor 2D napisany w C++20 z użyciem **SFML 2.6** i **Dear ImGui**. Pozwala edytować wielokąty (także z samoprzecięciami), zamieniać krawędzie na segmenty Béziera 3. stopnia i łuki okręgów, wymuszać ciągłość w wierzchołkach (G0, G1, C1) oraz zadawać ograniczenia geometryczne (długość, równoległość) w czasie rzeczywistym. Pełna dokumentacja jest dostępna w programie (**F1**).

---

## Funkcje

* **Wielokąty:** dowolna liczba wielokątów ze sceny początkowej, samoprzecięcia, przesuwanie wierzchołka, punktu kontrolnego i całego wielokąta, usuwanie wierzchołka, dodawanie wierzchołka w środku krawędzi.
* **Krawędzie:** każdą krawędź można przełączyć między odcinkiem, segmentem Béziera 3. stopnia (z przerywanym wielobokiem kontrolnym i dwoma przesuwalnymi punktami kontrolnymi) a łukiem okręgu.
* **Ciągłość w wierzchołkach:** G0, G1 i C1 dla Béziera, G0 i G1 dla łuku (tylko jeden koniec łuku może być G1; przy G0 na obu końcach łuk jest półokręgiem). Domyślnie G1. Zmiana jednego elementu wymusza odpowiednie zmiany innych.
* **Ograniczenia geometryczne:** zadana długość odcinka i równoległość dwóch odcinków (także z różnych wielokątów). Na jednej krawędzi może być kilka ograniczeń; aktywne są oznaczone etykietami na scenie, można je usuwać.
* **Algorytmy rysowania:** odcinki biblioteczne albo własny algorytm Bresenhama (radio buttony); segmenty Béziera rysowane algorytmem przyrostowym w bazie potęgowej.
* **Scena początkowa:** dwa wielokąty (z krzywą Béziera i łukiem) oraz cztery ograniczenia.

---

## Sterowanie

| Akcja | Efekt |
|---|---|
| LPM na wierzchołku + przeciąganie | przesuwa wierzchołek (podąża za kursorem) |
| Shift + LPM na wierzchołku + przeciąganie | przesuwa cały wielokąt |
| LPM na punkcie kontrolnym Béziera + przeciąganie | przesuwa punkt kontrolny |
| LPM na krawędzi | zaznacza krawędź; przeciąganie przesuwa wielokąt |
| LPM wewnątrz wielokąta + przeciąganie | przesuwa wielokąt |
| Ctrl + LPM na krawędzi | wybiera drugą krawędź (np. do równoległości) |
| PPM na wierzchołku | usuwa wierzchołek (min. 3 w wielokącie) |
| PPM na krawędzi | dodaje wierzchołek w jej środku |
| Delete / Backspace | usuwa zaznaczony wierzchołek |
| F1 | okno dokumentacji |

Panel sterowania: wybór algorytmu rysowania, reset sceny, typ zaznaczonej krawędzi, ograniczenia (długość, równoległość, lista z usuwaniem) i wybór ciągłości zaznaczonego wierzchołka (tylko klasy dozwolone w tym miejscu).

---

## Architektura

```text
.
├── makefile
├── README.md
├── imgui/                    # Dear ImGui + ImGui-SFML
├── tests/                    # testy jednostkowe (bez okna), make test
└── src/
    ├── main.cpp              # składanie: scena, edytor z regułami, UI
    ├── enum/                 # Continuity, EdgeEnd, EdgeType
    ├── model/
    │   ├── vertex.*, polygon.*
    │   ├── edge/             # Edge (interfejs stycznych), LineEdge, BezierEdge, ArcEdge, fabryka
    │   ├── continuity_strategy.hpp   # G0/G1/C1: matematyka na wektorach stycznych
    │   ├── continuity_policy.*       # co jest dozwolone w wierzchołku, wartości domyślne
    │   └── constraint/       # Constraint, LengthConstraint, ParallelConstraint, ConstraintSystem
    ├── editing/              # PolygonEditor, PostEditRule, ContinuityEnforcer, ConstraintSolver
    ├── scene/                # Scene: wielokąty, ograniczenia, scena początkowa
    ├── builder/              # PolygonBuilder, PolygonDirector
    ├── ui/                   # InputHandler, UIManager, okno dokumentacji
    └── utils/                # Drawer (biblioteczny, Bresenham), narzędzia wektorowe
```

Najważniejsze decyzje projektowe:

* **Styczna jako wspólna abstrakcja.** Każda krawędź umie podać (`tangentAt`) i zmienić (`adaptTangent`) swoją styczną w danym końcu. Dzięki temu strategie ciągłości nie znają typów krawędzi, a nowy typ krawędzi nie wymaga zmian w algorytmie ciągłości.
* **Strategie ciągłości** (`ContinuityStrategy`) opisują relację dwóch stycznych: G0 - brak warunku, G1 - ten sam kierunek, C1 - równe wektory.
* **`ContinuityEnforcer`** po edycji sprawdza wierzchołki w kolejce roboczej. Złamaną relację naprawia jedna z krawędzi (follower): przeciągana przez użytkownika jest zawsze driverem, w pozostałych przypadkach ustępuje ta o mniejszym `adaptCost` (łuk, Bezier, odcinek). Bezier przesuwa punkt kontrolny, łuk zapamiętuje kotwicę stycznej, a odcinek obraca się wokół wierzchołka.
* **Ograniczenia** to osobne obiekty (`Constraint`) trzymane w `ConstraintSystem` w scenie. `ConstraintSolver` rzutuje je po kolei (Gauss-Seidel), nie ruszając wierzchołków przypiętych przez użytkownika.
* **`PolygonEditor`** jest jedynym wejściem do edycji i stosuje reguły (`PostEditRule`) w pętli do punktu stałego, bo ciągłość i ograniczenia mogą się wzajemnie psuć. Dokłada też sprzątanie: zmiana typu, podział lub usunięcie wierzchołka usuwa związane z krawędzią ograniczenia i porządkuje ciągłość.
* **Rozszerzalność:** nowy rodzaj ograniczenia, krawędzi, klasy ciągłości lub reguły po edycji to nowa klasa bez zmian w istniejącym kodzie (patrz zakładka „Rozszerzanie" w oknie F1).

## Polimorfizm w projekcie (diagramy UML)

W projekcie są cztery hierarchie polimorficzne. Klient (np. `ContinuityEnforcer`, `Scene`) zawsze zna tylko interfejs, więc nowy typ to nowa klasa, a istniejący kod się nie zmienia (zasada otwarte-zamknięte). Diagramy są w formacie Mermaid i renderują się na GitHubie.

### 1. Krawędzie: jeden interfejs stycznych, trzy geometrie

`ContinuityEnforcer` pyta krawędź tylko o `tangentAt` i każe jej się dopasować przez `adaptTangent`. Nie wie, czy ma przed sobą odcinek, krzywą czy łuk: odcinek obraca się wokół wierzchołka, Bezier przesuwa punkt kontrolny, a łuk zapamiętuje kotwicę.

```mermaid
classDiagram
    direction TB
    class Edge {
        <<abstract>>
        #Vertex v1
        #Vertex v2
        +draw(Drawer) void
        +distanceToPoint(Vector2f) float
        +tangentAt(EdgeEnd) Vector2f
        +adaptTangent(EdgeEnd, Vector2f) vector~Vertex~
        +releaseTangent(EdgeEnd) void
        +supports(Continuity) bool
        +adaptCost() int
        +isCurved() bool
        +derivesTangent() bool
        +type() EdgeType
        +hitTestControlPoint(Vector2f, float) optional~ControlHandle~
    }
    class LineEdge {
        adaptCost = 2
        obraca odcinek, przesuwa drugi koniec
    }
    class BezierEdge {
        -controlPoint1 Vertex
        -controlPoint2 Vertex
        adaptCost = 1
        przesuwa punkt kontrolny
    }
    class ArcEdge {
        -anchor optional~Anchor~
        adaptCost = 0
        zapamietuje kotwice stycznej
        brak C1
    }
    Edge <|-- LineEdge
    Edge <|-- BezierEdge
    Edge <|-- ArcEdge

    class Vertex {
        -position Vector2f
        -continuity Continuity
        +hasExplicitContinuity bool
        +moveWithEdges(Vector2f) void
    }
    class Polygon {
        -vertices vector~Vertex~
        -edges vector~Edge~
        +splitEdge() void
        +removeVertex() bool
    }
    Polygon *-- Vertex : wlasciciel
    Polygon *-- Edge : wlasciciel
    Edge --> Vertex : v1, v2
    Vertex --> Edge : prevEdge, nextEdge
    BezierEdge *-- Vertex : 2 punkty kontrolne
```

### 2. Ciągłość: strategia niezależna od typu krawędzi

Strategia dostaje tylko dwa wektory styczne, więc ta sama klasa `G1Strategy` obsługuje Bezier-odcinek, Bezier-łuk i inne pary.

```mermaid
classDiagram
    direction LR
    class ContinuityStrategy {
        <<abstract>>
        +getType() Continuity
        +desiredFollower(Vector2f driver, Vector2f follower) optional~Vector2f~
        +isSatisfied(Vector2f, Vector2f, float) bool
        +get(Continuity)$ ContinuityStrategy
    }
    class G0Strategy {
        brak warunku
    }
    class G1Strategy {
        ten sam kierunek, dlugosc nastepcy zachowana
    }
    class C1Strategy {
        rowne wektory
    }
    ContinuityStrategy <|-- G0Strategy
    ContinuityStrategy <|-- G1Strategy
    ContinuityStrategy <|-- C1Strategy

    class ContinuityPolicy {
        +DEFAULT Continuity
        +isAllowed(Vertex, Continuity)$ bool
        +allowed(Vertex)$ vector~Continuity~
        +defaultFor(Vertex)$ Continuity
        +effective(Vertex)$ Continuity
        +sanitize(Vertex)$ void
    }
    class ContinuityEnforcer {
        +apply(EditEvent) vector~Vertex~
        -enforceAt(Vertex, Edge driver)$ vector~Vertex~
    }
    class Edge {
        <<abstract>>
    }
    ContinuityEnforcer ..> ContinuityStrategy : relacja stycznych
    ContinuityEnforcer ..> ContinuityPolicy : klasa w wierzcholku
    ContinuityEnforcer ..> Edge : tangentAt, adaptTangent
    ContinuityPolicy ..> Edge : supports, isCurved, derivesTangent
```

### 3. Edycja: reguły po edycji i ograniczenia

`PolygonEditor` nie wie, jakie reguły ma uruchomić: trzyma listę `PostEditRule` i powtarza ją do punktu stałego. `ConstraintSolver` z kolei nie wie, jakie ograniczenia rzutuje: woła `project` na interfejsie `Constraint`.

```mermaid
classDiagram
    direction TB
    class PolygonEditor {
        -rules vector~PostEditRule~
        +addRule(PostEditRule) void
        +moveVertex() void
        +moveControlPoint() void
        +toggleEdgeType() void
        +setEdgeLength() LengthConstraint
        +addParallelConstraint() ParallelConstraint
        -notify(EditEvent) void
    }
    class PostEditRule {
        <<abstract>>
        +apply(EditEvent) vector~Vertex~
    }
    class ContinuityEnforcer
    class ConstraintSolver {
        -MAX_PASSES = 50
    }
    PolygonEditor o-- PostEditRule : lista regul
    PostEditRule <|-- ContinuityEnforcer
    PostEditRule <|-- ConstraintSolver

    class ConstraintSystem {
        +add(Constraint) Constraint
        +remove(Constraint) bool
        +constraintsOf(Edge) vector~Constraint~
        +removeInvolving(Edge) void
    }
    class Constraint {
        <<abstract>>
        +edges() vector~Edge~
        +isSatisfied(float) bool
        +project(PinnedSet) vector~Vertex~
        +label() string
    }
    class LengthConstraint {
        -length float
        dzieli poprawke miedzy konce
    }
    class ParallelConstraint {
        obraca krawedzie o czesci kata
    }
    class Edge {
        <<abstract>>
    }
    Constraint <|-- LengthConstraint
    Constraint <|-- ParallelConstraint
    ConstraintSystem *-- Constraint : wlasciciel
    Constraint --> Edge : wskazuje
    ConstraintSolver --> ConstraintSystem : rzutuje wszystkie
    PolygonEditor --> ConstraintSystem : sprzata przy zmianie topologii
```

### 4. Rysowanie: wymienialny algorytm

Zmiana algorytmu (radio button) to podmiana obiektu `Drawer` w scenie. Odcinki, krzywe i łuki rysują się przez ten sam interfejs.

```mermaid
classDiagram
    direction LR
    class Drawer {
        <<abstract>>
        +drawLine(float, float, float, float) void
        +setColor(Color) void
    }
    class LibraryDrawer {
        uzywa sf Lines
    }
    class BresenhamDrawer {
        wlasny algorytm Bresenhama
    }
    Drawer <|-- LibraryDrawer
    Drawer <|-- BresenhamDrawer
    class Scene {
        -polygons vector~Polygon~
        -constraints ConstraintSystem
        -currentDrawer Drawer
        +setDrawer(Drawer) void
        +render() void
    }
    class Edge {
        <<abstract>>
        +draw(Drawer) void
    }
    Scene o-- Drawer : aktualny algorytm
    Edge ..> Drawer : draw(Drawer)
```

### Gdzie dokładnie działa polimorfizm

| Wywołanie przez interfejs | Kto woła | Co się dzieje zależnie od typu |
|---|---|---|
| `edge->tangentAt(end)` | `ContinuityEnforcer`, `ContinuityPolicy` | odcinek: `v2 - v1`; Bezier: `3 (P1 - P0)`; łuk: pochodna po okręgu |
| `edge->adaptTangent(end, d)` | `ContinuityEnforcer` | odcinek obraca się, Bezier przesuwa punkt, łuk zapamiętuje kotwicę |
| `edge->adaptCost()` | `ContinuityEnforcer` | wybór, która krawędź ustępuje (łuk, Bezier, odcinek) |
| `strategy.desiredFollower(a, b)` | `ContinuityEnforcer` | G0 nic, G1 kierunek, C1 pełny wektor |
| `constraint->project(pinned)` | `ConstraintSolver` | długość skaluje odcinek, równoległość obraca krawędzie |
| `rule->apply(event)` | `PolygonEditor` | najpierw ciągłość, potem ograniczenia, w pętli do punktu stałego |
| `edge->draw(drawer)`, `drawer.drawLine(...)` | `Scene` | dwa niezależne wymiary zmienności: kształt krawędzi i algorytm rysowania |

### Scena początkowa

Scenę buduje `Scene::initPredefinedScene()` ([src/scene/scene.cpp](src/scene/scene.cpp)). Dodanie wielokąta lub ograniczenia to jedna linia:

```cpp
Polygon& triangle = addPolygon(PolygonDirector::createDemoPolygon1());
constraints.add(std::make_unique<LengthConstraint>(triangle.getEdge(1), 150.0f));
constraints.add(std::make_unique<ParallelConstraint>(triangle.getEdge(1), diamond.getEdge(3)));
```

---

## Przyjęte założenia

* Ograniczenia dotyczą tylko odcinków; zmiana krawędzi na krzywą, jej podział lub usunięcie któregoś z jej wierzchołków usuwa jej ograniczenia.
* Zaimplementowane są: zadana długość i równoległość (nazwiska A-K). Prostopadłość (L-Z) nie jest zaimplementowana.
* Domyślna ciągłość to G1 (`ContinuityPolicy::DEFAULT`); wybór użytkownika nie jest nadpisywany wartością domyślną.
* Ciągłość ma sens tylko przy wierzchołku sąsiadującym z krzywą.
* Przesuwany wierzchołek zawsze podąża za kursorem; przy sprzecznych ograniczeniach zostaje błąd resztkowy (limit iteracji).
* Podział krawędzi daje dwa odcinki.

---

## Budowanie i uruchamianie (Linux)

Wymagane: `g++` (C++20), SFML 2.6 (moduły graphics, window, system) i OpenGL. Na Ubuntu:

```bash
sudo apt install build-essential libsfml-dev libgl-dev
```

Kompilacja i uruchomienie (makefile używa AddressSanitizera i śledzi zależności od nagłówków):

```bash
make
./app
```

Testy jednostkowe (bez okna):

```bash
make test
```

Czyszczenie: `make clean`.
