/*
PC-More, an improved pager for the IBM PC running PC-DOS 2.0 or later.
*/

#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define NCPL 79     /* Number of characters per line. */
#define NLPS 24     /* Number of lines per screen. */
#define NLPHS 12    /* Number of lines per half screen. */ 
#define CMASK 0377  /* Masks high bits of an int to zero. */

int x_mnl, x_srf, x_argc;
long x_rwp;
char x_file[128], *x_argv[128];
char *x_spc = "                                                     ";
FILE *x_fp = 0;


/*
x_mnl is the number of line to print before calling wait().

x_srf if a flag which is set to 1 if stdin is redirected, 0 if not.  This is 
determined by the number of parameters on the command line.  If there are no 
parameters on the command line, then it is assumed that redirection is taking 
place.  If there are parameters on the command line, then it is assumed that 
there is no redirection.  This is not neccessarily the case, but it is usually 
safe to assume that no one will type something like this:

    dir|more file.ext

If something like this is typed, then More will assume that stdin is not 
redirected, even though it is.  This can produce bad results if a push of some 
sort it attempted (via the '!' or 'e' commands).  Unfortunately, there is no 
other way to detect if stdin is being redirected.  This construction will work 
if something like the above example is not typed.  By the way, that example
will read from file.ext, not from the pipe.

x_rwp is the offset relative to the beginning of file which is given to fseek() 
in the bang() function.  The fseek() will return the file to where it was left 
before the push took place.

x_fp is the file pointer from which data is read.

x_file is an array which holds the name of the file currently being read from.
it is large enough to hold a very long pathname.

x_argc and x_argv are the external copies of argc and argv.

x_spc points to the string used by several functions to clear the screen.

*/


main(argc, argv)
int argc;
char *argv[];

