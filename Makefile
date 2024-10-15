CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: servermain.cpp client.cpp
	$(CXX) $(CXXFLAGS) servermain.cpp -o servermain
	$(CXX) $(CXXFLAGS) client.cpp -o client

clean:
	$(RM) servermain client