# ============================
# 🛠️  Makefile for Arbitrage Scanner
# Supports modular C++20 project with SSL (Windows, g++)
# ============================

# === Compiler & Flags ===
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -static
INCLUDES := -I../deps/include -I./src
LDFLAGS := -s -static -static-libgcc -static-libstdc++ -lssl -lcrypto -lws2_32 -lMswsock -lCrypt32

# === Directories ===
SRC_DIR := ./src
BUILD_DIR := ./bin
OBJ_DIR := ./obj

# === Source Files ===
MAIN_SRC := $(SRC_DIR)/main.cpp
CPP_SRCS := $(filter-out $(MAIN_SRC), $(wildcard $(SRC_DIR)/*.cpp))
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP_SRCS))

# === Target Executable ===
TARGET := $(BUILD_DIR)/arbitrage_scanner

# === Default target ===
all: $(TARGET)

# === Build main executable ===
$(TARGET): $(OBJS) $(MAIN_SRC) | $(BUILD_DIR)
	@echo "🔧 Linking $(TARGET)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(MAIN_SRC) $(OBJS) $(LDFLAGS)

# === Build object files ===
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "📦 Compiling $< -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# === Ensure directories exist ===
$(BUILD_DIR):
	@echo "📁 Creating build directory..."
	@mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	@echo "📁 Creating object directory..."
	@mkdir -p $(OBJ_DIR)

# === Clean rule ===
clean:
	@echo "🧹 Cleaning up..."
	@rm -rf $(BUILD_DIR) $(OBJ_DIR)

# === Run the scanner ===
run: $(TARGET)
	@echo "🚀 Running Arbitrage Scanner..."
	@$(TARGET)

# === Debug build (with debug symbols) ===
debug: CXXFLAGS := -std=c++20 -Wall -Wextra -g -O0 -DDEBUG
debug: $(TARGET)
	@echo "🐛 Debug build completed"

# === Release build (optimized) ===
release: CXXFLAGS := -std=c++20 -Wall -Wextra -O3 -DNDEBUG
release: clean $(TARGET)
	@echo "🚀 Release build completed"

# === Test configuration ===
test-config:
	@echo "🧪 Testing configuration..."
	@echo "Compiler: $(shell $(CXX) --version | head -n1)"
	@echo "SSL available: $(shell pkg-config --exists openssl && echo 'YES' || echo 'NO')"
	@echo "JSON available: $(shell pkg-config --exists nlohmann_json && echo 'YES' || echo 'NO')"

# === List source files ===
list-sources:
	@echo "📋 Source files:"
	@echo "Main: $(MAIN_SRC)"
	@echo "Modules:"
	@for f in $(CPP_SRCS); do echo "  > $$f"; done

# === Show build info ===
info:
	@echo "ℹ️  Build Information:"
	@echo "  Target: $(TARGET)"
	@echo "  Sources: $(words $(CPP_SRCS)) modules + main"
	@echo "  Objects: $(words $(OBJS)) files"
	@echo "  Compiler: $(CXX) $(CXXFLAGS)"
	@echo "  Libraries: SSL, Crypto, WS2_32, Mswsock, Crypt32"

# === Help ===
help:
	@echo "🔧 Arbitrage Scanner Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build the scanner executable"
	@echo "  clean            - Remove build artifacts"
	@echo "  debug            - Build with debug symbols"
	@echo "  release          - Build optimized release version"
	@echo "  run              - Build and run the scanner"
	@echo "  test-config      - Test build configuration"
	@echo "  list-sources     - List all source files"
	@echo "  info             - Show build information"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build the project"
	@echo "  make clean run    # Clean and run"
	@echo "  make debug        # Debug build"
	@echo "  make release      # Optimized build"

.PHONY: all clean run debug release test-config list-sources info help
