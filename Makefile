all: cracker
	make clean

cracker: main.o reverse.o sha256.o
	gcc -Wall -Werror -pthread -o cracker main.o sha256.o reverse.o

main.o: main.c Template/reverse.h
	gcc -Wall -Werror -pthread -c main.c

reverse.o: Template/reverse.c Template/reverse.h
	gcc -Wall -Werror -pthread -c Template/reverse.c

sha256.o: Template/sha256.c Template/sha256.h
	gcc -Wall -Werror -pthread -c Template/sha256.c

clean:
	rm -rf *.o Template/*.o Tests/*.o Fonctions/*.o
