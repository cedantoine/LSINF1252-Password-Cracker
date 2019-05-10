cracker: src/main.o reverse.o sha256.o
	gcc -Wall -g -Werror -pthread -std=c99 -o cracker src/main.o sha256.o reverse.o

# tests: tests.o
# 	gcc -Wall -Werror -o tests tests.o -lpthread -lcunit

main.o: src/main.c src/Template/reverse.h
	gcc -Wall -g -Werror -pthread -std=c99 -c src/main.c

reverse.o: src/Template/reverse.c src/Template/reverse.h
	gcc -Wall -g -Werror -pthread -std=c99 -c src/Template/reverse.c

sha256.o: src/Template/sha256.c src/Template/sha256.h
	gcc -Wall -g -Werror -pthread -std=c99 -c src/Template/sha256.c

# tests.o: ./tests/tests.c
# 	gcc -Wall -Werror -c tests/tests.c -lcunit

clean:
	rm -rf *.o cracker src/Template/*.o tests/*.o src/main.o

all: cracker tests
