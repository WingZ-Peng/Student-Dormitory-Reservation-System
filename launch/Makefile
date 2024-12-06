CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: servermain.cpp client.cpp
	$(CXX) $(CXXFLAGS) serverA.cpp -o serverA
	$(CXX) $(CXXFLAGS) serverB.cpp -o serverB
	$(CXX) $(CXXFLAGS) serverC.cpp -o serverC
	$(CXX) $(CXXFLAGS) servermain.cpp -o servermain
	$(CXX) $(CXXFLAGS) client.cpp -o client
	$(CXX) $(CXXFLAGS) guest.cpp -o guest

clean:
	$(RM) servermain client serverA serverB serverC guest