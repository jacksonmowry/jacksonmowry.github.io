# Cats
# C Auto Template System
# @file
# @version 0.1

buffer_string.h: buffer.h
	xxd -i $< > $@

# Debug build
cats_debug: CFLAGS += -fsanitize=address -g
cats_debug: macro_generation.c buffer_string.h
	clang $(CFLAGS) $< -o $@

# Optimized build
cats_optimized: CFLAGS += -O3
cats_optimized: macro_generation.c buffer_string.h
	clang $(CFLAGS) $< -o cats

.PHONY: all debug optimized
all: cats_debug

debug: cats_debug

cats: cats_optimized

run_debug: cats_debug
	./cats_debug index.html

run: cats_optimized
	./cats index.html

clean:
	rm -f buffer_string.h cats_debug cats_optimized

# end
