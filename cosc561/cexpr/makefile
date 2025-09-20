CC      := gcc
CFLAGS  := -Wall -Wextra -Iinclude -g -fPIE
LIBS    := -Llib -lscan

SRC       := src/cexpr.c
OBJDIR    := objs
CEXPR_OBJ := $(OBJDIR)/cexpr.o
TARGET    := cexpr

all: $(TARGET)

$(TARGET): $(CEXPR_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(CEXPR_OBJ) $(LIBS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: all clean

