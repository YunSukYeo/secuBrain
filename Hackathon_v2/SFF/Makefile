all: sff

sff: main.o nsf-sff-interface.o
	gcc -o ../bin/sff main.o nsf-sff-interface.o

main.o: main.h ../Interfaces/nsf-sff-interface.h 
	gcc -c main.c

nsf-sff-interface.o: ../Interfaces/nsf-sff-interface.h ../Interfaces/constants.h
	gcc -c ../Interfaces/nsf-sff-interface.c

clean:
	rm -rf *.o
	rm -rf ../bin/sff 
