CXX = g++
CPPFLAGS = -g -O3 -std=c++17 -fsanitize=address
LDFLAGS = -lssl -lcrypto

.PHONY: all clean

all: bob alice

bob: bob.cpp
	$(CXX) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

alice: alice.cpp
	$(CXX) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm bob alice
