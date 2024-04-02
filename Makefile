WORK_DIR  = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
TEST_DIR = $(WORK_DIR)/test

TEST_OBJS = $(TEST_DIR)/test.out $(TEST_DIR)/dummy.out $(TEST_DIR)/openmp.out $(TEST_DIR)/thread.out
 
CXX_TEST_FLAGS = -fopenmp

$(TEST_OBJS) : %out:%cc
	$(CXX) $(CXX_TEST_FLAGS) $< -o $@

ALL = test

EXE = $(WORK_DIR)/test/$(ALL).out

ARGS = --backend=stgen --executable=$(EXE)

CSRCS = $(shell find $(abspath ./src) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

$(shell mkdir -p $(BUILD_DIR))

BIN = $(BUILD_DIR)/bin/prism 

PRSIM_EXEC := $(BIN) $(ARGS) > $(ALL).txt


$(BIN): $(CSRCS)
	$(shell rm $(BUILD_DIR)/src/Backends/SynchroTraceGen/libSTGen.a $(BUILD_DIR)/src/Backends/SynchroTraceGen/libSTGenCore.a)
	cd $(BUILD_DIR) && cmake $(WORK_DIR)
	make -C $(BUILD_DIR) -j

default: $(BIN)

run: $(TEST_OBJS) $(BIN)
	$(PRSIM_EXEC)

clean:
	rm -rf $(BUILD_DIR)