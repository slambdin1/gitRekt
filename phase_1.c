//ls -l -a -r -t -h -s > osh_output_test 

#include <stdlib.h>
#include <stdio.h> 
#include <string.h>    
#include <unistd.h>
#include <fcntl.h>

#ifndef DEFS_H
#define DEFS_H

/*
 *
 */
#define OTHER_TOKEN     8

/*
 * Input Modes
 */
#define I_FILE  1
#define I_PIPE  2

/*
 * Output Modes
 */
#define O_APPND        2
#define O_PIPE         3 
#define O_FILE         4

/*
 * Run Next Command Modes
 */
#define JOIN_ANY        1 // ;
#define JOIN_SUCCESS    2 // &&
#define JOIN_FAIL       3 // ||
#define PIPE            4 // |
#define OUT_APPND       5 // >>

/*
 * Redirect Modes
 */
#define IN_REDIRECT     6 // >
#define OUT_REDIRECT    7 // <

/*
 * Parse State 
 */
#define NEED_ANY_TOKEN      0
#define NEED_NEW_COMMAND    1
#define NEED_IN_PATH        2
#define NEED_OUT_PATH       3

 /*
  * Command Combine mode
  */
#define NEXT_ON_ANY     1
#define NEXT_ON_SUCCESS 2
#define NEXT_ON_FAIL    3

extern int cerror;


/* A C sytle linked list to parse and build the argv structure */
typedef struct ArgX{
        char *arg;
        struct ArgX *next;
} Arg;


/* A C sytle linked list to parse the input line */
typedef struct CommandX{
        int  parse_state;
        char *cmd;

        Arg  *arg_list;
        Arg  *last_arg;

        int num_of_args;

        char *input_file;
        char *input_char;
        int  input_mode;
        int  input_fd;

        char *output_file;
        char *output_char;
        int  output_mode;
        int  output_fd;

        int  next_command_exec_on;
        char *next_command_exec_on_char;
        pid_t pid;
        struct CommandX *next;
} Command;

#endif

/*Phase 1 Methods*/
int findType(int state, char token[]);
int findParseState(int previous, int previousType, char token[]);
int isToken(char token[]);
struct CommandX* push(struct CommandX * head, char cmd[]);
void pushArg(struct ArgX* head, char token[]);
void printCommandX(struct CommandX* command);
void pipeProcess(struct CommandX* command);

/*Phase 2 Methods*/
void executeCommand(struct CommandX* command);

int verbose;

int main(int argc, char *argv[])
{
        verbose = 0;
        if(argc == 2){
            verbose = 1;
        }

        char originalCommand[100], *line, *tempToke;
        printf("osh>");

        while(fgets (originalCommand, 100, stdin) != NULL){

                int previousParseState = -1;
                int previousType = -1;

                line = originalCommand;
                line = strsep (&line, "\n");

                struct CommandX *root;
                root = malloc(sizeof(struct CommandX));

                struct CommandX *current;
                current = malloc(sizeof(struct CommandX));
                current = root;

                current->num_of_args = 0;

                //Parse into tokens
                while ((tempToke = strsep(&line, " ")) != NULL ){
                        if(tempToke && tempToke[0] != '\0'){
                                int parseState = findParseState(previousParseState, previousType, tempToke);
                                int type = findType(parseState, tempToke);

                                //update the linked list
                                if(parseState == NEED_NEW_COMMAND){
                                        
                                        if(previousType != IN_REDIRECT){
                                            if(root == NULL){
                                                root = current;
                                            }
                                            current = push(root, tempToke);
                                        }
                                        else{
                                            current->input_file = tempToke;
                                            current->input_mode = I_FILE;
                                            current->input_char = "I_FILE";
                                        }   
                                }
                                else if(parseState == NEED_OUT_PATH){
                                    current->output_file = tempToke;
                                }
                                else if(parseState == NEED_ANY_TOKEN){
                                    if(type == OUT_REDIRECT){
                                        current->output_mode = O_FILE;
                                        current->output_char = "O_FILE";
                                    }
                                    if(type == OUT_APPND){
                                        current->output_mode = O_APPND;
                                        current->output_char = "O_APPND";
                                    }
                                    if(type == PIPE){
                                        current->output_mode = O_PIPE;
                                        current->output_char ="O_PIPE";
                                        current->next_command_exec_on = NEXT_ON_ANY;
                                        current->next_command_exec_on_char ="NEXT_ON_ANY";
                                    }
                                    if(type == JOIN_SUCCESS){
                                        current->next_command_exec_on = NEXT_ON_SUCCESS;
                                        current->next_command_exec_on_char ="NEXT_ON_SUCCESS";
                                    }
                                    if(type == JOIN_FAIL){
                                        current->next_command_exec_on = NEXT_ON_FAIL;
                                        current->next_command_exec_on_char ="NEXT_ON_FAIL";
                                    }
                                    if(type == JOIN_ANY){
                                        current->next_command_exec_on = NEXT_ON_ANY;
                                        current->next_command_exec_on_char ="NEXT_ON_ANY";
                                    }
                                    if(type==OTHER_TOKEN){
                                        if(current->arg_list == NULL){
                                            current->arg_list = malloc(sizeof(struct ArgX));

                                        }
                                        pushArg(current->arg_list, tempToke);
                                        current->num_of_args++;
                                    }
                                }

                                if(previousType == PIPE){
                                    current->input_mode = I_PIPE;
                                    current->input_char="I_PIPE";
                                }

                                previousParseState = parseState;
                                previousType = type;
                        }                       
                };

                //Print list of Commands
                current = root;
                if (current) { /* Makes sure there is a place to start */
                    while ( current->next != 0 ) {
                        if(current->cmd != NULL){
                            printf("------------------------- \n");
                            printCommandX(current);
                        }
                        current = current->next;
                    }
                    printf("------------------------- \n");
                    printCommandX(current);
                }
                /*End of Phase 1*/

                /*Phase 2*/
                // printf("------------------------- \n");
                // current = root;
                // if(current){
                //     while(current->next != 0){
                //         if(current->cmd != NULL){
                             executeCommand(current);
                //         }
                //         current = current->next;
                //     }
                //     executeCommand(current);
                // }  
                
                printf("------------------------- \n");

                printf("osh>");
        };

        return 0;
}

