OS Principles: HW 2 README
By: Shannen Lambdin

I did not encounter any problems while writing this program. Therefore, error catching has been implemented should the user enter an erroneous command, and the program will terminate when the string 'exit' is received as a command. Included in my homework submission is a make file that executes the below commands. 

	all: phase_1.c
		gcc -g -Wall -o osh phase_1.c

	clean:
		$(RM) osh

This should create a osh file from the phase_1.c program that will then allow you to run my program.

While writing this program, I learned how file descriptors acted as reference points for interacting with pipes and how to control the low level calls that would allow us to open a non-blocking file descriptor. The non-blocking flag was crucial because it allowed us to read from one end even if no one has written from the other end. This would in turn allow asynchronous reading of the open file descriptors for the current running process. Furthermore, I learned how to monitor multiple file descriptors with the select() call and test to see which file descriptor was ready for certain I/O processes using the FD_ISSET() method.