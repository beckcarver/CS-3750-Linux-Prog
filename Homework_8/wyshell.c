/*
 * wytalkC.c
 * Author: Beckham Carver
 * Date: April 11, 2023
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

        current = 0;
        inputDir = 0;
        errorDir = 0;
        outputDir = 0;
        errOut = 0;
        isFile = 0;
        runError = 0;

        rtn=parse_line(buf);
        while(rtn !=  EOL && !runError){ 
            switch(rtn) {

                case WORD:
                    if (current == 0) {
                        printf(":--: %s\n", lexeme);
                        current = 1;
                        break;
                    }
                    else if (isFile == 1) {isFile = 0;}
                    printf("--: %s\n",lexeme);
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
                    current = 0;
                    inputDir = 0;
                    outputDir = 0;
                    errorDir = 0;
                    errOut = 0;
                    printf(" ;\n");
                    break;

                case AMP:
                    if (isFile) {
                        printf("error & : file not given\n");
                        runError = 1;
                        break;
                    }
                    printf(" &\n");

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
                    printf(" |\n");
                    break;
                
                case APPEND_OUT:
                    if (isFile) {
                        printf("error >> : file not given\n");
                        runError = 1;
                        break;
                    }
                    printf(" >>\n");

                case APPEND_ERR:
                    if (isFile) {
                        printf("error << : file not given\n");
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
            else {printf("--: EOL\n");}
        }
        printf("\n");
    }
}
