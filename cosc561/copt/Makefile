CC = gcc

all: copt_O3 copt_O0

copt_O3:
	$(CC) -O3 -g copt.c -o copt_O3
copt_O0: copt.c
	$(CC) -O0 -g copt.c -o copt_O0

clean :
	rm -f copt_O3 copt_O0
