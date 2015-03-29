INC=`pkg-config --cflags opencv`
OPTIONS=`pkg-config --libs opencv`
CXXFLAGS=-c -O3 -std=c++11 -Wall -Wextra
LDFLAGS=-O3
SOURCES=gaussian.cc main.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=sgm
TEST_SOURCES=compare.cc
TEST_OBJECTS=$(TEST_SOURCES:.cc=.o)
TEST_EXECUTABLE=compare

all: $(SOURCES) $(EXECUTABLE) $(TEST_EXECUTABLE)

.cc.o:
	$(CXX) $(CXXFLAGS) $< -o $@ $(INC)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(INC) $(OPTIONS)

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	$(CXX) $(LDFLAGS) $(TEST_OBJECTS) -o $@ $(INC) $(OPTIONS)

bull: $(EXECUTABLE)
	./$(EXECUTABLE) test/bull/left.png test/bull/right.png out.png 32

venus: $(EXECUTABLE)
	./$(EXECUTABLE) test/venus/left.png test/venus/right.png out.png 32

cones: $(EXECUTABLE)
	./$(EXECUTABLE) test/cones/left.png test/cones/right.png out.png 64

teddy: $(EXECUTABLE)
	./$(EXECUTABLE) test/teddy/left.png test/teddy/right.png out.png 64

kitti: $(EXECUTABLE)
	./$(EXECUTABLE) test/kitti/left.png test/kitti/right.png out.png 256

clean:
	-rm $(OBJECTS) $(EXECUTABLE) $(TEST_OBJECTS) $(TEST_EXECUTABLE)

.PHONY: clean