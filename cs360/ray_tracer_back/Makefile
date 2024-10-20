##
#raytracer
#

CC = clang
CFLAGS = -Wall -Wextra -Iinclude -O3 -mavx -Wmissing-field-initializers -Wpedantic
ANALYZERFLAGS = -g -fanalyzer -lm
SANITIZERFLAGS = -g -fsanitize=address -lm
PERFFLAGS = -g -Iinclude -lm -mavx -ffast-math -O3

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

MATH_SRC = $(SRC_DIR)/vec3.c
MATH_OBJ = $(BUILD_DIR)/vec3.o

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
EXEC = $(BUILD_DIR)/raytrace
all: $(EXEC)

# Link object files into final executable
$(EXEC): $(OBJS) #$(MATH_OBJ)
	$(CC) $(CFLAGS) $(OBJS) -o $@ -lm -lpthread -ldl

# Compile all source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build with gcc's -fanalyzer
analyzer: CFLAGS += $(ANALYZERFLAGS)
analyzer: $(EXEC)

# Build with address sanitizer
sanitizer: CC = clang
sanitizer: CFLAGS += $(SANITIZERFLAGS)
sanitizer: $(EXEC)

# Build and run perf
perf: CFLAGS = $(PERFFLAGS)
perf: $(EXEC)
	perf record ./$(EXEC) > sweep.ppm

clean:
	rm -f $(BUILD_DIR)/*.o $(EXEC) $(BUILD_DIR)/tests

.PHONY: all clean analyzer sanitizer tests
# end
