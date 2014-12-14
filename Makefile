INC=`pkg-config --cflags opencv`
OPTIONS=`pkg-config --libs opencv`
CXXFLAGS=-c -O3 -std=c++11 -Wall -Wextra
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
	./$(EXECUTABLE) test/simple/left.png test/simple/right.png

cones: $(EXECUTABLE)
	./$(EXECUTABLE) test/cones/im2.png test/cones/im6.png

teddy: $(EXECUTABLE)
	./$(EXECUTABLE) test/teddy/im2.png test/teddy/im6.png

bull: $(EXECUTABLE)
	./$(EXECUTABLE) test/bull/im2.png test/bull/im6.png

venus: $(EXECUTABLE)
	./$(EXECUTABLE) test/venus/im2.png test/venus/im6.png

clean:
	-rm $(OBJECTS) $(EXECUTABLE)

.PHONY: clean