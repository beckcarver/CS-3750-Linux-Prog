/*
 * wytalkC.c
 * Author: Beckham Carver
 * Last Modified Date: May 2, 2023
 *
 * COSC 3750, Homework 10
 *
 * Shell program that that creates a cmdLink structure
 * and adds arguments to linked-list contained in structure, 
 * the structure is then pieced into an argv and passed
 * to fork()/exec..()/pipe() as needed by syntax.
 */

#include "wyscanner.h"
#include "wyargs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define WRITE_END 1
#define READ_END 0

typedef struct word Word;
typedef struct cmdLink CmdLink;

struct word {
    struct word *next, *prev;
    char* data;
};

struct cmdLink {
    Word *cmd_argv;
    int cmd_argc;

    // std-in = 0
    // std-out = 1
    // std-err = 2
    // pipe = 3
    // self file = 4
    // append file = 5
    // err->outfile = 6

    // default: 0, 1, 2, 0
    int input, output, error, amp;
    char *infile, *outfile, *errfile;

    struct cmdLink *prev, *next;
};

void execute(CmdLink* currentCmd, int amp);
void executePipe(CmdLink* currentCmd, int amp);
void redirect(CmdLink* currentCmd);
char** argumentGenerator(CmdLink* currentCmd);

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
    int appendCond = 0;

    CmdLink* head;

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

        // accidentally deleted this line with CTRL+Z while
        // adding a while(wait()) and then spent THREE BLOODY HOURS
        // trying to understand why my execs() wouldn't exit.
        // ...they were fine.
        appendCond = 0;
        
        needFile = 0;
        runError = 0;

        rtn=parse_line(buf);
        while(rtn !=  EOL && !runError){ 

            switch(rtn) {

                case WORD:
                    if (appendCond) {
                        // if pipe, start pipe
                        if (appendCond == 3) {currentCmd->output = 3;}
                        // make next command
                        currentCmd = appendCmdLink(currentCmd);
                        currentCmd->amp = 0; 
                        // set input if pipe/semicolon
                        currentCmd->input = appendCond;
                        appendWord(currentCmd->cmd_argv, lexeme);
                        currentCmd->cmd_argc++;
                        appendCond = 0;
                        break;
                    }
                    if (needFile) {
                        // update in CmdLink
                        if (currentCmd->input != 0) {
                            currentCmd->infile = strdup(lexeme);
                        } 
                        if (currentCmd->output != 1) {
                            currentCmd->outfile = strdup(lexeme);
                        } 
                        if (currentCmd->error != 2) {
                            currentCmd->errfile = strdup(lexeme);
                        }
                        needFile = 0;
                        currentCmd->amp = 0;
                        break;
                    }
                    // new link not needed, append argv
                    appendWord(currentCmd->cmd_argv, lexeme);
                    currentCmd->cmd_argc++;
                    break;

                case REDIR_OUT:
                    if (currentCmd->output != 1){ 
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
                    currentCmd->output = 4;
                    currentCmd->amp = 0;
                    break;

                case REDIR_IN:
                    if (currentCmd->input != 1){ 
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
                    currentCmd->input = 4;
                    currentCmd->amp = 0;
                    break;

                case REDIR_ERR:
                    if (currentCmd->error != 2){ 
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
                    currentCmd->error = 4;
                    currentCmd->amp = 0;
                    break;

                case REDIR_ERR_OUT:
                    if (currentCmd->error != 2){ 
                        printf("error 2> : too many error redirects\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc == 0) {
                        printf("error 2>&1 : no command present\n");
                        runError = 1;
                        break;
                    }
                    // set to error to output-file
                    if (currentCmd->output != 1) {
                        currentCmd->error = 6;
                        break;
                    }
                    // set output to std-out
                    currentCmd->error = 2;
                    currentCmd->amp = 0;
                    break;

                case SEMICOLON:
                    if (needFile) {
                        printf("error ';' : no file given for redirect");
                        runError = 1;
                        break;
                    }
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
                    if (currentCmd->output != 1) {
                        printf("error | : output already redirected\n");
                        runError = 1;
                        break;
                    }
                    currentCmd->amp = 0;
                    appendCond = 3;
                    break;

                case APPEND_OUT:
                    if (currentCmd->output != 1){ 
                        printf("error >> : too many output redirects\n");
                        runError = 1;
                        break;
                    }
                    if (needFile) {
                        printf("error >> : file not given\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc == 0) {
                        printf("error >> : no command present\n");
                        runError = 1;
                        break;
                    }
                    needFile = 1;
                    currentCmd->output = 5;
                    currentCmd->amp = 0;             
                    break;
                    
                case APPEND_ERR:
                    if (currentCmd->error != 2){ 
                        printf("error 2>> : too many error redirects\n");
                        runError = 1;
                        break;
                    }
                    if (needFile) {
                        printf("error 2>> : file not given\n");
                        runError = 1;
                        break;
                    }
                    if (currentCmd->cmd_argc == 0) {
                        printf("error 2>> : no command present\n");
                        runError = 1;
                        break;
                    }
                    needFile = 1;
                    currentCmd->amp = 0;
                    currentCmd->error = 5;
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
            else if (head->cmd_argc > 0){

                int ampAll = currentCmd->amp;
                currentCmd = head;

                while(currentCmd != NULL) {

                    // Execute args array
                    if(currentCmd->cmd_argc > 0) {
                        int amp = (currentCmd->amp || ampAll) ? 1 : 0;
                        if (currentCmd->output == 3) {
                            executePipe(currentCmd, amp);
                            // skip ahead 1 extra
                            currentCmd = currentCmd->next;
                        } else {
                            execute(currentCmd, amp);
                        }
                    for (int i = STDERR_FILENO + 1; i < 256; i++) close(i);
                    currentCmd = currentCmd->next;
                    } 
                }    
            }
        }
        freeCmdLinks(head);
        
    }
}


void execute(CmdLink* currentCmd, int amp){

    printf("input file: %s\n", currentCmd->infile);
    printf("output file: %s\n", currentCmd->outfile);
    printf("error file: %s\n", currentCmd->errfile);
    
    pid_t pid;
    pid = fork();

    char** argv = argumentGenerator(currentCmd);
    if (pid == 0) {
        redirect(currentCmd);
        if (execvp(argv[0], argv) < 0) {
            printf("%s: command not found\n",argv[0]);
            exit(1);
        }
        exit(0);
    }
    if (!amp) {
        wait(NULL);
    }
}


void executePipe(CmdLink* currentCmd, int amp){

    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();

    
    

    char** argv = argumentGenerator(currentCmd);
    char** argv2 = argumentGenerator(currentCmd->next);
    
    

    if(pid==0)
    {
        redirect(currentCmd);

        dup2(fd[WRITE_END], STDOUT_FILENO);
        close(fd[READ_END]);
        close(fd[WRITE_END]);
        execvp(argv[0], argv);
        printf("%s: command not found\n", argv[0]);
        exit(1);
    }
    else
    { 
        pid=fork();

        if(pid==0)
        {
            redirect(currentCmd->next);

            dup2(fd[READ_END], STDIN_FILENO);
            close(fd[WRITE_END]);
            close(fd[READ_END]);
            execvp(argv2[0], argv2);
            printf("%s: command not found\n", argv2[0]);
            exit(1);
        }
        else
        {
            int status;
            close(fd[READ_END]);
            close(fd[WRITE_END]);
            if (!amp) {
                waitpid(pid, &status, 0);
            }
        }
    }
}


// handle file all redirects EXCEPT pipe
void redirect(CmdLink* currentCmd) {
    int file;
    
    if (currentCmd->input == 4) {
        //printf("input file: %s\n", currentCmd->infile);
        file = open(currentCmd->infile, O_RDONLY|O_TRUNC, 0666);
        dup2(file, 0);
    }
    if (currentCmd->output == 4) {
        //printf("output file: %s\n", currentCmd->outfile);
        file = open(currentCmd->outfile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        dup2(file, 1);
    }
    if (currentCmd->output == 5) {
        //printf("output append file: %s\n", currentCmd->outfile);
        file = open(currentCmd->outfile, O_WRONLY|O_CREAT|O_APPEND, 0666);
        dup2(file, 1);
    }
    if (currentCmd->error == 4) {
        //printf("error file: %s\n", currentCmd->errfile);
        file = open(currentCmd->errfile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        dup2(file, 2);
    }
    if (currentCmd->error == 5) {
        //printf("error append file: %s\n", currentCmd->outfile);
        file = open(currentCmd->outfile, O_WRONLY|O_CREAT|O_APPEND, 0666);
        dup2(file, 2);
    }
    if (currentCmd->error == 6) {
        //printf("error->output file: %s\n", currentCmd->outfile);
        file = open(currentCmd->outfile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        dup2(file, 2);
    }
    if (currentCmd->error == 1) {
        //printf("error->stdout \n");
        dup2(1, 2);
    }
}

char** argumentGenerator(CmdLink* currentCmd) {
    char** args = (char**)calloc((currentCmd->cmd_argc + 1), sizeof(char*));
    if (args == NULL) {
        printf("allocation failed\n");
        return NULL;
    }

    Word* tmp = currentCmd->cmd_argv;
    //printf("CMD : ");
    for(int i = 0; i <currentCmd->cmd_argc; i++) {
        if(tmp->data != NULL) {
            args[i] = tmp->data;
            tmp = tmp->next;
            //printf(", %s", args[i]);
        } else {
            i = currentCmd->cmd_argc;                            
        }
    }
        
    args[currentCmd->cmd_argc] = '\0';
    //printf(", %s\n", args[currentCmd->cmd_argc]);
    return args;
}