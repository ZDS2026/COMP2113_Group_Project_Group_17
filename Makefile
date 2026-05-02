CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
TARGET = mini_magic_tower
SRCS = main.cpp game.cpp common.cpp difficulty.cpp map.cpp renderer.cpp io.cpp input.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.exe save*.dat

.PHONY: all clean
