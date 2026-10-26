CXX = g++
CXXFLAGS = -std=c++20 -Wall -g -fsanitize=address -MMD -MP -I./imgui -I./src
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

# Testy jednostkowe (bez okna i bez ImGui); kazdy plik tests/test_<nazwa>.cpp daje osobny program
TEST_LIB_SRC = $(filter-out src/main.cpp src/ui/% src/scene/%, $(SRC_FILES))
TEST_NAMES = continuity constraints solver

test:
	@for t in $(TEST_NAMES); do \
		$(CXX) $(CXXFLAGS) tests/test_$$t.cpp $(TEST_LIB_SRC) -o tests/run_$${t}_tests $(LDFLAGS) || exit 1; \
		./tests/run_$${t}_tests || exit 1; \
	done

.PHONY: test clean

# Reguła czyszcząca
clean:
	rm -f $(OBJS) $(OBJS:.o=.d) app

# zaleznosci od naglowkow (generowane przez -MMD), zeby zmiana .hpp przebudowywala zalezne pliki
-include $(OBJS:.o=.d)
