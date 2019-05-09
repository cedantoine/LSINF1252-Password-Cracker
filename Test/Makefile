all: test

testq: test.o ../main.o
	gcc -Wall -Werror -lcunit -o -lpthread

testq.o: test.c
	gcc -Wall -Werror -c testFrac.c
