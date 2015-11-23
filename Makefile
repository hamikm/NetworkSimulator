# Author: Hamik Mukelyan 
# For: CS143 project, Caltech, FA 2015

# For warnings, gdb, and valgrind, respectively.
CXX_FLAGS = -Wall -g -O0

# JSON input files for simulation.
INPUT_DIR = input_files

# Simulation's data dump directory.
DATA_DIR = data_dump

# Auto-generated documentation directory
DOCS = docs

# Graphs of data in $(DATA_DIR) live here.
GRAPH_DIR = graphs

# Source code directory.
SRC_DIR = src

# Unit tests (Google C++ ones) live here.
TST_DIR = test

# Google C++ Test library. Is used for unit testing.
GT_DIR = gtest-1.5.0

# Unit tests need to be linked with this.
GT_OBJ = gtest-all.o

# JSON parsing library.
JSON_LIB = .

# Name of simulation binary.
NETSIM = netsim
  
# Name of binary that runs all unit tests.
TESTS = tests

# Update this list of object files every time a new .cpp is added to simulation
OBJS = $(SRC_DIR)/network.o $(SRC_DIR)/events.o \
$(SRC_DIR)/simulation.o $(SRC_DIR)/driver.o

# Update this list of source files every time a new .cpp is added to simulation
SRCS = $(SRC_DIR)/network.cpp $(SRC_DIR)/events.cpp \
$(SRC_DIR)/simulation.cpp $(SRC_DIR)/driver.cpp

# Makes the simulation binary as well as the unit test binary.
all: $(NETSIM) $(TESTS)

# Makes just the network simulation binary.
$(NETSIM): $(OBJS)
	$(CXX) $(CXX_FLAGS) $(CPP_FLAGS) $(OBJS) -o $(NETSIM)

# Makes just the unit tests binary.
$(TESTS): $(TST_DIR)/alltests.o $(GT_DIR)/make/$(GT_OBJ) \
$(filter-out $(SRC_DIR)/driver.o, $(OBJS))
	$(CXX) $(CXX_FLAGS) $(CPP_FLAGS) $(GT_DIR)/make/$(GT_OBJ) \
	$(TST_DIR)/alltests.o $(filter-out $(SRC_DIR)/driver.o, $(OBJS)) \
	-lpthread -o $(TESTS)

# Make object files declared in the list of object files
%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(CPP_FLAGS) -I$(JSON_LIB) -c $< -o $@

# Make alltests.o
$(TST_DIR)/alltests.o: $(TST_DIR)/alltests.cpp
	$(CXX) $(CXX_FLAGS) $(CPP_FLAGS) -I$(JSON_LIB) -I$(SRC_DIR) -I$(TST_DIR) \
	-I$(GT_DIR)/include -c $(TST_DIR)/alltests.cpp \
	-o $(TST_DIR)/alltests.o

# Make Google C++ testing framework object file required for unit tests
$(GT_DIR)/make/$(GT_OBJ): 
	make -C $(GT_DIR)/make

.PHONY: clean docs depend

