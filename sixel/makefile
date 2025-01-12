##
# Sixel
#
# @file
# @version 0.1
all: main color image
	gcc main.o color.o image.o -o sixel

main: main.c
	gcc -c main.c -o main.o

image: image.c
	gcc -c image.c -o image.o

color: color.c
	gcc -c color.c -o color.o

clean:
	rm -f *.o sixel

# end
