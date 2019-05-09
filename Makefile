all: cracker
	make clean

cracker: main.o reverse.o sha256.o
	gcc -Wall -g -Werror -pthread -std=c99 -o cracker main.o sha256.o reverse.o

main.o: main.c Template/reverse.h
	gcc -Wall -g -Werror -pthread -std=c99 -c main.c

reverse.o: Template/reverse.c Template/reverse.h
	gcc -Wall -g -Werror -pthread -std=c99 -c Template/reverse.c

sha256.o: Template/sha256.c Template/sha256.h
	gcc -Wall -g -Werror -pthread -std=c99 -c Template/sha256.c

test:
	cd Test && make
	make clean

clean:
	rm -rf *.o Template/*.o Test/*.o Fonctions/*.o
