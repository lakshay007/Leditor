CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lncurses

TARGET = leditor
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# Default target builds terminal version
all: $(BUILD_DIR) $(TARGET)

# Terminal version
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# GUI version targets
gui: gui-build

gui-build:
	cd gui && make

gui-run: gui-build
	./leditor-gui

gui-clean:
	cd gui && make clean

# Combined targets
build-all: all gui-build

clean: gui-clean
	rm -rf $(BUILD_DIR) $(TARGET)

run-terminal: $(TARGET)
	./$(TARGET)

run-gui: gui-run

.PHONY: all clean run-terminal run-gui gui gui-build gui-run gui-clean build-all 