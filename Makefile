CXX ?= clang++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude

BUILD_DIR := build
CORE_SOURCES := src/lexer.cpp src/parser.cpp src/interpreter.cpp src/runtime.cpp
APP_SOURCES := $(CORE_SOURCES) src/main.cpp
TEST_SOURCES := $(CORE_SOURCES) tests/test_dpp.cpp

.PHONY: all test

all: $(BUILD_DIR)/dpp

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/dpp: $(APP_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(APP_SOURCES) -o $@

$(BUILD_DIR)/test_dpp: $(TEST_SOURCES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $@

test: $(BUILD_DIR)/test_dpp
	./$(BUILD_DIR)/test_dpp