{
    int ci, c, nc, nl, nlp, ts, i;
    char *strcpy();

/*
c is ci with the upper 8 bits masked to zero.

ci is the character read from the file.

i is a working variable used in for loops, etc.

nc is the number of characters on the current line.  It is used to detect lines 
that are too long and to determine where the next tab stop is.

nl is used to determine when to exit the for loop and call wait.

nlp is used to determine when to clear the screen.  The screen is only cleared 
when it is full.

ts is a working variable for the tab expander.  It is the number of spaces to 
the next tab stop.
*/
    x_argc = argc;              /* \   These 3 lines set up the external */
    for (i = 0; i < argc; i++)  /*  >  copies of argc and argv.          */
        x_argv[i] = argv[i];    /* /                                     */

    if (argc >= 2) {
        nextfile(1);    /* This function will set x_fp and x_file. */
        x_srf = FALSE;  /* This flag set to 0 if not reading from stdin. */
    }
    else  {
        x_fp = stdin;
        strcpy(x_file, "stdin");
        x_srf = 1;      /* This flag set to 1 if reading from stdin. */
    }
    x_mnl = NLPS;   /* Start off by outputing one screenful. */
    nc = 0;         /* Haven't put out any characters yet. */
    nlp = 24;       /* This will make sure that the screen is cleared. */
    while(TRUE) {   /* Loop forever, guts of the loop will exit the loop. */
        if ((nlp >= NLPS) && (x_mnl != 1)) {

/*
The (x_mnl != 1) condition keeps the screen from being cleared if only one
line is to be displayed.
*/

            crt_cls();  /* This clears the screen. */
            crt_mode(7);    /* crt_cls() is broken, this makes chars print. */
            nlp = 0;    /* Reset number of lines printed. */
        }
        for (nl = 0; nl < x_mnl; ) { /* Put out x_mnl number of lines. */
            if ((ci = getc(x_fp)) == EOF) {
                wait(1);    /* Prompt for next file. */
                nlp = 0;    /* Wait(1) will clear screen, so reset nlp */
                nl = 0;     /*  and nl to 0 to reflect this. */
                ci = '\0';  /* Ci will still be printed, so make it a null. */
            }
            c = ci & CMASK; /* Mask off upper 8 bits for safety. */
            if (c == '\t') {  /* Tab expander */
                if ( nc > NCPL - 6) {   /* This is part 1 (see notes below). */
                    c = '\n';
                }
                else {                  /* This is part 2. */
                    ts = 8 - (nc % 8);  /* Find spaces to next tab. */
                    if (ts == 8)        /* If it comes out to be 8, */
                        ts = 0;         /* then it should be 0. */
                    for (i = 1; i < ts; i++) {
                        nc++;           /* Increment # of chars. */
                    }  /* for */
                }  /* else */
            }  /* End of tab expander */

/*
If 73 or more chars have been printed out, then part 1 of the tab expander
will turn the tab into a newline.  In this program, tab stop 81 is the same as
column 1 on the next line.  Since 81 is the next tab stop after 73, changing
the tab into a newline is the correct solution.  The rest of the program does
not even know that this has happened and simply thinks that a newline was read
from the file.

If 72 or less chars have been printed, then part 2 finds out how many spaces
to the next stop and increments nc the proper number of times minus 1.  The tab
is not actually expanded, the char count is just incremented.  When the tab
prints, it is automagically expanded.  It is important that this tab expander
should come before newline detection and the truncate-too-long-line algorithm.

*/

            if ((nc >= NCPL) && (c != '\n')) {  /* If beyond right margin, */
                nl++;           /* increment # of lines, */
                nlp++;          /* increment # of lines on current screen, */
                nc = 0;         /* reset # of chars on current line to 0, */
                putchar('\n');  /* start a new line, */
                ungetc(c,x_fp); /* put the char back where it came from, */
                c = '\0';       /* and make c a NULL character. */

/*
c is made a NULL so that a test can be performed before "putchar(c)" and 
"nc++" are executed.  If c is a NULL, these operations are not performed.
The "ungetc(c,x_fp)" is executed so that the char is the first one available
for the next line.  This stuff is needed so that printing out one line to the
screen will not result in the char being printed in front of the prompt.
This does not happen when printing several lines.  A newline is not ungetced
because it will cause a blank line to be printed when it is put on the next
line.
*/

            }  /* if */
            if (c != '\0') {    /* If c is not a NULL, */
                putchar(c);     /* print the character, */
                nc++;           /* and increment # of chars on line. */
            }  /* if */
            if (c == '\n') {    /* If c is a newline, */
                nl++;           /* increment # of lines, */
                nlp++;          /* increment # lines on current screen, */
                nc = 0;     /* and reset # of chars on current line to 0. */
            }
        }  /* for */


    wait(0);    /* After max # of lines have been output, prompt user. */
    }  /* while */
}  /* main */


wait(e_o_f)  /* Print prompt, then wait for and iterpret command. */

/*
If wait is called with an argument of 0, then it has been called in the middle 
of a file.  If it is called with an argument of 1, then it has been called at 
EOF and it will call nextfile() as necessary.  The prompt is also made
different at EOF.
*/

int e_o_f;

