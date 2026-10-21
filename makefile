# Kompilator
CXX = g++

# Flagi kompilacji (c++20, ostrzeżenia, ścieżki do nagłówków)
CXXFLAGS = -std=c++20 -Wall -g -I./imgui -I./src -I./SFML/include

# Pliki źródłowe (Twoje pliki + pliki biblioteki)
SRC = src/main.cpp \
      imgui/imgui.cpp \
      imgui/imgui_draw.cpp \
      imgui/imgui_tables.cpp \
      imgui/imgui_widgets.cpp \
      imgui/imgui-SFML.cpp

# Pliki obiektowe (zamienia .cpp na .o)
OBJ = $(SRC:.cpp=.o)

# Flagi linkera dla Windows (MinGW)
LDFLAGS = -L./SFML/lib -lsfml-graphics -lsfml-window -lsfml-system -lopengl32

# Nazwa pliku wykonywalnego z rozszerzeniem Windowsowym
EXEC = app.exe

# Główna reguła
all: $(EXEC)

# Łączenie wszystkiego w plik wykonywalny .exe
$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Kompilacja plików .cpp do .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Czyszczenie dostosowane do Windowsa (PowerShell)
# Czyszczenie (znak '-' na początku linijki nakazuje make ignorować błędy, gdy plików już nie ma)
clean:
	-powershell -NoProfile -Command "Remove-Item src/*.o, imgui/*.o, $(EXEC) -Force -ErrorAction SilentlyContinue; exit 0"