OS Principles: HW 2 README
By: Shannen Lambdin

Below is a list of problems I ran into while writing this program. The program will terminate when the string 'exit' is received as a command. 
1) Memory leaks
2) Erroneous command checking (phase 1)

Included in my homework submission is a make file that executes the below commands. 

	all: phase_1.c
		gcc -g -Wall -o osh phase_1.c

	clean:
		$(RM) osh

This should create a osh file from the phase_1.c program that will then allow you to run my program.


While writing this program, I learned about the importance of malloc() and free(), and the impact impropriate use of them can lead to read and other potential errors in code execution (for example accessing already freed memory). I also started using valgrind on the cse server to help minimize some memory leaks so for future programs I will be able to successfully managing such leaks. I also learned how to properly manage pipes while recursively iterating through commands, so using the dup() method and the necessity of closing all pipes to avoid hanging pipes. This program was also a good refresher for linked lists and general C programming practices. 
