tests: tests.o
	gcc -Wall -Werror -o tests tests.o -lpthread -lcunit

tests.o: tests.c
	gcc -Wall -Werror -c tests.c -lcunit

all: tests
