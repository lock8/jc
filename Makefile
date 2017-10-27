SHELL   := bash
DIFF    := diff
AFLCC   := afl-gcc
AFLFUZZ := afl-fuzz

BUILD_DIR    := $(PWD)/build
TEST_DIR     := $(PWD)/test
EXAMPLES_DIR := $(PWD)/examples

CFLAGS := --std=c89 -Wall -pedantic -g -Isrc

EXAMPLES         := tokenizer
EXAMPLE_PROGRAMS := $(addprefix $(BUILD_DIR)/, $(EXAMPLES))

TEST_PROGRAM := $(BUILD_DIR)/test
TEST_CASES   := $(addsuffix .case, $(subst .in.txt,, $(notdir $(wildcard $(TEST_DIR)/cases/*.in.txt))))


all: test examples

.PHONY: test
test: $(TEST_CASES)

.PHONY: fuzzytest
fuzzytest: $(BUILD_DIR)
	$(AFLCC) $(CFLAGS) $(TEST_DIR)/test.c -o $(BUILD_DIR)/afl-test
	mkdir -p $(BUILD_DIR)/findings
	$(AFLFUZZ) -i $(TEST_DIR)/afl-cases -o $(BUILD_DIR)/findings -- $(BUILD_DIR)/afl-test @@

.PHONY: examples
examples: $(BUILD_DIR) $(EXAMPLE_PROGRAMS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%: $(BUILD_DIR)/%.o src/jc.h $(BUILD_DIR)
	$(CC) $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLES_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: %.case
%.case: $(TEST_DIR)/cases/%.in.txt $(TEST_DIR)/cases/%.out.txt $(TEST_PROGRAM)
	$(DIFF) $(word 2, $?) <($(TEST_PROGRAM) $<) || (echo "FAILED: $@" && exit 1)
	echo "PASSED: $@"

