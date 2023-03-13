CXX = g++
CXX_FLAGS = -Wall -g -fsanitize=address -fsanitize=undefined -O0 -std=c++20 -rdynamic
#CXX_FLAGS = -Wall -O3 -std=c++20 -march=corei7
CXX_INCLUDES = -Ivendor/fmt/include

BIN := main
BUILD_DIR := $(CURDIR)/build
SOURCE_DIR := $(CURDIR)/src
VENDOR_DIR := $(CURDIR)/vendor

SRC_FILES := $(shell find $(SOURCE_DIR) -name "*.cpp")

VNDR_FILES := $(shell find $(VENDOR_DIR) -name "*.cc" -o -name "*.cpp")
VNDR_OBJ_FILES := $(VNDR_FILES:${VENDOR_DIR}/%.cc=${BUILD_DIR}/vendor/%.o)

OBJ_FILES := $(SRC_FILES:$(SOURCE_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEP_FILES := $(OBJ_FILES:%.o=%.d)

$(BIN) : $(OBJ_FILES) $(VNDR_OBJ_FILES)
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $^ -o $@

-include $(DEP_FILES)

$(BUILD_DIR)/%.o : $(SOURCE_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -MMD -c $< -o $@

$(BUILD_DIR)/vendor/%.o : ${VENDOR_DIR}/%.cc
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -MMD -c $< -o $@

.PHONY : clean
clean :
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)