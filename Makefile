COMPILER ?= g++

BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
GLAD_DIR = lib/glm
IMGUI_DIR = lib/imgui

PYTHON_DIR = /usr/include/python3.10
NUMPY_DIR = /home/srinidhi/.local/lib/python3.10/site-packages/numpy/_core/include

PYTHON_CFLAGS = $(shell python3-config --cflags)

CXXFLAGS = -std=c++17 -g -O0 \
           -I$(INCLUDE_DIR) \
		   -I$(GLAD_DIR) \
		   -I$(IMGUI_DIR) \
		   -fsanitize=address \
		   -fno-omit-frame-pointer

LDFLAGS = -lglfw -lGL -fsanitize=address

TARGET = $(BUILD_DIR)/main

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(BUILD_DIR)/phyVector.o $(BUILD_DIR)/guidance.o $(BUILD_DIR)/cguidance.o $(BUILD_DIR)/phySim.o $(BUILD_DIR)/glad.o $(BUILD_DIR)/gltest.o $(BUILD_DIR)/quaternion.o $(BUILD_DIR)/simObject.o $(BUILD_DIR)/simBody.o $(BUILD_DIR)/imgui.o $(BUILD_DIR)/imgui_draw.o $(BUILD_DIR)/imgui_tables.o $(BUILD_DIR)/imgui_widgets.o $(BUILD_DIR)/imgui_demo.o $(BUILD_DIR)/imgui_impl_glfw.o $(BUILD_DIR)/imgui_impl_opengl3.o
	$(COMPILER) $^  -o $@ $(LDFLAGS)

$(BUILD_DIR)/phyVector.o: $(SRC_DIR)/phyVector.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/guidance.o: $(SRC_DIR)/guidance.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/cguidance.o: $(SRC_DIR)/cguidance.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/phySim.o: $(SRC_DIR)/phySim.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/glad.o: $(SRC_DIR)/glad.c | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/gltest.o: $(SRC_DIR)/gltest.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/quaternion.o: $(SRC_DIR)/quaternion.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/simObject.o: $(SRC_DIR)/simObject.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/simBody.o: $(SRC_DIR)/simBody.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/imgui.o: $(IMGUI_DIR)/imgui.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/imgui_draw.o: $(IMGUI_DIR)/imgui_draw.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/imgui_tables.o: $(IMGUI_DIR)/imgui_tables.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/imgui_widgets.o: $(IMGUI_DIR)/imgui_widgets.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/imgui_demo.o: $(IMGUI_DIR)/imgui_demo.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/imgui_impl_glfw.o: $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

$(BUILD_DIR)/imgui_impl_opengl3.o: $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp | $(BUILD_DIR)
	$(COMPILER) -c $< $(CXXFLAGS) -o $@ 

clean:
	rm -rf $(BUILD_DIR)