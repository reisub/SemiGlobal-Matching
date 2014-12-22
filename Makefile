INC=`pkg-config --cflags opencv`
OPTIONS=`pkg-config --libs opencv`
CXXFLAGS=-c -O3 -std=c++11 -Wall -Wextra
LDFLAGS=-O3
SOURCES=gaussian.cc main.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=sgm

all: $(SOURCES) $(EXECUTABLE)

.cc.o:
	$(CXX) $(CXXFLAGS) $< -o $@ $(INC)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(INC) $(OPTIONS)

bull: $(EXECUTABLE)
	./$(EXECUTABLE) test/bull/left.png test/bull/right.png 32

venus: $(EXECUTABLE)
	./$(EXECUTABLE) test/venus/left.png test/venus/right.png 32

cones: $(EXECUTABLE)
	./$(EXECUTABLE) test/cones/left.png test/cones/right.png 64

teddy: $(EXECUTABLE)
	./$(EXECUTABLE) test/teddy/left.png test/teddy/right.png 64

clean:
	-rm $(OBJECTS) $(EXECUTABLE)

.PHONY: clean