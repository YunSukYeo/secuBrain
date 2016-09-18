all: firewall nsf-sff-interface
firewall: main nsf-sff-interface
	gcc -o ../../bin/firewall main.o nsf-sff-interface.o

main: main.h ../../Interfaces/nsf-sff-interface.h
	gcc -c main.c
nsf-sff-interface: ../../Interfaces/nsf-sff-interface.h ../../Interfaces/constants.h
	gcc -c ../../Interfaces/nsf-sff-interface.c

clean:
	rm -rf *.o
	rm -rf ../../bin/firewall
