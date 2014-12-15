INC=`pkg-config --cflags opencv`
OPTIONS=`pkg-config --libs opencv`
CXXFLAGS=-c -O3 -fopenmp -std=c++11 -Wall -Wextra
LDFLAGS=
SOURCES=gaussian.cc main.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

.cc.o:
	$(CXX) $(CXXFLAGS) $< -o $@ $(INC)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(INC) $(OPTIONS)

simple: $(EXECUTABLE)
	./$(EXECUTABLE) test/simple/left.png test/simple/right.png 4

cones: $(EXECUTABLE)
	./$(EXECUTABLE) test/cones/im2.png test/cones/im6.png 64

teddy: $(EXECUTABLE)
	./$(EXECUTABLE) test/teddy/im2.png test/teddy/im6.png 64

bull: $(EXECUTABLE)
	./$(EXECUTABLE) test/bull/im2.png test/bull/im6.png 32

venus: $(EXECUTABLE)
	./$(EXECUTABLE) test/venus/im2.png test/venus/im6.png 32

clean:
	-rm $(OBJECTS) $(EXECUTABLE)

.PHONY: clean