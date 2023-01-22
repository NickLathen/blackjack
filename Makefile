CPP=gcc
CXX=gcc
SANITIZE=-fsanitize=undefined
WARNALL=-Wall -Wextra -Wpedantic
DEBUG=-g
STD=--std=c++17
LDLIBS=-lstdc++ -lncurses
CXXFLAGS=$(STD) $(SANITIZE) $(WARNALL) $(DEBUG)

#implicit rules
#  %.o : %.cc
# 	$(CXX) $(CPPFLAGS) $< $(CXXFLAGS) -c -o $@
#  % : %.o
# 	$(CXX) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

%.i : %.cc
	$(CXX) $(CPPFLAGS) $< $(CXXFLAGS) -E -o $@
