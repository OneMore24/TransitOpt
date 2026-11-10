CXX = g++
CXXFLAGS = -Iinclude -Wall -Wextra -std=c++17 -pthread
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -pthread

SRC_DIR = src
OBJ_DIR = build

# Toma todos los .cpp de la carpeta src
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(OBJ_DIR)/TransitOpt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

run: all
	./$(TARGET)