/*Phase 2 Methods*/
void executeCommand(struct CommandX* command){
    
    int pipefd[2];
    char buf;
    pid_t cpid = fork();
    
    if (cpid < 0 ){ 
       fprintf(stderr, "Fork Failed \n");
       exit(1);
    }
    
    else if (cpid == 0 ){ //Code executed only by child process
    
       printf("Child %d Running: %s \n", cpid, command->cmd);
       char* argArray[command->num_of_args+1];

       struct ArgX *current;
       current = malloc(sizeof(struct ArgX));
       current = command->arg_list;
        
       //Create an array from from linked list
       int i = 0;
       argArray[i] = command->cmd;
       if (current) { /* Makes sure there is a place to start */  
           i++;
           while ( current->next != 0 ) {
               if(current->arg != NULL){
                   argArray[i] = current->arg;
               }
               current = current->next;
               i++;
           }
           argArray[i] = current->arg;
       }

        argArray[i+1] = NULL;
        //End of array creation

        int in;
        int out;

        if(command->input_mode == I_FILE)
        {  
            if( access(command->input_file, F_OK) != -1 ) {
                // file exists and replace standard input with input file
                in = open(command->input_file, O_RDONLY);
                dup2(in, 0);
            } else {
                // ERROR: file does not exist 
                printf("Unable to open input file \n");
                _exit(EXIT_FAILURE);
            }
        }
        else if(command->input_mode == I_PIPE)
        {

        }

        if(command->output_mode == O_FILE)
        {
            // replace standard output with output file
            out = open(command->output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            dup2(out, 1);
        }
        else if(command->output_mode == O_APPND)
        {
            out = open(command->output_file, O_WRONLY|O_APPEND);
        }
        else if(command->output_mode == O_PIPE)
        {
            pipeProcess(command);
        }
        
        // close unused file descriptors
        close(in);
        close(out);

        //execute command
        execvp(command->cmd, argArray);

        fprintf(stderr, "Exec Failed \n");
        exit(1);
     }
     else { //Code executed only by parent process

        int status;  
        wait(&status); 
        printf("Parent picked up child %d, status = %d \n", cpid, status);
     }

}
/*End Phase 2 Methods*/

/*Phase 3 Methods*/
void pipeProcess(struct CommandX* command){

    int pipefd[2];
    int pid;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", "Villanova", NULL};

    // make a pipe (fds go in pipefd[0] and pipefd[1])
    pipe(pipefd);

    pid = fork();

    if (pid == 0)
    {
      // child gets here and handles "grep Villanova"
      // replace standard input with input part of pipe
      dup2(pipefd[0], 0);

      // close unused hald of pipe
      close(pipefd[1]);

      // execute grep
      execvp("grep", grep_args);
    }
    else
    {
      // parent gets here and handles
      // replace standard output with output part of pipe
      dup2(pipefd[1], 1);

      // close unused unput half of pipe
      close(pipefd[0]);

      // execute cat

      execvp("cat", cat_args);
    }
}

int findType(int state, char token[]){
        char stateString[20];

        int type = OTHER_TOKEN;
        strcpy(stateString, "OTHER_TOKEN");

        if(state == NEED_ANY_TOKEN){

                if(strcmp(token, "&&") == 0){
                        type = JOIN_SUCCESS;
                        strcpy(stateString, "JOIN_SUCCESS");
                }
                else if(strcmp(token, ";") == 0){
                        type = JOIN_ANY;
                        strcpy(stateString, "JOIN_ANY");
                }
                else if(strcmp(token, "||") == 0){
                        type = JOIN_FAIL;
                        strcpy(stateString, "JOIN_FAIL");
                }
                else if(strcmp(token, "<") == 0){
                        type = IN_REDIRECT;
                        strcpy(stateString, "IN_REDIRECT");
                }
                else if(strcmp(token, ">") == 0){
                        type = OUT_REDIRECT;
                        strcpy(stateString, "OUT_REDIRECT");
                }
                else if(strcmp(token, ">>") == 0){
                        type = OUT_APPND;
                        strcpy(stateString, "OUT_APPND");
                }
                else if(strcmp(token, "|") == 0){
                        type = PIPE;
                        strcpy(stateString, "PIPE");
                }
        }
        
        if(verbose){
            printf("Token:->%s<-Type:%s \n", token, stateString);
        }  
        return type;
}
/*End of Phase 3 Methods*/


/*Phase 1 Methods*/
int findParseState(int previous, int previousType, char token[]){

        int parseState;
        char printString[20];

        if(previous == -1){
                parseState =  NEED_NEW_COMMAND;
                strcpy(printString,"NEED_NEW_COMMAND");
        }
        else if(previous == NEED_NEW_COMMAND){
                parseState =  NEED_ANY_TOKEN;   
                strcpy(printString,"NEED_ANY_TOKEN");
        }
        else if(previous == NEED_OUT_PATH || previous == NEED_IN_PATH){
                if(isToken(token)){
                        parseState =  NEED_ANY_TOKEN;
                        strcpy(printString, "NEED_ANY_TOKEN");
                }
                else{
                        parseState =  NEED_NEW_COMMAND;
                        strcpy(printString,"NEED_NEW_COMMAND");
                }   
        }
        else if(previous == NEED_ANY_TOKEN){
                if((strcmp(token, ">") == 0 && previousType != OTHER_TOKEN) || 
                        (strcmp(token, ">>")==0 && previousType != OTHER_TOKEN) || 
                        previousType == OUT_REDIRECT || 
                        previousType == OUT_APPND){

                        parseState =  NEED_OUT_PATH;
                        strcpy(printString,"NEED_OUT_PATH");
                }
                else if(previousType == JOIN_SUCCESS || previousType == JOIN_ANY){
                        parseState = NEED_NEW_COMMAND;
                        strcpy(printString, "NEED_NEW_COMMAND");
                }
                else if(previousType == OTHER_TOKEN){
                        parseState = NEED_ANY_TOKEN;
                        strcpy(printString, "NEED_ANY_TOKEN");
                }
                else if(previousType == IN_REDIRECT){
                        parseState = NEED_NEW_COMMAND;
                        strcpy(printString, "NEED_NEW_COMMAND");
                }
                else{
                        parseState =  NEED_NEW_COMMAND;
                        strcpy(printString,"NEED_NEW_COMMAND");
                }
        }

        if(verbose){
            printf("Parse state = %s \n", printString);
        }
        
        return parseState;
}

int isToken(char token[]){
        char *tokenList[7] = {">","<",">>","|","&&","||",";"};

        int i;
        for(i = 0; i<7; i++){
                int ret = strcmp(token, tokenList[i]);
                if(ret == 0){
                        return 1;
                }
        }
        return 0;
}

struct CommandX* push(struct CommandX * head, char cmd[]) {
    struct CommandX * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(struct CommandX));
    current->next->cmd = cmd;
    current->next->next = NULL;

    return current->next;
}

