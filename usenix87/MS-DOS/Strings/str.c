/*
 * str - extract the ASCII strings from a binary file
 *
 * This program opens a file and then copies input to stdout, searching for
 * ASCII strings.  A string is "tlen" printable ASCII characters terminated by
 * a NULL, <newline> or <return>, or "slen" characters ended with a
 * non-printing ASCII code.  It pauses output after SCRSIZ lines if output is
 * to the console screen.  The values of "tlen" and "slen" can be set by
 * command-line options.  To compile with DeSmet C define the symbolic constant
 * DESMET; for CI-C86 define the symbolic constant CIC86; for Microsoft C
 * define the symbolic constant MSC.  Your config.sys file must contain
 * "device=ansi.sys".
 *
 * This program was written by R. Edward Nather and is placed in the Public
 * Domain without any restrictions whatsoever.
 */

#include <stdio.h>
#include <ctype.h>
#include "intdefs.h"

int slen = 6;       /* default length for unterminated string */
int tlen = 4;       /* default length for terminated string   */

int tsc;            /* set true if output is to console screen */

main(argc,argv)
int argc;
char *argv[];
{
extern int tsc, toscreen();
int i;
FILE *fp;

tsc = toscreen();
while(argc > 1 && argv[1][0] == '-') {
    switch(argv[1][1]) {
        case 'l':           /* -l: set min unterminated string length */
            slen = atoi(&argv[1][2]);
            break;
        case 't':           /* -t: set min terminated string length */
            tlen = atoi(&argv[1][2]);
            break;
        default:
            fprintf(stderr, "str: unknown option \"%s\"\n", argv[1]);
            exit(1);
        }
    argc--;
    argv++;
    }
if(argc < 2) {
	fprintf(stderr, "Use: str [-t##] [-l##] filename [filename...]\n");
	exit(1);
	}
for(i = 1; i < argc; i++)
    if((fp=fopen(argv[i], "rb")) == NULL) {
        fprintf(stderr, "str: can't open \"%s\"\n",argv[i]);
        exit(1);
        } 
    else {
        filecopy(fp);
        fclose(fp);
    }
exit(0);
}

#define MAX 81              /* insert newline before 81st char */
    
filecopy(fp)
FILE *fp;
{
int c;
unsigned char hold[MAX];             /* buffer to hold ASCII strings */
int cnt = 0;                /* current string length */
int wf = 0;                 /* 80 characters seen if non-zero */
int i;

while((c = getc(fp)) != EOF) {
    if(isprint(c)) {                        /* if char is printable ASCII */
        hold[cnt++] = c;                    /* save it */
        hold[cnt] = 0;                      /* terminate potential string */
        if(cnt >= MAX) {
            fputs(hold, stdout);            /* print a buffer full */
            cnt = 0;
            hold[0] = 0;
            wf = 1;                         /* and remember we did it */
            }
        }
    else {                                          /* if char not printable */
        if(wf || cnt >= slen || (cnt >= tlen &&
            (c == NULL || c == '\r' || c == '\n'))) {   /* if it's a string */
                fputs(hold, stdout);                            /* print it */
                if(isprint(c &= '\177')) {      /* some strings end with    */
                    fputc(c, stdout);           /* 8th bit set in last char */
                    }
                fputc(endlin(), stdout);              /* end the line */
                }
        cnt = wf = 0;
        }
    }
}


/* endlin - end a line and watch for screen overflow */

#define SCRSIZ 22
static int lc = 0;                                        /* line counter */

endlin()
{
extern int tsc;                             /* true if output is to screen */
register int c;

if(tsc && ++lc >= SCRSIZ) {        /* pause if output is to console screen */
    fputs("\n\033[7m--More--", stdout);     /* and we've shown a screenful */
    c = GETC;
    fputs("\033[0m\r\033[K ", stdout);
    switch(c) {
        case '\r':                          /* <RETURN> - show 1 more line */
            lc = SCRSIZ - 1;
            break;
        case 'q':                             /* quit with "q" or "ctrl-C" */
        case '\003':
            exit(0);
        default:
            lc = 0;                         /* else show another screenful */
            break;
        }
    return('\b');
    }   
else
    return('\n');
}


/* toscreen - find out if output is to console screen */

toscreen()
{
STRUCTURE

RAX = 0x4400;
RBX = 1;
CALLDOS;
return((RDX & 1) && (RDX & 0x80));                      /* isdev && iscin */
}
