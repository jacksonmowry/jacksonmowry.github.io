##
#shell
#

CFLAGS = -Wall -Wpedantic -Wextra  -Iinclude
ANALYZERFLAGS = -g -fanalyzer
SANITIZERFLAGS = -g -fsanitize=address

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
EXEC = $(BUILD_DIR)/main
all: $(EXEC)

# Link object files into final executable
    $(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Compile all source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build with gccs -fanalyzer
analyzer: CFLAGS += $(ANALYZERFLAGS)
analyzer: $(EXEC)

sanitizer: CC = clang
sanitizer: CFLAGS += $(SANITIZERFLAGS)
sanitizer: $(EXEC)

# Build tests
# NO-OP for now

clean:
	rm -f $(BUILD_DIR)/*.o $(EXEC) $(BUILD_DIR)/tests

.PHONY: all clean analyzer sanitizer tests
# end
