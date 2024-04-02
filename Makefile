WORK_DIR  = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
TEST_DIR = $(WORK_DIR)/test

TEST_OBJS = $(WORK_DIR)/test/test.out



$(TEST_OBJS) : %out:%c 
	$(CC) $< -o $@
ARGS = --backend=stgen --executable=$(TEST_OBJS)

CSRCS = $(shell find $(abspath ./src) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

$(shell mkdir -p $(BUILD_DIR))

BIN = $(BUILD_DIR)/bin/prism 

PRSIM_EXEC := $(BIN) $(ARGS)


$(BIN): $(CSRCS)
	$(shell rm $(BUILD_DIR)/src/Backends/SynchroTraceGen/libSTGen.a $(BUILD_DIR)/src/Backends/SynchroTraceGen/libSTGenCore.a)
	cd $(BUILD_DIR) && cmake $(WORK_DIR)
	make -C $(BUILD_DIR) -j

default: $(BIN)

run: $(TEST_OBJS) $(BIN)
	$(PRSIM_EXEC)

clean:
	rm -rf $(BUILD_DIR)