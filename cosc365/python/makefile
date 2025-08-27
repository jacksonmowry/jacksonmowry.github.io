CFLAGs=-Wall -I/usr/include/python3.13 -fPIC -shared
LIBS=-lpython3.13

all: hello.so

hello.so: hello.c makefile
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f hello.so

.PHONY: clean
