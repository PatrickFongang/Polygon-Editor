#include "documentation.hpp"
#include "imgui.h"

namespace
{
    void heading(const char *text)
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.4f, 1.0f), "%s", text);
        ImGui::Separator();
    }

    void paragraph(const char *text)
    {
        ImGui::TextWrapped("%s", text);
    }

    void bullet(const char *text)
    {
        ImGui::Bullet();
        ImGui::TextWrapped("%s", text);
    }

    void usageTab()
    {
        heading("Mysz");
        bullet("LPM na wierzcholku + przeciaganie: przesuwa wierzcholek. Wierzcholek zawsze podaza za kursorem, punkty kontrolne sasiednich krzywych Beziera jada razem z nim.");
        bullet("Shift + LPM na wierzcholku + przeciaganie: przesuwa caly wielokat.");
        bullet("LPM na punkcie kontrolnym Beziera (pomaranczowe kolo) + przeciaganie: przesuwa punkt. Nie rusza wierzcholkow krawedzi, chyba ze wymaga tego ciaglosc lub ograniczenie (np. sasiedni odcinek obraca sie wokol wierzcholka).");
        bullet("LPM na krawedzi: zaznacza krawedz (zolta); przeciaganie przesuwa caly wielokat.");
        bullet("LPM wewnatrz wielokata: zaznacza wielokat; przeciaganie przesuwa go.");
        bullet("Ctrl + LPM na krawedzi: wybiera druga krawedz (fioletowa), takze z innego wielokata. Ctrl + LPM w puste miejsce odznacza druga krawedz.");
        bullet("LPM w puste miejsce: usuwa zaznaczenie.");
        bullet("PPM na wierzcholku: usuwa wierzcholek (wielokat ma co najmniej 3).");
        bullet("PPM na krawedzi: dodaje wierzcholek w jej srodku (krawedz zamienia sie na dwa odcinki).");

        heading("Klawisze");
        bullet("F1: okno dokumentacji.");
        bullet("Delete / Backspace: usuwa zaznaczony wierzcholek.");

        heading("Panel sterowania");
        bullet("Algorytm rysowania odcinkow: biblioteczny (SFML) lub wlasny Bresenham (radio buttony). Dotyczy tez odcinkow, z ktorych rysowane sa krzywe i luki.");
        bullet("Resetuj do sceny poczatkowej: wraca do sceny demonstracyjnej wraz z jej ograniczeniami.");
        bullet("Zaznaczona krawedz: Dodaj wierzcholek na srodku; typ krawedzi (Odcinek, Bezier 3. st., Luk okregu).");
        bullet("Ograniczenia: pole Dlugosc + Ustaw dlugosc; Rownolegle do drugiej krawedzi (po zaznaczeniu drugiej krawedzi przez Ctrl + LPM); lista aktywnych ograniczen obu zaznaczonych krawedzi z przyciskiem Usun.");
        bullet("Zaznaczony wierzcholek: wybor ciaglosci G0 / G1 / C1 (tylko dozwolone) i Usun wierzcholek.");

        heading("Oznaczenia na scenie");
        bullet("Zolta etykieta G0 / G1 / C1 przy wierzcholku: aktualnie wymuszana ciaglosc (pokazywana tylko przy wierzcholkach sasiadujacych z krzywa).");
        bullet("Niebieska etykieta przy srodku krawedzi: ograniczenie. L=150 to zadana dlugosc, ||1 to rownoleglosc; ten sam numer na dwoch krawedziach oznacza ich pare.");
        bullet("Przerywane linie: wielobok kontrolny Beziera. Pomaranczowe kola: punkty kontrolne. Zielony wierzcholek: zaznaczony.");
    }

    void assumptionsTab()
    {
        heading("Wielokaty");
        bullet("Wielokaty umieszcza scena poczatkowa (kod); uzytkownik moze je edytowac, nie tworzy ich. Dozwolone sa samoprzeciecia.");
        bullet("Krawedz i laczy wierzcholek i z wierzcholkiem i+1 (ostatnia z pierwszym). Wielokat ma zawsze co najmniej 3 wierzcholki.");
        bullet("Podzial krawedzi (PPM) zawsze daje dwa odcinki; krzywizna dzielonej krawedzi jest tracona.");

        heading("Ograniczenia");
        bullet("Dostepne: zadana dlugosc i rownoleglosc (nazwiska A-K). Prostopadlosc nie jest zaimplementowana; dodanie jej wymaga tylko nowej klasy Constraint.");
        bullet("Ograniczenia dotycza wylacznie odcinkow. Zmiana krawedzi na krzywa, jej podzial lub usuniecie ktoregokolwiek z jej wierzcholkow usuwa jej ograniczenia (ograniczenia pozostalych krawedzi zostaja).");
        bullet("Na jednej krawedzi moze byc kilka ograniczen roznych rodzajow. Ponowne zadanie dlugosci zmienia wartosc, a nie dodaje drugiego ograniczenia. Ta sama para krawedzi nie moze byc rownolegla dwa razy.");
        bullet("Przesuwany wierzcholek zawsze podaza za kursorem, reszta ukladu dopasowuje sie do niego. Gdy ograniczenia sa sprzeczne (lub przeciazone), po limicie iteracji zostaje blad resztkowy - program nie zglasza bledu.");

        heading("Ciaglosc");
        bullet("Ciaglosc ma sens tylko w wierzcholku sasiadujacym z krzywa. Miedzy dwoma odcinkami wierzcholek jest G0, a zapamietany wybor jest zachowany na wypadek pozniejszej zmiany krawedzi.");
        bullet("Domyslna klasa po przelaczeniu krawedzi na krzywa to G1 (nazwiska A-K); zmienia sie ja jedna stala ContinuityPolicy::DEFAULT. Wybor uzytkownika (flaga hasExplicitContinuity) nigdy nie jest nadpisywany domyslna, najwyzej obnizany, gdy przestaje byc mozliwy.");
        bullet("Luk: tylko G0 i G1; tylko jeden koniec luku moze miec G1; przy G0 na obu koncach luk jest polokregiem o promieniu rownym polowie dlugosci krawedzi. Wierzcholek miedzy dwoma lukami nie moze byc G1.");
        bullet("Gdy luk traci jedyny koniec G1 (zmiana na G0), wraca do polokregu - tego wymaga specyfikacja.");

        heading("Inne");
        bullet("Domyslne punkty kontrolne nowego Beziera leza w 1/3 i 2/3 cieciwy, przesuniete o 40 px; od razu dopasowuja sie do ciaglosci sasiadow.");
        bullet("Trafienie w element: promien 8 px; kolejnosc: wierzcholek, punkt kontrolny, krawedz, wnetrze wielokata (regula parzystosci).");
    }

    void modelTab()
    {
        heading("Model geometryczny (src/model)");
        bullet("Vertex: pozycja, kolor, klasa ciaglosci (enum Continuity), flaga hasExplicitContinuity oraz wskazniki na krawedz poprzednia i nastepna.");
        bullet("Edge (klasa abstrakcyjna): dwa wierzcholki, draw, distanceToPoint, getLength, getMidpoint oraz interfejs stycznych: tangentAt, adaptTangent, releaseTangent, supports, adaptCost, isCurved, type.");
        bullet("LineEdge, BezierEdge (dwa punkty kontrolne), ArcEdge (opcjonalna kotwica stycznej na jednym koncu). Nowy rodzaj krawedzi to nowa klasa + wpis w EdgeFactory.");
        bullet("Polygon: wlasciciel wierzcholkow i krawedzi (unique_ptr), dzielenie krawedzi, usuwanie wierzcholka, translacja.");
        bullet("ContinuityStrategy (G0, G1, C1): czysta matematyka na wektorach stycznych, nie zna typow krawedzi. ContinuityPolicy: co jest dozwolone w wierzcholku i jakie sa wartosci domyslne.");
        bullet("Constraint (LengthConstraint, ParallelConstraint) wskazuje krawedzie i nimi nie wlada. ConstraintSystem (w Scene) przechowuje ograniczenia, takze miedzy wielokatami.");

        heading("Edycja (src/editing)");
        bullet("PolygonEditor: jedyne wejscie do edycji (przesun wierzcholek, przesun punkt kontrolny, zmien typ krawedzi, podziel, usun, ustaw ciaglosc, dodaj/usun ograniczenie). Po zmianie modelu uruchamia reguly.");
        bullet("PostEditRule: interfejs reguly stosowanej po edycji; zwraca wierzcholki, ktore przesunela. Implementacje: ContinuityEnforcer i ConstraintSolver.");
        bullet("EditEvent: wierzcholki do sprawdzenia (seeds), wierzcholki przypiete przez uzytkownika (pinned) i krawedz przeciagana przez uzytkownika (driver).");

        heading("Pozostale");
        bullet("Scene: wielokaty, ConstraintSystem, aktualny Drawer, zaznaczenie. initPredefinedScene() buduje scene demonstracyjna.");
        bullet("PolygonBuilder / PolygonDirector: budowa wielokatow sceny (wierzcholki, krawedzie, krzywe).");
        bullet("Drawer (LibraryDrawer, BresenhamDrawer): rysowanie odcinkow; krzywe i luki sa rysowane jako lamane przez ten sam interfejs.");
        bullet("InputHandler (mysz, klawisze) i UIManager (panel, etykiety, dokumentacja) znaja tylko Scene i PolygonEditor.");
    }

    void algorithmsTab()
    {
        heading("Rysowanie odcinkow");
        paragraph("Do wyboru: funkcja biblioteczna SFML albo wlasny algorytm Bresenhama (liczby calkowite, blad err = dx - dy, kroki sx i sy po obu osiach, dzieki czemu dziala w kazdym oktancie).");

        heading("Segment Beziera 3. stopnia (algorytm przyrostowy)");
        paragraph("Punkty P0..P3 przechodza do bazy potegowej: B(t) = a t^3 + b t^2 + c t + d, gdzie a = -P0 + 3 P1 - 3 P2 + P3, b = 3 P0 - 6 P1 + 3 P2, c = -3 P0 + 3 P1, d = P0. Krzywa jest rysowana w N = 50 krokach dt = 1/N metoda rownic roznicowych (forward differences): "
                  "pozycja += d1; d1 += d2; d2 += d3, gdzie poczatkowo d1 = a dt^3 + b dt^2 + c dt, d2 = 6 a dt^3 + 2 b dt^2, d3 = 6 a dt^3. Kazdy punkt kosztuje trzy dodawania wektorow, bez potegowania.");

        heading("Luk okregu");
        paragraph("Bez zadanej stycznej (G0 na obu koncach) luk jest polokregiem nad cieciwa. Przy G1 na jednym koncu zadana jest styczna t w tym koncu: luk jest jedynym lukiem przechodzacym przez oba konce i stycznym do t. "
                  "Niech alpha bedzie katem skierowanym od t do cieciwy c. Kat srodkowy to 2 alpha, promien R = |c| / (2 sin alpha), a srodek lezy w P + perp(t) R. Gdy styczna jest rownolegla do cieciwy, luk degeneruje sie do odcinka. "
                  "Kotwica na drugim koncu jest liczona na odwroconym luku (od Q do P ze styczna -t) i obracana z powrotem. Styczna luku w koncu to pochodna wzgledem t, zorientowana zgodnie z obiegiem wielokata.");

        heading("Ciaglosc w wierzcholku");
        paragraph("Kazda krawedz podaje swoja styczna w koncu (pochodna dla t w [0, 1], zgodnie z obiegiem): odcinek v2 - v1, Bezier 3 (P1 - P0) i 3 (P3 - P2), luk - pochodna lukowa. Wierzcholek ma klase; strategia opisuje relacje stycznych: G0 - brak warunku, "
                  "G1 - ten sam kierunek (nastepca zachowuje swoja dlugosc), C1 - rowne wektory. Dla G1 nowa styczna to unit(driver) * |follower|, dla C1 to driver (np. P1' = 2 V - P2 miedzy dwoma Beziera).");
        paragraph("ContinuityEnforcer sprawdza wierzcholki dotkniete edycja (kolejka robocza). Jesli relacja jest zlamana, jedna z krawedzi (follower) dopasowuje sie do drugiej (driver): krawedz przeciagana przez uzytkownika jest zawsze driverem, a w pozostalych przypadkach ustepuje ta o mniejszym adaptCost: luk (0) < Bezier (1) < odcinek (2). "
                  "Bezier dopasowuje sie przesuwajac swoj punkt kontrolny, luk zapamietujac kotwice, odcinek obracajac sie (i ewentualnie skalujac) wokol wspolnego wierzcholka, czyli przesuwajac drugi koniec. Przesuniete wierzcholki trafiaja do kolejki razem z sasiadami. Limit: 1000 krokow.");
        paragraph("Dozwolone klasy w wierzcholku okresla ContinuityPolicy: tylko przy krzywej, luk nie ma C1, tylko jeden koniec luku G1, miedzy dwoma lukami nie ma G1. Po zmianie krawedzi (zmiana typu, podzial, usuniecie wierzcholka) sanitize nadaje domyslne klasy, obniza niemozliwe wybory uzytkownika i zwalnia kotwice lukow.");

        heading("Ograniczenia geometryczne (relacje)");
        paragraph("ConstraintSolver wykonuje do 50 przebiegow Gaussa-Seidla: kazde ograniczenie po kolei rzutuje (project) uklad tak, by jego relacja zachodzila, co moze zepsuc sasiednie ograniczenia, wiec przebiegi powtarza sie az nic sie nie rusza. Wierzcholki przypiete (ten pod kursorem) nie sa przesuwane.");
        bullet("Dlugosc: poprawka (dlugosc - zadana) jest dzielona po rowno miedzy konce odcinka; przypiety koniec stoi, a drugi przesuwa sie wzdluz krawedzi.");
        bullet("Rownoleglosc: katem theta jest kat miedzy prostymi (modulo pi: przeciwny kierunek tez jest rownolegly). Kazda krawedz obraca sie o polowe theta (wzgledem swojej wagi) wokol przypietego konca albo wokol srodka; krawedz z obydwoma koncami przypietymi nie obraca sie, a druga obraca sie o cale theta.");
        paragraph("Punkty kontrolne sasiednich Beziera przesuwaja sie razem z wierzcholkami, wiec ograniczenia nie psuja ksztaltu krzywych.");

        heading("Wspolpraca ciaglosci i ograniczen");
        paragraph("Ograniczenie moze obrocic odcinek (a wtedy Bezier przy jego koncach musi to sledzic), a ciaglosc moze obrocic odcinek (a wtedy ograniczenia trzeba sprawdzic ponownie). PolygonEditor powtarza wiec obie reguly w petli do punktu stalego: runda = ContinuityEnforcer, potem ConstraintSolver; kolejna runda zaczyna sie od wierzcholkow przesunietych w poprzedniej. "
                  "Koniec, gdy nic sie nie rusza, albo po 20 rundach. Driver (przeciagana krawedz) i wierzcholki przypiete obowiazuja we wszystkich rundach, dzieki czemu przeciagany element zostaje pod kursorem nawet w ukladzie sprzecznym (wtedy zostaje blad resztkowy).");
    }

    void extendingTab()
    {
        heading("Scena poczatkowa");
        paragraph("Scena jest budowana w Scene::initPredefinedScene() (src/scene/scene.cpp). Wielokat to jedna linia: Polygon& p = addPolygon(PolygonDirector::createDemoPolygonN()); wielokaty buduje PolygonBuilder w PolygonDirector (addVertex, withLastEdgeAsBezier, withLastEdgeAsArc, closePolygon).");
        paragraph("Ograniczenie to jedna linia: constraints.add(std::make_unique<LengthConstraint>(p.getEdge(i), 150.0f)); albo constraints.add(std::make_unique<ParallelConstraint>(p.getEdge(i), q.getEdge(j))); krawedz i leci od wierzcholka i do nastepnego. Polozenia z kodu nie musza spelniac ograniczen: solver dopasowuje je przy starcie i po resecie.");
        paragraph("Scena demo zawiera dwa wielokaty: trojkat z krawedzia Beziera (G1 w wierzcholku miedzy krzywa a odcinkiem) oraz romb z lukiem, a takze cztery ograniczenia, w tym relacje miedzy wielokatami i dwa ograniczenia na jednej krawedzi.");

        heading("Nowe elementy");
        bullet("Nowy rodzaj ograniczenia (np. prostopadlosc): klasa dziedziczaca po Constraint (edges, isSatisfied, project, label), bez zmian w solverze i w ConstraintSystem.");
        bullet("Nowa klasa ciaglosci: nowa ContinuityStrategy i wpis w ContinuityStrategy::get.");
        bullet("Nowy rodzaj krawedzi: klasa dziedziczaca po Edge i wpis w createEdge (EdgeFactory).");
        bullet("Nowa regula po edycji: klasa dziedziczaca po PostEditRule dodana przez PolygonEditor::addRule (np. w main.cpp).");

        heading("Testy");
        paragraph("make test buduje i uruchamia testy jednostkowe z katalogu tests (bez okna): matematyka ciaglosci i krawedzi, ograniczenia, wspolpraca solvera z ciagloscia, API ograniczen edytora i scena poczatkowa.");
    }
}

void drawDocumentation(bool &show)
{
    if (!show)
        return;

    ImGui::SetNextWindowSize(ImVec2(760.0f, 600.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Dokumentacja projektu", &show);

    if (ImGui::BeginTabBar("DocumentationTabs"))
    {
        if (ImGui::BeginTabItem("Instrukcja obslugi"))
        {
            usageTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Zalozenia"))
        {
            assumptionsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Model danych"))
        {
            modelTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Algorytmy"))
        {
            algorithmsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Rozszerzanie"))
        {
            extendingTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
