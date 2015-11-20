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
OBJS = $(SRC_DIR)/nethost.o $(SRC_DIR)/netrouter.o $(SRC_DIR)/netlink.o \
$(SRC_DIR)/netflow.o $(SRC_DIR)/packet.o $(SRC_DIR)/event.o \
$(SRC_DIR)/router_discovery_event.o $(SRC_DIR)/start_flow_event.o \
$(SRC_DIR)/timeout_event.o $(SRC_DIR)/send_packet_event.o \
$(SRC_DIR)/receive_packet_event.o $(SRC_DIR)/simulation.o \
$(SRC_DIR)/netnode.o $(SRC_DIR)/driver.o

# Update this list of source files every time a new .cpp is added to simulation
SRCS = $(SRC_DIR)/nethost.cpp $(SRC_DIR)/netrouter.cpp $(SRC_DIR)/netlink.cpp \
$(SRC_DIR)/netflow.cpp $(SRC_DIR)/packet.cpp $(SRC_DIR)/event.cpp \
$(SRC_DIR)/router_discovery_event.cpp $(SRC_DIR)/start_flow_event.cpp \
$(SRC_DIR)/timeout_event.cpp $(SRC_DIR)/send_packet_event.cpp \
$(SRC_DIR)/receive_packet_event.cpp $(SRC_DIR)/netnode.cpp \
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

