CXX = g++
CXXFLAGS = -std=c++20 -Wall -g -fsanitize=address -I./imgui -I./src
LDFLAGS = -fsanitize=address -lsfml-graphics -lsfml-window -lsfml-system -lGL

# 1. Automatyczne wyszukiwanie wszystkich plików .cpp w podfolderach
SRC_FILES = $(shell find src -name '*.cpp')
IMGUI_FILES = $(wildcard imgui/*.cpp)

# 2. Zamiana rozszerzeń .cpp na .o dla plików obiektowych
OBJS = $(SRC_FILES:.cpp=.o) $(IMGUI_FILES:.cpp=.o)

# Główny cel kompilacji
app: $(OBJS)
	$(CXX) $(OBJS) -o app $(LDFLAGS)

# Reguła kompilacji dla każdego pliku .cpp do .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Testy matematyki ciągłości (bez okna i bez ImGui)
TEST_SRC = tests/test_continuity.cpp src/model/vertex.cpp src/model/edge/edge.cpp src/model/edge/line_edge.cpp src/model/edge/bezier_edge.cpp src/model/edge/arc_edge.cpp src/model/polygon.cpp src/builder/polygon_builder.cpp src/editing/polygon_editor.cpp src/editing/continuity_enforcer.cpp

test: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o tests/run_tests $(LDFLAGS)
	./tests/run_tests

# Reguła czyszcząca
clean:
	rm -f $(OBJS) app