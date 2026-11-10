CXX = g++
CXXFLAGS = -Iinclude -Wall -Wextra -std=c++17 -pthread
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -pthread

SRC_DIR = src
OBJ_DIR = build

# Busca main.cpp en la raiz y los demas en src/
SRCS = main.cpp $(wildcard $(SRC_DIR)/*.cpp)

# notdir quita la ruta del archivo, asi q src/Game.cpp se combierte en Game.o
# addprefix le pone build/ adelante, quedando build/Game.o
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.cpp=.o)))

TARGET = $(OBJ_DIR)/TransitOpt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# VPATH le dice a make q busque los .cpp en la raiz y en src/
VPATH = . $(SRC_DIR)

# esta regla compila cualquier .o desde su .cpp correspondiente
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

run: all
	./$(TARGET)