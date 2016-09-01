sff: main.o nsf-sff-interface.o
	gcc -o sff main.o nsf-sff-interface.o

main.o: main.h constants.h nsf-sff-interface.h main.c
	gcc -c main.c

nsf-sff-interface.o: nsf-sff-interface.h constants.h
	gcc -c nsf-sff-interface.c

clean:
	rm -rf *.o
	rm -rf sff