clean:
	rm -rf *~ *.o $(NETSIM) $(TESTS) $(DOCS) $(TST_DIR)/*.o Makefile.bak \
	$(TST_DIR)/*~ $(SRC_DIR)/*~ $(SRC_DIR)/*.o $(INPUT_DIR)/*~
	make clean -C $(GT_DIR)/make

# Auto-generates HTML documentation into the docs directory using doxygen.
docs:
	doxygen

# Automatically and recursively determines the dependencies of each source file
# then writes them below the "DO NOT DELETE" line in this makefile.
depend:
	makedepend $(CXX_FLAGS) $(CPP_FLAGS) -Y -I$(SRC_DIR) -I$(TST_DIR) \
	$(SRCS) $(TST_DIR)/alltests.cpp

# DO NOT DELETE

src/network.o: src/network.h src/util.h src/simulation.h rapidjson/document.h
src/network.o: rapidjson/reader.h rapidjson/rapidjson.h
src/network.o: rapidjson/allocators.h rapidjson/encodings.h
src/network.o: rapidjson/internal/meta.h rapidjson/rapidjson.h
src/network.o: rapidjson/internal/stack.h rapidjson/internal/swap.h
src/network.o: rapidjson/internal/strtod.h rapidjson/internal/ieee754.h
src/network.o: rapidjson/internal/biginteger.h rapidjson/internal/diyfp.h
src/network.o: rapidjson/internal/pow10.h rapidjson/error/error.h
src/network.o: rapidjson/internal/strfunc.h rapidjson/prettywriter.h
src/network.o: rapidjson/writer.h rapidjson/internal/dtoa.h
src/network.o: rapidjson/internal/itoa.h rapidjson/internal/itoa.h
src/network.o: rapidjson/stringbuffer.h src/events.h
src/events.o: src/events.h src/util.h src/network.h src/simulation.h
src/events.o: rapidjson/document.h rapidjson/reader.h rapidjson/rapidjson.h
src/events.o: rapidjson/allocators.h rapidjson/encodings.h
src/events.o: rapidjson/internal/meta.h rapidjson/rapidjson.h
src/events.o: rapidjson/internal/stack.h rapidjson/internal/swap.h
src/events.o: rapidjson/internal/strtod.h rapidjson/internal/ieee754.h
src/events.o: rapidjson/internal/biginteger.h rapidjson/internal/diyfp.h
src/events.o: rapidjson/internal/pow10.h rapidjson/error/error.h
src/events.o: rapidjson/internal/strfunc.h rapidjson/prettywriter.h
src/events.o: rapidjson/writer.h rapidjson/internal/dtoa.h
src/events.o: rapidjson/internal/itoa.h rapidjson/internal/itoa.h
src/events.o: rapidjson/stringbuffer.h
src/simulation.o: src/simulation.h rapidjson/document.h rapidjson/reader.h
src/simulation.o: rapidjson/rapidjson.h rapidjson/allocators.h
src/simulation.o: rapidjson/encodings.h rapidjson/internal/meta.h
src/simulation.o: rapidjson/rapidjson.h rapidjson/internal/stack.h
src/simulation.o: rapidjson/internal/swap.h rapidjson/internal/strtod.h
src/simulation.o: rapidjson/internal/ieee754.h
src/simulation.o: rapidjson/internal/biginteger.h rapidjson/internal/diyfp.h
src/simulation.o: rapidjson/internal/pow10.h rapidjson/error/error.h
src/simulation.o: rapidjson/internal/strfunc.h rapidjson/prettywriter.h
src/simulation.o: rapidjson/writer.h rapidjson/internal/dtoa.h
src/simulation.o: rapidjson/internal/itoa.h rapidjson/internal/itoa.h
src/simulation.o: rapidjson/stringbuffer.h src/events.h src/util.h
src/simulation.o: src/network.h
src/driver.o: src/simulation.h rapidjson/document.h rapidjson/reader.h
src/driver.o: rapidjson/rapidjson.h rapidjson/allocators.h
src/driver.o: rapidjson/encodings.h rapidjson/internal/meta.h
src/driver.o: rapidjson/rapidjson.h rapidjson/internal/stack.h
src/driver.o: rapidjson/internal/swap.h rapidjson/internal/strtod.h
src/driver.o: rapidjson/internal/ieee754.h rapidjson/internal/biginteger.h
src/driver.o: rapidjson/internal/diyfp.h rapidjson/internal/pow10.h
src/driver.o: rapidjson/error/error.h rapidjson/internal/strfunc.h
src/driver.o: rapidjson/prettywriter.h rapidjson/writer.h
src/driver.o: rapidjson/internal/dtoa.h rapidjson/internal/itoa.h
src/driver.o: rapidjson/internal/itoa.h rapidjson/stringbuffer.h src/events.h
src/driver.o: src/util.h src/network.h
test/alltests.o: src/events.h src/util.h src/network.h src/simulation.h
test/alltests.o: rapidjson/document.h rapidjson/reader.h
test/alltests.o: rapidjson/rapidjson.h rapidjson/allocators.h
test/alltests.o: rapidjson/encodings.h rapidjson/internal/meta.h
test/alltests.o: rapidjson/rapidjson.h rapidjson/internal/stack.h
test/alltests.o: rapidjson/internal/swap.h rapidjson/internal/strtod.h
test/alltests.o: rapidjson/internal/ieee754.h rapidjson/internal/biginteger.h
test/alltests.o: rapidjson/internal/diyfp.h rapidjson/internal/pow10.h
test/alltests.o: rapidjson/error/error.h rapidjson/internal/strfunc.h
test/alltests.o: rapidjson/prettywriter.h rapidjson/writer.h
test/alltests.o: rapidjson/internal/dtoa.h rapidjson/internal/itoa.h
test/alltests.o: rapidjson/internal/itoa.h rapidjson/stringbuffer.h
test/alltests.o: test/test_jsonlib.cpp test/test_simulation_input.cpp
test/alltests.o: test/test_simulation.cpp test/test_event.cpp
test/alltests.o: test/test_packet.cpp test/test_link.cpp test/test_flow.cpp
