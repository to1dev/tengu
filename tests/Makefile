# ============================
# 🛠️  Makefile for Tengu Test Suite
# Supports multiple C++20 test programs (Windows, g++)
# ============================

# === Compiler & Flags ===
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -static
INCLUDES := -I./deps/include
LDFLAGS := -s -static -static-libgcc -static-libstdc++ -lssl -lcrypto -lws2_32 -lMswsock -lCrypt32

# === Source & Output Rules ===
SRC_DIR := .
BUILD_DIR := ./bin

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
BINS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%, $(SRCS))

# === Default target ===
all: $(BINS)

# === Build rule for each .cpp => executable ===
$(BUILD_DIR)/%: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "🔧 Compiling $< -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

# === Ensure bin directory exists ===
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# === Clean rule ===
clean:
	@echo "🧹 Cleaning up..."
	@rm -f $(BUILD_DIR)/*

# === List available targets ===
list:
	@echo "📦 Available programs:"
	@for f in $(BINS); do echo "  > $$f"; done

.PHONY: all clean list
