/*
 * wytalkC.c
 * Author: Beckham Carver
 * Date: April 25, 2023
 *
 * COSC 3750, Homework 9
 *
 * Shell program that that creates a cmdLink structure
 * and adds arguments to linked-list contained in structure, 
 * command structures are then also linked together.
 * Program parses these into seperate arrays for execution at end.
 */

#include"wyscanner.h"
#include"wyargs.h"

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>


typedef struct word Word;
typedef struct cmdLink CmdLink;

struct word {
    struct word *next, *prev;
    char* data;
};

struct cmdLink {
    Word *cmd_argv;
    int cmd_argc;

    // set depending on pipe
    int input, output, error;
    char *infile, *outfile, *errfile;

    struct cmdLink *prev, *next;
};

void execute(char **argv);


int main()
{
    char *tokens[]={ "QUOTE_ERROR", "ERROR_CHAR", "SYSTEM_ERROR",
        "EOL", "REDIR_OUT", "REDIR_IN", "APPEND_OUT",
        "REDIR_ERR", "APPEND_ERR", "REDIR_ERR_OUT", "SEMICOLON",
        "PIPE", "AMP", "WORD" };

    int rtn;
    char *rpt;
    char buf[1024];

    int current = 0;
    int isFile = 0;
    int runError = 0;


    int inputDir = 0;
    int errorDir = 0;
    int outputDir = 0;
    int errOut = 0;

    while(1) {
        printf("&> ");
        rpt=fgets(buf,256,stdin);

        if(rpt == NULL) {
            if(feof(stdin)) {
                return 0;
            }
            else {
                printf("fgets from stdin\n");
                return 1;
            }
        }
        CmdLink* currentCmd = (CmdLink*)malloc(sizeof(CmdLink));
        current = 0;
        isFile = 0;
        runError = 0;
        
        // !!! these can and will be replaced by checks inside
        // !!! the currentCmd data structure in the future
        inputDir = 0;
        errorDir = 0;
        outputDir = 0;
        errOut = 0;
        
        rtn=parse_line(buf);
        while(rtn !=  EOL && !runError){ 
            switch(rtn) {

                case WORD:
                    if (current == 0) {
                        // requires that current is moved to next by pipe/semicolon
                        currentCmd = appendCmdLink(currentCmd,lexeme);
                        current = 1;
                        break;
                    }
                    if (isFile == 1) {
                        // update in CmdLink
                        isFile = 0;
                    }
                    // new link not needed, append argv
                    appendWord(currentCmd->cmd_argv, lexeme);
                    currentCmd->cmd_argc++;
                    break;

                case REDIR_OUT:
                    if (outputDir == 1){ 
                        printf("error > : too many output redirects\n");
                        runError = 1;
                        break;
                    }
                    if (current == 0) {
                        printf("error > : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (isFile == 1) {
                        printf("error > : file not given\n");
                        runError = 1;
                        break;
                    }
                    outputDir = 1;
                    break;

                case REDIR_IN:
                    if (inputDir == 1){ 
                        printf("error < : too many input redirects\n");
                        runError = 1;
                        break;
                    }
                    if (current == 0) {
                        printf("error < : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (isFile == 1) {
                        printf("error < : file not given\n");
                        runError = 1;
                        break;
                    }
                    inputDir = 1;
                    break;

                case REDIR_ERR:
                    if (errorDir == 1){ 
                        printf("error 2> : too many error redirects\n");
                        runError = 1;
                        break;
                    }
                    if (current == 0) {
                        printf("error 2> : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (isFile == 1) {
                        printf("error 2> : file not given\n");
                        runError = 1;
                        break;
                    }
                    errorDir = 1;

                case REDIR_ERR_OUT:
                    if (errOut == 1){ 
                        printf("error 2>&1 : error already redirected\n");
                        runError = 1;
                        break;
                    }
                    if (current == 0) {
                        printf("error 2>&1 : no command present\n");
                        runError = 1;
                        break;
                    }
                    errOut = 1;
                    break;

                case SEMICOLON:
                    if (current == 0) {
                        printf("error ';' : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (isFile && (outputDir | inputDir | errorDir)) {
                        printf("error ';' : no file given for redirect");
                        runError = 1;
                        break;
                    }

                    // these flags will be made unecessary in next HW
                    inputDir = 0;
                    outputDir = 0;
                    errorDir = 0;
                    errOut = 0;

                    current = 0;
                    break;

                case AMP:
                    if (isFile) {
                        printf("error & : file not given\n");
                        runError = 1;
                        break;
                    }
                    if (current == 0) {
                        printf("error & : no command present\n");
                        runError = 1;
                        break;
                    }
                    break;

                case PIPE:
                    if (isFile) {
                        printf("error | : file not given prior\n");
                        runError = 1;
                        break;
                    }
                    if (current == 0) {
                        printf("error | : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (outputDir == 1) {
                        printf("error | : output already redirected\n");
                        runError = 1;
                        break;
                    }

                    inputDir = 1;
                    current = 0;
                    break;

                case APPEND_OUT:
                    if (isFile) {
                        printf("error >> : file not given\n");
                        runError = 1;
                        break;
                    }
                    if (current == 0) {
                        printf("error >> : no command present\n");
                        runError = 1;
                        break;
                    }             
                    break;
                case APPEND_ERR:
                    if (isFile) {
                        printf("error << : file not given\n");
                        runError = 1;
                        break;
                    }
                    if (current == 0) {
                        printf("error << : no command present\n");
                        runError = 1;
                        break;
                    }
                    break;

                case QUOTE_ERROR:
                    printf("error : quote error\n");
                    runError = 1;
                    break;

                case SYSTEM_ERROR:
                    perror("error : system error");
                    return 1;

                case ERROR_CHAR:
                    printf("error char: %d",error_char);
                    runError = 1;
                    break;

                default:
                    printf("error num %d : '%s'\n",rtn,tokens[rtn%96]);
                    runError = 1;
                    break;
            }
            rtn=parse_line(NULL);
        }
        if (rtn == EOL && !runError){
            if (isFile) {printf("error EOL : file name not given prior\n");}
            else {
                while(currentCmd->prev != NULL) {
                        currentCmd = currentCmd->prev;
                }
                while(currentCmd != NULL) {
                    
                    char** args = (char**)malloc((currentCmd->cmd_argc + 1) * sizeof(char*));
                    if (args == NULL) {
                        printf("allocation failed\n");
                        return -1;
                    }

                    Word* tmp = currentCmd->cmd_argv;
                    for(int i = 0; i <currentCmd->cmd_argc; i++) {
                        args[i] = tmp->data;
                        tmp = tmp->next;
                    }
                    args[currentCmd->cmd_argc] = NULL;

                    currentCmd = currentCmd->next;
                    execute(args);
                }
            }
        }
        printf("\n");
        freeCmdLinks(currentCmd);
    }
}


void execute(char** args){
    int stat;
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        if (execvp(*args, args) < 0) {
            printf("%s: command not found\n",args[0]);
            exit(1);
        }
        exit(0);
    }
    if (pid > 0) {
        wait(&stat);
    }
}
