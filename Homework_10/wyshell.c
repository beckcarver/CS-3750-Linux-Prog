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

    // set depending on pipe, pipe = 3
    int input, output, error, amp;
    char *infile, *outfile, *errfile;

    struct cmdLink *prev, *next;
};

void execute(char **argv);
void executeNow(char **argv);

int main()
{
    char *tokens[]={ "QUOTE_ERROR", "ERROR_CHAR", "SYSTEM_ERROR",
        "EOL", "REDIR_OUT", "REDIR_IN", "APPEND_OUT",
        "REDIR_ERR", "APPEND_ERR", "REDIR_ERR_OUT", "SEMICOLON",
        "PIPE", "AMP", "WORD" };

    int rtn;
    char *rpt;
    char buf[1024];

    int needFile = 0;
    int runError = 0;

    int inputDir = 0;
    int errorDir = 0;
    int outputDir = 0;
    int appendCond = 0;

    CmdLink* head;
    int status;

    while(1) {
        printf("$> ");
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

        CmdLink* currentCmd = createEmptyCmdLink();
        head = currentCmd;

        needFile = 0;
        runError = 0;
        
        // accidentally deleted this line with CTRL+Z while
        // messing with forks() and then spent THREE BLOODY HOURS
        // trying to understand why my forks were wrong. They weren't.
        appendCond = 0;
        
        inputDir = 0;
        errorDir = 0;
        outputDir = 0;

        

        rtn=parse_line(buf);

        while(rtn !=  EOL && !runError){ 
            //waitpid(-1, &status,WNOHANG)
            // "convenient" time to wait for children


            switch(rtn) {

                case WORD:
                    if (needFile) {
                        // update in CmdLink
                        if (outputDir == 1) {
                            currentCmd->outfile = strdup(lexeme);
                        } else if (inputDir == 0) {
                            currentCmd->infile = strdup(lexeme);
                        } else if (errorDir == 0) {
                            currentCmd->errfile = strdup(lexeme);
                        }
                        needFile = 0;
                        currentCmd->amp = 0;
                        break;
                    }
                    if (appendCond) {
                        currentCmd = appendCmdLink(currentCmd);
                        currentCmd->amp = 0;
                        // 1 for semicolon, 3 for pipe
                        currentCmd->input = appendCond;
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
                    if (currentCmd->cmd_argc == 0) {
                        printf("error > : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (needFile == 1) {
                        printf("error > : file not given\n");
                        runError = 1;
                        break;
                    }
                    needFile = 1;
                    outputDir = 1;
                    currentCmd->amp = 0;
                    break;

                case REDIR_IN:
                    if (inputDir == 1 || currentCmd->input == 3){ 
                        printf("error < : too many input redirects\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc == 0) {
                        printf("error < : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (needFile == 1) {
                        printf("error < : file not given\n");
                        runError = 1;
                        break;
                    }
                    needFile = 1;
                    inputDir = 1;
                    currentCmd->amp = 0;
                    break;

                case REDIR_ERR:
                    if (errorDir == 1){ 
                        printf("error 2> : too many error redirects\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc == 0) {
                        printf("error 2> : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (needFile == 1) {
                        printf("error 2> : file not given\n");
                        runError = 1;
                        break;
                    }
                    needFile = 1;
                    errorDir = 1;
                    currentCmd->amp = 0;
                    break;

                case REDIR_ERR_OUT:
                    if (errorDir == 1){ 
                        printf("error 2>&1 : error already redirected\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc == 0) {
                        printf("error 2>&1 : no command present\n");
                        runError = 1;
                        break;
                    }
                    currentCmd->error = currentCmd->output;
                    errorDir = 1;
                    currentCmd->amp = 0;
                    break;

                case SEMICOLON:
                    if (needFile && (outputDir | inputDir | errorDir)) {
                        printf("error ';' : no file given for redirect");
                        runError = 1;
                        break;
                    }
                    inputDir = 0;
                    outputDir = 0;
                    errorDir = 0;
                    appendCond = 1;
                    break;

                case AMP:
                    if (needFile) {
                        printf("error & : file not given\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc == 0) {
                        printf("error & : no command present\n");
                        runError = 1;
                        break;
                    }
                    currentCmd->amp = 1;
                    break;

                case PIPE:
                    if (needFile) {
                        printf("error | : file not given prior\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc == 0) {
                        printf("error | : no command present\n");
                        runError = 1;
                        break;
                    }
                    if (outputDir == 1) {
                        printf("error | : output already redirected\n");
                        runError = 1;
                        break;
                    }
                    currentCmd->output = 3;
                    currentCmd->amp = 0;
                    appendCond = 3;
                    break;

                case APPEND_OUT:
                    if (needFile) {
                        printf("error >> : file not given\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc== 0) {
                        printf("error >> : no command present\n");
                        runError = 1;
                        break;
                    }
                    needFile = 1;
                    currentCmd->amp = 0;             
                    break;
                    
                case APPEND_ERR:
                    if (needFile) {
                        printf("error << : file not given\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc== 0) {
                        printf("error << : no command present\n");
                        runError = 1;
                        break;
                    }
                    needFile = 1;
                    currentCmd->amp = 0;
                    break;

                case QUOTE_ERROR:
                    printf("error : quote error\n");
                    runError = 1;
                    break;

                case SYSTEM_ERROR:
                    perror("error : system error");
                    return 1;

                case ERROR_CHAR:
                    printf("error char: %d\n",error_char);
                    runError = 1;
                    break;

                default:
                    printf("error num %d : '%s'\n",rtn,tokens[rtn%96]);
                    runError = 1;
                    break;
            }
            rtn=parse_line(NULL);
        }

        while(wait(NULL) > 0);

        if (rtn == EOL && !runError){
            if (needFile) {
                printf("error EOL : file name not given prior\n");
            }
            else if (head->cmd_argc == 0) {}
            else {
                // Last cmd
                int ampAll = currentCmd->amp;
                // first command
                currentCmd = head;
                printf("amp is %d\n", currentCmd->amp);
                while(currentCmd != NULL) {

                    // Allocate args array
                    printf("----NEW COMMAND (");
                    char** args = (char**)calloc((currentCmd->cmd_argc + 1), sizeof(char*));
                    if (args == NULL) {
                        printf("allocation failed\n");
                        return -1;
                    }

                    // Fill args array
                    Word* tmp = currentCmd->cmd_argv;
                    for(int i = 0; i <currentCmd->cmd_argc; i++) {
                        if(tmp->data != NULL) {
                            args[i] = tmp->data;
                            tmp = tmp->next;
                        } else {
                            i = currentCmd->cmd_argc;                            
                        }
                    }
                    args[currentCmd->cmd_argc] = '\0';

                    // Debug
                    printf("%d) ", currentCmd->cmd_argc);
                    for(int i = 0; i <= currentCmd->cmd_argc; i++) {
                        printf("%s :: ", args[i]);
                    }

                    // Execute args array
                    printf("\n");
                    if(args[0] != NULL) {
                        if (currentCmd->amp || ampAll) {
                            printf("-------executeNow\n");
                            executeNow(args);
                        } else {
                            printf("-------execute\n");
                            execute(args);
                        }
                    currentCmd = currentCmd->next;
                    } 
                }
                
            }
        }
        freeCmdLinks(head);
    }
}


void execute(char** argv){
    int status;
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        if (execvp(argv[0], argv) < 0) {
            printf("%s: command not found\n",argv[0]);
            exit(1);
        }
        exit(0);
    }
    if (pid > 0) {
        wait(&status);
    }
}

void executeNow(char** argv){

    pid_t pid;
    pid = fork();
    if(pid == 0){
        if(execvp(argv[0], argv) < 0){
            printf("%s: command not found\n", argv[0]);
            exit(1);
        }
        exit(0);
    }
}