src/nethost.o: src/nethost.h src/netnode.h src/netelement.h src/netlink.h
src/netrouter.o: src/netrouter.h src/netnode.h src/netelement.h src/netlink.h
src/netlink.o: src/netlink.h src/netelement.h
src/netflow.o: src/netflow.h src/netelement.h src/nethost.h src/netnode.h
src/netflow.o: src/netlink.h
src/packet.o: src/packet.h src/nethost.h src/netnode.h src/netelement.h
src/packet.o: src/netlink.h src/netflow.h
src/event.o: src/event.h
src/router_discovery_event.o: src/router_discovery_event.h src/netrouter.h
src/router_discovery_event.o: src/netnode.h src/netelement.h src/netlink.h
src/router_discovery_event.o: src/event.h src/simulation.h
src/router_discovery_event.o: rapidjson/document.h rapidjson/reader.h
src/router_discovery_event.o: rapidjson/rapidjson.h rapidjson/allocators.h
src/router_discovery_event.o: rapidjson/encodings.h rapidjson/internal/meta.h
src/router_discovery_event.o: rapidjson/rapidjson.h
src/router_discovery_event.o: rapidjson/internal/stack.h
src/router_discovery_event.o: rapidjson/internal/swap.h
src/router_discovery_event.o: rapidjson/internal/strtod.h
src/router_discovery_event.o: rapidjson/internal/ieee754.h
src/router_discovery_event.o: rapidjson/internal/biginteger.h
src/router_discovery_event.o: rapidjson/internal/diyfp.h
src/router_discovery_event.o: rapidjson/internal/pow10.h
src/router_discovery_event.o: rapidjson/error/error.h
src/router_discovery_event.o: rapidjson/internal/strfunc.h
src/router_discovery_event.o: rapidjson/prettywriter.h rapidjson/writer.h
src/router_discovery_event.o: rapidjson/internal/dtoa.h
src/router_discovery_event.o: rapidjson/internal/itoa.h
src/router_discovery_event.o: rapidjson/internal/itoa.h
src/router_discovery_event.o: rapidjson/stringbuffer.h src/nethost.h
src/router_discovery_event.o: src/netflow.h
src/start_flow_event.o: src/start_flow_event.h src/netflow.h src/netelement.h
src/start_flow_event.o: src/nethost.h src/netnode.h src/netlink.h src/event.h
src/start_flow_event.o: src/simulation.h rapidjson/document.h
src/start_flow_event.o: rapidjson/reader.h rapidjson/rapidjson.h
src/start_flow_event.o: rapidjson/allocators.h rapidjson/encodings.h
src/start_flow_event.o: rapidjson/internal/meta.h rapidjson/rapidjson.h
src/start_flow_event.o: rapidjson/internal/stack.h rapidjson/internal/swap.h
src/start_flow_event.o: rapidjson/internal/strtod.h
src/start_flow_event.o: rapidjson/internal/ieee754.h
src/start_flow_event.o: rapidjson/internal/biginteger.h
src/start_flow_event.o: rapidjson/internal/diyfp.h rapidjson/internal/pow10.h
src/start_flow_event.o: rapidjson/error/error.h rapidjson/internal/strfunc.h
src/start_flow_event.o: rapidjson/prettywriter.h rapidjson/writer.h
src/start_flow_event.o: rapidjson/internal/dtoa.h rapidjson/internal/itoa.h
src/start_flow_event.o: rapidjson/internal/itoa.h rapidjson/stringbuffer.h
src/start_flow_event.o: src/netrouter.h
src/timeout_event.o: src/timeout_event.h src/netflow.h src/netelement.h
src/timeout_event.o: src/nethost.h src/netnode.h src/netlink.h src/event.h
src/send_packet_event.o: src/send_packet_event.h src/netflow.h
src/send_packet_event.o: src/netelement.h src/nethost.h src/netnode.h
src/send_packet_event.o: src/netlink.h src/event.h src/simulation.h
src/send_packet_event.o: rapidjson/document.h rapidjson/reader.h
src/send_packet_event.o: rapidjson/rapidjson.h rapidjson/allocators.h
src/send_packet_event.o: rapidjson/encodings.h rapidjson/internal/meta.h
src/send_packet_event.o: rapidjson/rapidjson.h rapidjson/internal/stack.h
src/send_packet_event.o: rapidjson/internal/swap.h
src/send_packet_event.o: rapidjson/internal/strtod.h
src/send_packet_event.o: rapidjson/internal/ieee754.h
src/send_packet_event.o: rapidjson/internal/biginteger.h
src/send_packet_event.o: rapidjson/internal/diyfp.h
src/send_packet_event.o: rapidjson/internal/pow10.h rapidjson/error/error.h
src/send_packet_event.o: rapidjson/internal/strfunc.h
src/send_packet_event.o: rapidjson/prettywriter.h rapidjson/writer.h
src/send_packet_event.o: rapidjson/internal/dtoa.h rapidjson/internal/itoa.h
src/send_packet_event.o: rapidjson/internal/itoa.h rapidjson/stringbuffer.h
src/send_packet_event.o: src/netrouter.h src/packet.h
src/receive_packet_event.o: src/receive_packet_event.h src/netflow.h
src/receive_packet_event.o: src/netelement.h src/nethost.h src/netnode.h
src/receive_packet_event.o: src/netlink.h src/event.h src/simulation.h
src/receive_packet_event.o: rapidjson/document.h rapidjson/reader.h
src/receive_packet_event.o: rapidjson/rapidjson.h rapidjson/allocators.h
src/receive_packet_event.o: rapidjson/encodings.h rapidjson/internal/meta.h
src/receive_packet_event.o: rapidjson/rapidjson.h rapidjson/internal/stack.h
src/receive_packet_event.o: rapidjson/internal/swap.h
src/receive_packet_event.o: rapidjson/internal/strtod.h
src/receive_packet_event.o: rapidjson/internal/ieee754.h
src/receive_packet_event.o: rapidjson/internal/biginteger.h
src/receive_packet_event.o: rapidjson/internal/diyfp.h
src/receive_packet_event.o: rapidjson/internal/pow10.h
src/receive_packet_event.o: rapidjson/error/error.h
src/receive_packet_event.o: rapidjson/internal/strfunc.h
src/receive_packet_event.o: rapidjson/prettywriter.h rapidjson/writer.h
src/receive_packet_event.o: rapidjson/internal/dtoa.h
src/receive_packet_event.o: rapidjson/internal/itoa.h
src/receive_packet_event.o: rapidjson/internal/itoa.h
src/receive_packet_event.o: rapidjson/stringbuffer.h src/netrouter.h
src/receive_packet_event.o: src/packet.h
src/netnode.o: src/netnode.h src/netelement.h src/netlink.h
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
src/simulation.o: rapidjson/stringbuffer.h src/nethost.h src/netnode.h
src/simulation.o: src/netelement.h src/netlink.h src/netrouter.h
src/simulation.o: src/netflow.h src/event.h
src/driver.o: src/event.h src/simulation.h rapidjson/document.h
src/driver.o: rapidjson/reader.h rapidjson/rapidjson.h rapidjson/allocators.h
src/driver.o: rapidjson/encodings.h rapidjson/internal/meta.h
src/driver.o: rapidjson/rapidjson.h rapidjson/internal/stack.h
src/driver.o: rapidjson/internal/swap.h rapidjson/internal/strtod.h
src/driver.o: rapidjson/internal/ieee754.h rapidjson/internal/biginteger.h
src/driver.o: rapidjson/internal/diyfp.h rapidjson/internal/pow10.h
src/driver.o: rapidjson/error/error.h rapidjson/internal/strfunc.h
src/driver.o: rapidjson/prettywriter.h rapidjson/writer.h
src/driver.o: rapidjson/internal/dtoa.h rapidjson/internal/itoa.h
src/driver.o: rapidjson/internal/itoa.h rapidjson/stringbuffer.h
src/driver.o: src/nethost.h src/netnode.h src/netelement.h src/netlink.h
src/driver.o: src/netrouter.h src/netflow.h
test/alltests.o: test/test_jsonlib.cpp rapidjson/document.h
test/alltests.o: rapidjson/reader.h rapidjson/rapidjson.h
test/alltests.o: rapidjson/allocators.h rapidjson/encodings.h
test/alltests.o: rapidjson/internal/meta.h rapidjson/rapidjson.h
test/alltests.o: rapidjson/internal/stack.h rapidjson/internal/swap.h
test/alltests.o: rapidjson/internal/strtod.h rapidjson/internal/ieee754.h
test/alltests.o: rapidjson/internal/biginteger.h rapidjson/internal/diyfp.h
test/alltests.o: rapidjson/internal/pow10.h rapidjson/error/error.h
test/alltests.o: rapidjson/internal/strfunc.h rapidjson/prettywriter.h
test/alltests.o: rapidjson/writer.h rapidjson/internal/dtoa.h
test/alltests.o: rapidjson/internal/itoa.h rapidjson/internal/itoa.h
test/alltests.o: rapidjson/stringbuffer.h test/test_simulation_input.cpp
test/alltests.o: src/netelement.h src/nethost.h src/netnode.h src/netlink.h
test/alltests.o: src/netrouter.h src/netflow.h test/test_simulation.cpp
test/alltests.o: src/simulation.h src/event.h test/test_event.cpp
