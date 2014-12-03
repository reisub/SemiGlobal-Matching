INC=`pkg-config --cflags opencv`
OPTIONS=`pkg-config --libs opencv`
CXXFLAGS=-c -Wall -Wextra
LDFLAGS=
SOURCES=main.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

.cc.o:
	$(CXX) $(CXXFLAGS) $< -o $@ $(INC)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(INC) $(OPTIONS)

cones: $(EXECUTABLE)
	./$(EXECUTABLE) test/cones/im2.png test/cones/im6.png

teddy: $(EXECUTABLE)
	./$(EXECUTABLE) test/teddy/im2.png test/teddy/im6.png

clean:
	-rm $(OBJECTS) $(EXECUTABLE)

.PHONY: clean