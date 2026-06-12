COMPILER ?= g++

BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

PYTHON_DIR = /usr/include/python3.10
NUMPY_DIR = /home/srinidhi/.local/lib/python3.10/site-packages/numpy/_core/include

CXXFLAGS = -std=c++17 -g -O0 \
           -I$(INCLUDE_DIR) \
           -I$(PYTHON_DIR) \
           -I$(NUMPY_DIR)

LDFLAGS = $(shell python3-config --embed --ldflags)

TARGET = $(BUILD_DIR)/main

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(BUILD_DIR)/phyVector.o $(BUILD_DIR)/guidance.o
	$(COMPILER) $^ $(LDFLAGS) -o $@ -lpython3.10

$(BUILD_DIR)/phyVector.o: $(SRC_DIR)/phyVector.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ -lpython3.10

$(BUILD_DIR)/guidance.o: $(SRC_DIR)/guidance.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ -lpython3.10

clean:
	rm -rf $(BUILD_DIR)