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

    while(1) {
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
        rtn=parse_line(buf);
        while(rtn !=  EOL){ 
            switch(rtn) {
                case WORD:
                    printf("%s\n",lexeme);
                    break;
                case ERROR_CHAR:
                    printf("%d: %s\t =%d\n",rtn,tokens[rtn%96],error_char);
                    break;
                default:
                    printf("-------%d: %s\n",rtn,tokens[rtn%96]);
            }
            rtn=parse_line(NULL);
        }
        printf("BREAK\n");
    }
}