{
    int w_c, i, arglen, pgf;
    char cmd[116], prompt[128], arg[6], *editor, *envfind(), *strcat();
    char *strcpy();
    FILE *fopen(), *temp;
    long ftell();

    strcpy(prompt,x_file);          /* Prompt with the filename. */
    if (e_o_f)
        strcat(prompt," [EOF]");    /* If at EOF, make "EOF" part of prompt. */
    printf("--%s--",prompt);        /* Print prompt. */
    arg[0] = '\0';                  /* Initialize the argument string and */
    arglen = 0;                     /*  its length. */
    pgf = FALSE;                    /* Set the "prompt gone flag". */

getcmd:

/*
Get char from keyboard and mask off upper 8 bits, which contain the scan code
of the key pressed.  Then act on it as a command.
*/

    w_c = (key_getc() & CMASK); /* Get char and mask off scan code. */

/*
All the cases in this switch statment (except the for the default) which end 
in a "goto getcmd;" statement contain the statments: 

            arg[0] = '\0';
            arglen = 0;
            pgf = FALSE;

This resets the argument back to 0.  Even commands which do not use the
argument must reset it to 0 to avoid problems.  This also resets the
"prompt gone flag" to false.  This flag is used to determine if the prompt
has been erased for printing the argument.  If the prompt has not been erased,
pgf = FALSE.  When a digit is typed, the prompt is erased, the digit printed, 
and pgf set to TRUE.  When a command is executed, it will always cause the 
prompt or some message to be printed, so pgf is set to FALSE again.  This is
not needed when a case ends in "break," since wait() returns when the switch
exits.

An argument of 0 is interpreted by the function getval() to mean 1, so when
no argument is entered, the default is 1.
*/

    switch (w_c) {
        case ' ':               /* Space means "next screen". */
            if (e_o_f)          /* If at end of current file, */
                nextfile(1);    /*  go to next file. */
            x_mnl = NLPS * getval(arg); /* Get # of pages to move. */
            x_rwp = ftell(x_fp);    /* Save current position in file for !. */
            break;              /* Break out of switch. */
        case '\n':
        case '\r':              /* Enter key means "next line". */
            if (e_o_f)          /* Everything else is similar to above. */
                nextfile(1);
            x_mnl = getval(arg);
            x_rwp = ftell(x_fp);
            break;
        case 'h':
        case 'H':               /* 'H' means "next half screen". */
            if (e_o_f)
                nextfile(1);
            x_mnl = NLPHS * getval(arg);
            x_rwp = ftell(x_fp);
            break;
        case 'r':
        case 'R':               /* 'R' Means "rewind current file". */
            rewind(x_fp);
            x_mnl = NLPS;       /* Print out full screen next time. */
            x_rwp = 0L;         /* Current position in file is beginning. */
            break;
        case 'n':
        case 'N':               /* 'N' means "go to next file". */
            nextfile(1);        /* Nextfile(1) goes to next file. */
            x_mnl = NLPS;       /* Put out a full screen next time. */
            break;
        case 'p':
        case 'P':               /* 'P' means "go to previous file". */
            x_mnl = NLPS;       /* Put out a full screen next time. */
            if (nextfile(0)) {  /* Nextfile(0) goes to previous file. */
                arg[0] = '\0';
                arglen = 0;
                pgf = FALSE;
                goto getcmd;    /* If you tried to go before 1st file, */
            }  /* if */         /*  a message is printed and you try a diff. */
            else
                break;          /*  command, otherwise, exit "switch". */
        case 'f':
        case 'F':
            printf("\r%s\rNew file:  ",x_spc);
            i = key_gets(cmd, 13, "New file:  Aborted.");   /* Get filename. */
            if (i == -1) {          /* If key_gets() aborted, */
                arg[0] = '\0';
                arglen = 0;
                pgf = FALSE;
                goto getcmd;        /*  then get next command. */
            }  /* if */
            temp = fopen(cmd,"r");  /* Attempt to open file just named. */
            if (temp == 0) {        /* If couldn't open file, */
                printf("\r%s\rCould not open %s.", x_spc, cmd); /* say so. */
                arg[0] = '\0';
                arglen = 0;
                pgf = FALSE;
                goto getcmd;        /* Get next command. */
            }  /* if */
            else {                      /* Otherwise, */
                if (x_fp != stdin)      /* if not reading from stdin, */
                    fclose(x_fp);       /* close old file, */
                x_fp = temp;            /* set x_fp to new file, */
                strcpy(x_file, cmd);    /* make x_file correct, */
                x_mnl = NLPS;           /* set x_mnl to its default, */
                x_rwp = 0L;             /* and set x_rwp to top of file. */
            }  /* else */
            break;

/*
Note that x_srf is not reset to 0.  Even if you quit reading from stdin, it is
still redirected because of the < or | on the command line.  This means that
all of the restrictions still apply; i.e. you still cannot push to an inferior
shell after you change files, etc.  You will also not be able to edit if you 
change files after reading from a pipe.  This should not be too much of an 
annoyance, since reading from a pipe and reading from a file are usually not 
done in the same invokation of More.
*/
        case 'q':
        case 'Q':               /* 'Q' means "quit". */
            printf("\r%s\r",x_spc); /* Erase prompt or message on last line. */
            exit(0);
        case 'e':
        case 'E':               /* 'E' means "edit current file". */
            if (x_srf) {        /* If stdio is redirected, can't edit. */
                printf("\r%s\rCannot edit with stdin redirected.", x_spc);
                arg[0] = '\0';
                arglen = 0;
                pgf = FALSE;
                goto getcmd;    /* Go get another command. */
            }  /* if */
            editor = envfind("EDITOR"); /* Get editor name from environ var. */
            if (editor == 0)            /* If there is no such environ var., */
                editor = "edlin";       /* then use EDLIN. */
            strcpy(cmd, editor);        /* Start command string with editor. */
            strcat(cmd, " ");           /* Now put a space. */
            strcat(cmd, x_file);        /* Now end command with filename. */
            if (strcmp(editor,"edlin")) /* If editor != "edlin", string was */
                free(editor);           /*  obtained with malloc, free it. */
            bang(strlen(cmd),cmd,prompt);   /* Execute the command. */
            arg[0] = '\0';
            arglen = 0;
            pgf = FALSE;
            goto getcmd;                /* Get next command. */
        case '!':               /* '!' means "execute DOS command". */

            printf("\r%s\r!",x_spc);    /* Erase last line, print '!'. */
            i = key_gets(cmd, 115, "Push aborted.");  /* Get command string. */
            if (i == -1) {
                arg[0] = '\0';
                arglen = 0;
                pgf = FALSE;
                goto getcmd;
            }  /* if */

/*
A command string passed to DOS must not be more than 127 characters long.  DOS 
will ignore the rest.  Since the system() command must always pass the string
"command /c" to DOS (see DOS 2.0 manual), that leaves 117 characters.  I
rounded to 115 to give myself a bit of headroom.

See definition of key_gets() to see what it expects for arguments.  It returns 
the number of characters read from the keyboard or -1 if it was aborted.
*/

            if (x_srf) {    /* If stdin is redirected, issue warnings. */
                if (!i) {   /* If i == 0, then this is a push to shell. */
                    printf("\r%s\rCan't push when reading from stdin.", x_spc);
                    arg[0] = '\0';
                    arglen = 0;
                    pgf = FALSE;
                    goto getcmd;
                }  /* if */
                printf("\r%s\rThis is risky when reading from stdin.  ",x_spc);
                if (!confirm()) {
                    printf("\r%s\rAborted.",x_spc);
                    arg[0] = '\0';
                    arglen = 0;
                    pgf = FALSE;
                    goto getcmd;
                }
            }

            bang(i,cmd,prompt);    /* Bang pushes to an inferior command.com. */
            arg[0] = '\0';
            arglen = 0;
            pgf = FALSE;
            goto getcmd;

/*
If stdin is redirected, then pushing to DOS is bad news.  If a program 
(including command.com) reads from stdin (not through the BIOS or directly from 
the keybd), then instead of reading from the keybd, it gets its input from the 
pipe which More is reading from.  If you simply pushed to an inferior shell,
result is similar to a batch file.  This is a fault of DOS, not mine.
*/

        case '?':           /* '?' means "help". */
            printf("\r%s",x_spc);   /* Erase prompt or message. */
            help(prompt);   /* Help prints out the prompt, so it needs it, */
            fseek(x_fp, x_rwp, 0);  /* and reposition read/write pointer. */
/*
The read/write pointer is backed up so that the screen which was erased by
help is reprinted.
*/

            arg[0] = '\0';
            arglen = 0;
            pgf = FALSE;
            goto getcmd;
        case 'v':
        case 'V':               /* 'V' means "print version number." */
            printf("\r%s\rPC-More v1.2",x_spc);
            arg[0] = '\0';
            arglen = 0;
            pgf = FALSE;
            goto getcmd;
        case '\007':            /* ^G means abort current argument. */
            arg[0] = '\0';
            arglen = 0;
            pgf = FALSE;
            printf("\r%s\r--%s--", x_spc, prompt);  /* Erase argument. */
            goto getcmd;
        default:
            if (w_c >= '0' && w_c <= '9') { /* If w_c is a digit, */
                if (!pgf) {                     /* If prompt is not gone, */
                    printf("\r%s\r",x_spc);     /* erase it, */
                    pgf = 1;                    /* and set flag. */
                }
                arg[arglen++] = w_c;        /*  add w_c to string, */
                arg[arglen] = '\0';         /*  terminate string properly, */
                putchar(w_c);               /*  and put w_c on screen. */
                if (arglen > 3) {           /* If more than 3 digits long, */
                    printf("\r%s\rArgument too long.",x_spc);   /* say so, */
                    arg[0] = '\0';          /*  and start over. */
                    arglen = 0;
                    pgf = FALSE;
                }  /* if */
                goto getcmd;
            }  /* if */

/*
Arg[] is limited to 3 digits because the largest integer which can be held in
a int is 32767.  Arg[] is multiplied by a number which can be as large as 24.
24 * 999 (the largest 3 digit number) = 23976, which is within the range of an
int.  24 * 9999 (the largest 4 digit number) = 239976, which is too big to fit
in an int.  Therefore, it is a safety precaution not to allow any number larger
than 3 digits to be entered as an argument.
*/

            if (w_c == '\b' && arglen >= 1) {    /* If w_c is a backspace, */
                arg[--arglen] = '\0';   /* then delete the last char in */
                fputs("\b \b",stdout);  /* arg[] if there is one there */
                goto getcmd;            /* and erase it from the screen. */
            }  /* if */
            putchar('\007');    /* If the above tests fail, then beep. */
            goto getcmd;
    }  /* switch */
    printf("\r%s\r",x_spc);         /* Erase prompt or message. */
}  /* wait */                       /* Now return to main(). */


