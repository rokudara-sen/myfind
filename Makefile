# Makefile for myfind

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

TARGET = myfind

all: $(TARGET)

$(TARGET): myfind.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) myfind.cpp

clean:
	rm -f $(TARGET)

