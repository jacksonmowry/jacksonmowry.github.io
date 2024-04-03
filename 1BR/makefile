# 1BR
#
# @file
# @version 0.1

CC = clang
# CFLAGS = -O3
CFLAGS = -g -fno-omit-frame-pointer
TARGET = 1BR
SRCS = indiv_map.c hashmap/hashmap.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

# end