help(prompt)    /* Prints out help text and the prompt. */

char *prompt;

{
    char *ht;

    ht = "\nPC-More Help:\n\n    <sp>    next screen\n    <cr>    next line\n\
    h       next half page\n    r       rewind current file\n\
    e       edit current file\n    n       go to next file\n\
    p       go to previous file \n    f       choose a new file\n\
    !       push to shell\n    ?       print this text\n\
    v       print version\n    ^G      abort a command while entering text\n\
    q       quit\n\n";

    crt_cls();
    crt_mode(7);
    printf("%s--%s--",ht,prompt);
}


bang(i,cmd,prompt)

/*
Bang() will push to an inferior shell if i == 0 or it will execute the 
command pointed to by cmd if i != 0.  The command can be an internal DOS
command or it can be a program to run.
*/

int i;          /* Length of command to execute. */
char *cmd;      /* Pointer to command to execute. */
char *prompt;   /* Pointer to prompt to print when done. */

{
    int retrn;
    FILE *fopen();

    if (!x_srf)                     /* If reading from a file, then */
        fclose(x_fp);               /* close file before push. */

    if (!i)                         /* If no command was entered, */
        retrn = system("command");  /* push to DOS. */

    else                            /* If a command was entered, */
        retrn = system(cmd);        /* give DOS the command in cmd[]. */

    if (retrn)
        printf("Push not successful, DOS error code = %d", retrn);

    if (!x_srf) {                   /* If reading from a file, */
        x_fp = fopen(x_file,"r");   /* reopen file, */
        if (x_fp == 0)              /* check to see if it was reopened, */
            abort("Unable to reopen file after push to DOS.");

        fseek(x_fp, x_rwp, 0);      /* and reposition read/write pointer. */
    }
    else                            /* If reading from stdin, */
        x_fp = stdin;               /* then "reopen" it. */

    fputs("\n\nReturning to more...\n\n",stdout);
    printf("--%s--",prompt);

}

