#include <stdlib.h>
#include <stdio.h> 
#include <string.h>    

#ifndef DEFS_H
#define DEFS_H


/*
 *
 */
#define OTHER_TOKEN     1

/*
 * Input Modes
 */
#define I_FILE  1
#define I_PIPE  2

/*
 * Output Modes
 */
#define O_WRITE        1 // 

/*
 * Run Next Command Modes
 */
#define JOIN_ANY        1 // ;
#define JOIN_SUCCESS    2 // &&
#define JOIN_FAIL       3 // ||
#define PIPE            4 // |
#define OUT_APPND         5 // >>

/*
 * Redirect Modes
 */
#define IN_REDIRECT     6 // >
#define OUT_REDIRECT    7 // <

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

int findType(int state, char token[]);
int findParseState(int previous, int previousType, char token[]);
int isToken(char token[]);
struct CommandX* push(struct CommandX * head, char cmd[]);
void printCommandX(struct CommandX* command);

int verbose;

int main(int argc, char *argv[])
{

        verbose = 0;
        //TODO: Check if there is a -v or -verbose flat

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

                //Parse into tokens
                while ((tempToke = strsep(&line, " ")) != NULL ){
                        if(tempToke && tempToke[0] != '\0'){
                                int parseState = findParseState(previousParseState, previousType, tempToke);
                                int type = findType(parseState, tempToke);
                                previousParseState = parseState;
                                previousType = type;

                                //update the linked list
                                if(previousParseState == NEED_NEW_COMMAND){
                                        printf("IT IS A NEW COMMAND \n");
                                        if(root == NULL){
                                                root = current;
                                        }
                                        current = push(root, tempToke);

                                        printf("END OF \n");
                                }
                                else{
                                        if(type == OUT_REDIRECT){
                                                current->output_mode = OUT_REDIRECT;
                                                current->output_file = tempToke;
                                        }
                                }
                        }                       
                };

                printf("------------------------- \n");

                //Print list of Commands
                current = root;
                if (current) { /* Makes sure there is a place to start */
                    while ( current->next != 0 ) {
                        
                        printCommandX(current);
                        current = current->next;
                    }
                    printf("------------------------- \n");
                    printCommandX(current);
                }

                printf("------------------------- \n");
                printf("osh>");
        };

        return 0;
}

int findType(int state, char token[]){
        char stateString[20];

        int type = OTHER_TOKEN;
        strcpy(stateString, "OTHER_TOKEN");

        if(state == NEED_ANY_TOKEN){

                if(strcmp(token, "&&") == 0){
                        printf("join success \n");
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

        printf("Token:->%s<-Type:%s \n", token, stateString);
        return type;
}

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
                if(strcmp(token, ">") == 0 || 
                        strcmp(token, ">>")==0 || 
                        previousType == OUT_REDIRECT || 
                        previousType == OUT_APPND){

                        parseState =  NEED_OUT_PATH;
                        strcpy(printString,"NEED_OUT_PATH");
                }
                else if(previousType == OTHER_TOKEN){
                        parseState = NEED_ANY_TOKEN;
                        strcpy(printString, "NEED_ANY_TOKEN");
                }
                else if(previousType == JOIN_SUCCESS){
                        parseState = NEED_NEW_COMMAND;
                        strcpy(printString, "NEED_NEW_COMMAND");
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

        printf("Parse state = %s \n", printString);
        return parseState;
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

void printCommandX(struct CommandX* command){
        printf("Cmd: %s \n", command->cmd);
        // if(command->arg != NULL){
        // }
        printf("input file: %s \n", command->input_file);
        printf("Input mode: %s \n", command->input_mode);
        printf("output_file: %s \n", command->output_file);
        printf("Output mode: %s \n", command->output_mode);
        printf("Command combine mode: %s \n", command->next_command_exec_on);
}
