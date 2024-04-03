##
# Server Exploration
#
# @file
# @version 0.1

# Define compiler and flags
CC := clang
CFLAGS := -Wall -Wextra -O3 -luring
# CFLAGS := -fsanitize=address -g -luring

# Define source files
SRCS := $(wildcard *.c)

# Define targets (binaries) by replacing .c extension with an empty string
BINS := $(SRCS:.c=)

# Default target
all: $(BINS)

# Rule to build each binary
%: %.c
	$(CC) $(CFLAGS) $< -o $@

# Clean rule
clean:
	rm -f $(BINS)


# end