key_gets(string, len, msg)

/*
Key_gets() gets a string by reading one character at a time from the 
keyboard, not from stdin.  It reads up to len-1 characters and puts them in 
string[].  If the user tries to backspace too far, then this is interpreted to 
mean "I really didn't want to do this after all," and key_gets() aborts, 
returning -1 to signal error.  Key_gets() will read up until it receives a 
carriage return, a linefeed, or an end of file indication.  It returns the 
number of characters actually read, or -1 if it was aborted.
*/

int len;        /* How much to get. */
char string[];  /* Where to put it. */
char *msg;      /* What to say when aborting. */

{

    int i, k_c;

    for (i = 0; i < len; i++) { /* Get len-1 characters for the string. */
        k_c = (key_getc() & CMASK);     /* Get char, mask scan code. */ 

        if (k_c == '\r' || k_c == '\n' || k_c == EOF) {
            break;  /* If 'Enter' or EOF found, quit getting characters. */
        }  /* if */

        if (k_c == '\b') {  /* If 'backspace', */
            if (i > 0) {    /* don't erase before start of command. */
                string[--i] = '\0'; /* remove last char in string[], */
                fputs("\b \b",stdout);  /* erase it from screen. */
            }  /* if */
            else {  /* If user tries to erase too far, then abort. */
                printf("\r%s",msg); /* Print aborting message. */
                return (-1);        /* Return with error. */
            }  /* else */
            i--;    /* Dec. i again since 'for' will increment it, */
            continue;               /* and go to end of for loop. */
        }  /* if */

        if (k_c == '\007') {                /* If 'bell', then abort: */
            printf("\r%s\r%s", x_spc, msg); /* Erase anything, print msg, */
            return (-1);                    /* and return with error. */
        }  /* if */

        putchar(k_c);   /* If not 'enter' or 'bs', then echo it, */
        string[i] = k_c;   /* and add it to the command string. */
    }  /* for */
    string[i] = '\0';  /* Terminate the command string. */
    return (i);
}  /* key_gets() */

