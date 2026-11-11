CXX = g++
CXXFLAGS = -Iinclude -Wall -Wextra -std=c++17 -pthread
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -pthread

SRC_DIR = src
OBJ_DIR = build

SRCS = main.cpp $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.cpp=.o)))
TARGET = $(OBJ_DIR)/TransitOpt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

VPATH = . $(SRC_DIR)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

run: all
	./$(TARGET)