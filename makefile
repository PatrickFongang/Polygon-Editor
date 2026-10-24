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

# Reguła czyszcząca
clean:
	rm -f $(OBJS) app