/*
I can't use gets() to collect the string since it reads from stdin.  If stdin
is redirected, strange things will happen.  That's why I wrote key_gets() to
read from the keyboard.  
*/

confirm()

{
    int c_c;

    printf("Confirm [N]\b\b");
    c_c = key_getc() & CMASK;
    putchar(c_c);

    if (c_c == 'y' || c_c == 'Y')
        return(1);
    else
        return(0);
}


nextfile(next)

int next;

/*

Nextfile steps through the argument list on the command line, trying to open 
each argument as a file.  If it succeeds, then x_fp points to the file and 
x_file points to a string which is the name of the file.  If it fails, abort is 
called.  Since where is a static variable, this function "remembers" where it 
was the last time it was called and moves on to the next argument on the
command line each time it is called.  Nextfile does not return a value, since
x_fp and x_file are external variables.  
Note:  Nextfile also sets x_rwp to point to the top of the file.

*/

{
    static int where;
    FILE *fopen();

    if (next) {                 /* If going to next file do this: */
        if (++where == x_argc)  /* Go to next file.  Gone too far? */
            exit(0);            /* If so, exit. */
    }
    else {                      /* If going to previous file, do this: */
        if (--where < 1) {      /* Go to prev file.  Gone too far? */
            printf("\r%s\rAt first file.",x_spc);   /* If so, say so, */
            where++;            /* put where back where it belongs, */
            return(1);          /* and return with an error code. */
        }
    }

    if (x_fp)           /* If x_fp != 0 then it has been opened, so */
        fclose(x_fp);   /* close it so won't run out of file handles. */
    x_fp = fopen(x_argv[where], "r");   /* Open next (or previous) file. */
    strcpy(x_file, x_argv[where]);      /* Set x_file to the new filename. */
    x_rwp = 0L;                         /* Tell ! that at start of file. */

    if (x_fp == 0)
        abort("Could not open file %s.\n",x_argv[where]);
    crt_cls();
    crt_mode(7);    /* Clear screen before going to the next file. */
    return(0);      /* For consistancy, return with an OK code. */
}

/*
Nextfile will return a value of 1 only if you try to go to the file which is 
before the first file on the command line.  If no error occurs, then it returns 
0.  If any other error occurs, then it is fatal and execution is aborted.  The 
only place where the return value is checked is "case 'p':" segment of the 
wait() subroutine.
*/


getval(string)
char *string;

{
    int i;                          /* This function gets the number entered */

    i = atoi(string);               /* as an argument in wait().  If atoi() */
    if (i == 0)                     /* returns 0, then it is assumed that no */
        i = 1;                      /* argument was entered and that the */
                                    /* default of 1 should be used. */
    return(i);
}
