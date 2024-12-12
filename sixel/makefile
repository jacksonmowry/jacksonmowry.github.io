##
# Sixel
#
# @file
# @version 0.1
all: main color
	gcc main.o color.o -o sixel

main: main.c
	gcc -c main.c -o main.o

color: color.c
	gcc -c color.c -o color.o

clean:
	rm *.o sixel

# end