void pushArg(struct ArgX* head, char token[]) {
    struct ArgX * current;
    current = malloc(sizeof(struct ArgX));
    current = head;

    while (current->next != NULL && current->next->arg != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(struct ArgX));
    current->next->arg = token;
    current->next->next = NULL;
}


void printCommandX(struct CommandX* command){
        printf("Cmd: %s \n", command->cmd);
        
        //Print ArgList
        struct ArgX *current;
        current = malloc(sizeof(struct ArgX));
        current = command->arg_list;
        
        if (current) { /* Makes sure there is a place to start */
            while ( current->next != 0 ) {
                if(current->arg != NULL){
                    printf("Arg: %s \n", current->arg);
                }
                current = current->next;
            }
            printf("Arg: %s \n", current->arg);
        }

        printf("input file: %s \n", command->input_file);

        if(command->input_char){
            printf("Input mode: %s \n", command->input_char);
        }
        else{
            printf("Input mode: - \n");
        }
        
        printf("output_file: %s \n", command->output_file);

        if(command->output_char){
            printf("Output mode: %s \n", command->output_char);
        }
        else{
            printf("Output mode: - \n");
        }

        if(command->next_command_exec_on_char){
            printf("Command combine mode: %s \n", command->next_command_exec_on_char); 
        }
        else{
            printf("Command combine mode: - \n");
        }
}

/*End of Phase 1 Methods*/
