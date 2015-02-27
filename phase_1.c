#include <stdlib.h>
#include <stdio.h> 
#include <string.h>    

#ifndef	DEFS_H
#define DEFS_H


/*
 *
 */
#define OTHER_TOKEN     1

/*
 * Redirect Modes
 */
#define IN_REDIRECT     1 // >
#define OUT_REDIRECT    2 // <

/*
 * Input Modes
 */
#define I_FILE  1
#define I_PIPE  2

/*
 * Output Modes
 */
#define O_WRITE        1 // 
#define O_APPND        2 // >>
#define O_PIPE         3 // |

/*
 * Run Next Command Modes
 */
#define NEXT_ON_ANY        1 // ;
#define NEXT_ON_SUCCESS    2 // &&
#define NEXT_ON_FAIL       3 // ||


/*
 * Parse State 
 */
#define NEED_ANY_TOKEN 0
#define NEED_NEW_COMMAND 1
#define NEED_IN_PATH 2
#define NEED_OUT_PATH 3

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

        char *input_file;
        int  input_mode;
        int  input_fd;

        char *output_file;
        int  output_mode;
        int  output_fd;

        int  next_command_exec_on;
        pid_t pid;
        struct CommandX *next;
} Command;

#endif

int findType(int state);
int findParseState(int current, int previous, char token[]);
int isToken(char token[]);

int verbose;

int main(int argc, char *argv[])
{

        verbose = 0;
        //TODO: Check if there is a -v or -verbose flat

        char originalCommand[100], *line, *tempToke;
        printf("osh>");

        while(fgets (originalCommand, 100, stdin) != NULL){

                struct CommandX *firstCommand;
                
                int previousParseState = -1;

                line = originalCommand;
                line = strsep (&line, "\n");

                //Parse into tokens
                int i = 0;
                struct CommandX *previousCommand;
                struct CommandX *currentCommand = firstCommand;

                while ((tempToke = strsep(&line, " ")) != NULL){
                        
                        int parseState = findParseState(i, previousParseState, tempToke);
                        int type = findType(parseState);

                        if(parseState == NEED_NEW_COMMAND){

                                if(previousCommand->cmd == NULL){
                                        firstCommand = currentCommand;
                                        previousCommand = currentCommand;
                                }
                                else{
                                        previousCommand->next = currentCommand;
                                }
                        }
                        else if(parseState == NEED_ANY_TOKEN){

                                struct ArgX newArg;
                                newArg.arg = tempToke;

                                if(currentCommand->arg_list->arg == NULL){
                                        currentCommand->arg_list = &newArg;
                                }
                                else{
                                        currentCommand->arg_list->next = &newArg;
                                }
                              
                                currentCommand->last_arg = &newArg;
                        }
                        else if(parseState == NEED_IN_PATH){

                        }
                        else if(parseState == NEED_OUT_PATH){

                        }

                        currentCommand->parse_state = parseState;
                        currentCommand->cmd = tempToke;

                        printf("Parse state = %d \n", parseState);
                        printf("Token:->%s <-Type:%d \n", tempToke, type);

                        previousParseState = parseState;
                        i++;
                };

                printf("-------------------------");

                //Print list of commands

                printf("osh>");
        };

   
        return 0;
}

int findType(int state){
        if(state == NEED_NEW_COMMAND){
                return OTHER_TOKEN;
        }
        return 0;
}

int findParseState(int current, int previous, char token[]){
        if(previous == -1){
                return NEED_NEW_COMMAND;
        }
        else if(previous == NEED_NEW_COMMAND){
                return NEED_ANY_TOKEN;   
        }
        else if(previous == NEED_OUT_PATH || previous == NEED_IN_PATH){
                if(isToken(token)){
                        return NEED_ANY_TOKEN;
                }
                else{
                        return NEED_NEW_COMMAND;
                }   
        }
        else if(previous == NEED_ANY_TOKEN){
                if(strcmp(token, "<") ==0 || strcmp(token, "<<")==0){
                        return NEED_IN_PATH;
                }
                else if(strcmp(token, ">") == 0 || strcmp(token, ">>")){
                        return NEED_OUT_PATH;
                }
                else{
                        return NEED_NEW_COMMAND;
                }
        }

}

int isToken(char token[]){
        char *tokenList[7] = {">","<",">>","|","&&","||",";"};

        for(int i = 0; i<7; i++){
                int ret = strcmp(token, tokenList[i]);
                if(ret == 0){
                        return 1;
                }
        }
        return 0;
}
