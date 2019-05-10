all: test

testq: test.o ../main.o
	gcc -Wall -Werror -o -lpthread -lcunit

testq.o: test.c
	gcc -Wall -Werror -c test.c -lcunit
