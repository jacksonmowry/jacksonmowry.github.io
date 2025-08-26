##
# scripting
#
# @file
# @version 0.1

all: scripting

run: scripting
	bin/$<

scripting: src/*
	$(CC) --std=c23 -Iinclude $(CFLAGS) $^ -o bin/$@

clean:
	rm bin/*

# end
