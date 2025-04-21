CXX = g++
CPPFLAGS = -g -O3 -std=c++17

.PHONY: all clean

all: bob alice

bob: bob.cpp
	$(CXX) $(CPPFLAGS) $< -o $@

alice: alice.cpp
	$(CXX) $(CPPFLAGS) $< -o $@

clean:
	rm bob alice
