/*
 * wytalkC.c
 * Author: Beckham Carver
 * Date: April 25, 2023
 *
 * COSC 3750, Homework 8
 *
 * General logic for a shell program, this takes in input from the user
 * and parses that input, then outputs the action to be taken.
 *
 * !!NOTE!!, this does not store the parsed information in a command data 
 * structure, this is only the logic for how to treat the user commands, 
 * consider all printf() calls (not error) portions of the code where we would
 * store the command in our command data structure.
 */

#include<stdio.h>
#include"wyscanner.h"
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

Word* createWord(const char* data);
Word* appendWord(Word* word, const char* data);
void freeWords(Word* word);

CmdLink* createCmdLink(const char* cmd_data);
CmdLink* appendCmdLink(CmdLink* cmdLink, const char* cmd_data);
void freeCmdLinks(CmdLink* cmdLink);

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
    int inputDir = 0;
    int errorDir = 0;
    int outputDir = 0;
    int errOut = 0;
    int isFile = 0;
    int runError = 0;

    while(1) {
        printf("&> ");
        rpt=fgets(buf,256,stdin);

        if(rpt == NULL) {
            if(feof(stdin)) {
                return 0;
            }
            else {
                perror("fgets from stdin");
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
                        currentCmd = createCmdLink(lexeme);
                        printf("cmd %d\n", currentCmd->cmd_argc);
                        printf(":STARTED: %s\n", lexeme);
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
                    printf(":APPEND ARGV: %s\n",lexeme);
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
                    printf(" >\n");
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
                    printf(" <\n");
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
                    printf(" 2>\n");

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
                    printf(" 2>&1\n");
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
                    
                    inputDir = 0;
                    outputDir = 0;
                    errorDir = 0;
                    errOut = 0;

                    current = 0;
                    currentCmd = currentCmd->next;
                    printf(":SEMICOLON:\n");
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
                    printf(" &\n");
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

                    currentCmd = currentCmd->next;
                    printf(" |\n");
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
                    printf(" >>\n");
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
                    printf(" <<\n");
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
                printf("--: EOL\n");
                while(currentCmd->prev != NULL) {
                        currentCmd = currentCmd->prev;
                }
                while(currentCmd != NULL) {
                    printf("NUM is %d", currentCmd->cmd_argc);
                    
                    char** args = (char**)malloc((currentCmd->cmd_argc + 1) * sizeof(char*));
                    if (args == NULL) {
                        perror("allocation failed");
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

Word* createWord(const char* data) {
    Word* newWord = (Word*)malloc(sizeof(Word));
    newWord->next = NULL;
    newWord->prev = NULL;
    newWord->data = strdup(data); // Copy the data string
    return newWord;
}

Word* appendWord(Word* word, const char* data) {
    Word* newWord = createWord(data);
    Word* currentWord = word;
    while(currentWord->next != NULL) {
        currentWord = currentWord->next;
    }
    currentWord->next = newWord;
    newWord->prev = currentWord;
    return newWord;
}

CmdLink* createCmdLink(const char* cmd_data) {
    Word* cmd_argv = createWord(cmd_data); // Create a new Word from cmd_data
    CmdLink* newCmdLink = (CmdLink*)malloc(sizeof(CmdLink));
    newCmdLink->cmd_argv = cmd_argv;
    newCmdLink->cmd_argc = 1;
    newCmdLink->input = 0;
    newCmdLink->output = 1;
    newCmdLink->error = 2;
    newCmdLink->infile = NULL;
    newCmdLink->outfile = NULL;
    newCmdLink->errfile = NULL;
    newCmdLink->prev = NULL;
    newCmdLink->next = NULL;
    return newCmdLink;
}

CmdLink* appendCmdLink(CmdLink* current, const char* cmd_data) {
    CmdLink* newCmdLink = createCmdLink(cmd_data);
    current->next = newCmdLink;
    newCmdLink->prev = current;
    return newCmdLink;
}


void freeWords(Word* word) {
    while (word != NULL) {
        Word* temp = word;
        word = word->next;
        free(temp->data);
        free(temp);
    }
}

void freeCmdLinks(CmdLink* cmdLink) {
    while (cmdLink != NULL) {
        CmdLink* tmp = cmdLink;
        cmdLink = cmdLink->prev;
        freeWords(tmp->cmd_argv);
        free(tmp->cmd_argv);
        free(tmp->infile);
        free(tmp->outfile);
        free(tmp->errfile);
        free(tmp);
    }
}

void execute(char** args){
    pid_t pid;
    int stat;
    pid = fork();
    if(pid ==0){
        if(execvp(*args, args) < 0){
            printf("wyshell: %s: command not found\n",args[0]);
            exit(1);
        }
        exit(0);
    }
    if(pid > 0){
        wait(&stat);
    }
}
