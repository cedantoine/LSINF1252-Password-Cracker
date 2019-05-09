all: test

test: test.o ../main.o
	gcc -Wall -Werror -lcunit -o -L$(HOME)/local/lib -lpthread

test.o: test.c
	gcc -Wall -Werror -c testFrac.c -I$(HOME)/local/include
