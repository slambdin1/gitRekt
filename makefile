# build an executable named osh from phase_1.c
all: phase_1.c
	gcc -g -Wall -o osh phase_1.c

clean:
	$(RM) osh