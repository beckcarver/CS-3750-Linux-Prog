/******************
 * reg.c
 * Author: Kim Buckner
 * Date: October 17, 2016
 * COSC 3750
 * 
 * This is an example of how to the standard library POSIX.2 
 * regular expressession functions. Refer to regex(7) and regex(3) for more
 * information and exact syntax, flags and return values. 
 */

#include<stdio.h>
#include<string.h>
#include<sys/types.h> // in case
#include<regex.h> // regcomp, regexec, regerror
#include<stdlib.h> // malloc if I need it
#include<errno.h>

int main()
{
  char errbuf[256];
  char outbuf[256];
  char *inbuf;
  regex_t pattern;
  regmatch_t matches[10];

  int rtn,othrtn;
  int i,len;
  size_t bsize;
  size_t size;
  size_t nmatch=10;

  /*
   * Default is "old" POSIX REs. Need to 
   * use the flag to get the extended (better?) version. 
   */
  rtn=regcomp(&pattern,"^HTTP/1.1 ([0-9]{3}) (.*)",REG_EXTENDED);

  if(rtn) 
  {
    size=regerror(rtn,&pattern,errbuf,256);
    fprintf(stderr,"%s",errbuf);
    if(size > 255) fprintf(stderr,"... more\n");
    else fprintf(stderr,"\n");
    return 1;
  }

  /*
   * The pointer is needed because getline() will realloc memory
   * if there is not enough for what it is trying to do. So cannot
   * use a static array.
   */
  inbuf=malloc(256);
  if(inbuf == NULL) {
    perror("inbuf");
    return 1;
  }
  bsize=256;

  /* 
   * The regmatch_t (matches array) is a struct that has a starting offset and
   * an ending offset. The end is one PAST the last character of the match.
   * That makes subtraction a valid way to get the length of the match.
   * If the rm_so is -1 there was no match. 
   *
   * strnlen() copies at most len bytes from source to destination. It does 
   * NOT put in a nul character to terminate the string. You have to do that
   * by hand.
   */

  while(1) {
    othrtn=getline(&inbuf,&bsize,stdin);
    if(othrtn > 0) {
      rtn=regexec(&pattern,inbuf,nmatch,matches,0);
      if(rtn != REG_NOMATCH) 
      {
        for(i=0;i<nmatch;i++) {
          if (matches[i].rm_so== -1) break;
          len=matches[i].rm_eo-matches[i].rm_so;
          strncpy(outbuf,inbuf+matches[i].rm_so,len);
          outbuf[len]='\0';
          printf("Match from %d to %d: %s\n",matches[i].rm_so,matches[i].rm_eo,
              outbuf);
        }
      }
    }
    else if(othrtn == -1) // EOF returned by getline()
    {
      break;
    }
  }
  /*
   * If program were not using "pattern" anymore or if I was going to reuse it 
   * for a different pattern need to do
   *
   * regfree(&pattern);
   *
   */

  return 0;
}

