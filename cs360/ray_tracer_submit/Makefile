##
#raytracer
#

CC = clang
CFLAGS = -Wall -Wextra -Iinclude -O3 -mavx -Wmissing-field-initializers -Wpedantic
ANALYZERFLAGS = -g -fanalyzer -lm
SANITIZERFLAGS = -g -fsanitize=address -lm
PERFFLAGS = -g -Iinclude -lm -mavx -O3

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

MATH_SRC = $(SRC_DIR)/vec3.c
MATH_OBJ = $(BUILD_DIR)/vec3.o

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
EXEC = $(BUILD_DIR)/raytrace
all: $(EXEC) ppm_plugin bmp_plugin

# Link object files into final executable
$(EXEC): $(OBJS) #$(MATH_OBJ)
	$(CC) $(CFLAGS) $(OBJS) -o $@ -lm -lpthread -ldl

# Compile all source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build with gcc's -fanalyzer
analyzer: CC = gcc
analyzer: CFLAGS += $(ANALYZERFLAGS)
analyzer: $(EXEC)

# Build with address sanitizer
sanitizer: CFLAGS += $(SANITIZERFLAGS)
sanitizer: $(EXEC)

# Build and run perf
perf: CFLAGS = $(PERFFLAGS)
perf: $(EXEC) ppm_plugin bmp_plugin
	cd build && perf record ./raytrace -t 12 -d ppm > sweep.ppm

ppm_plugin: plugins/ppm.c src/interval.c src/vec3.c
	$(CC) -fPIC -shared plugins/ppm.c -o $(BUILD_DIR)/libppm.so src/interval.c src/vec3.c -Iinclude

bmp_plugin: plugins/bmp.c src/interval.c src/vec3.c
	$(CC) -fPIC -shared plugins/bmp.c -o $(BUILD_DIR)/libbmp.so src/interval.c src/vec3.c -Iinclude

clean:
	rm -f $(BUILD_DIR)/*.o $(EXEC) $(BUILD_DIR)/tests $(BUILD_DIR)/*.so

.PHONY: all clean analyzer sanitizer tests
# end
