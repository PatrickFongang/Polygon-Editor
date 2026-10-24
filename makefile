# Kompilator
CXX = g++

# Flagi kompilacji (c++20, ostrzeżenia, symbole debugowania, AddressSanitizer)
CXXFLAGS = -std=c++20 -Wall -g -fsanitize=address -I./imgui -I./src

# Pliki źródłowe (Twoje pliki + pliki biblioteki ImGui)
SRC = src/main.cpp \
      imgui/imgui.cpp \
      imgui/imgui_draw.cpp \
      imgui/imgui_tables.cpp \
      imgui/imgui_widgets.cpp \
      imgui/imgui-SFML.cpp

# Pliki obiektowe (zamienia .cpp na .o)
OBJ = $(SRC:.cpp=.o)

# Flagi linkera dla Linuksa (AddressSanitizer, systemowe SFML i OpenGL)
LDFLAGS = -fsanitize=address -lsfml-graphics -lsfml-window -lsfml-system -lGL

# Nazwa pliku wykonywalnego (na Linuksie przyjęto brak rozszerzenia .exe)
EXEC = app

# Główna reguła
all: $(EXEC)

# Łączenie wszystkiego w plik wykonywalny
$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Kompilacja plików .cpp do .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Czyszczenie dostosowane do Linuksa
clean:
	rm -f src/*.o imgui/*.o $(EXEC)