/*
** Program:      TLOCK
**
** Function:     VT200 terminal lock program.
**
** Author:       Grenville Whelan 1986, (gdw@uk.co.ssl-macc)
**
** Description:  TLOCK is basically another 'lock' program, but with screen
**               layout designed to look like a VT200 set-up screen. When
**               called, the user is prompted for a key, (or password), and
**               verification of that key; the terminal then "locks" up
**               until the same key is re-entered. A prompt for this "unlock"
**               key will appear once "CONTROL-C" is entered. If the key is
**               incorrect, the screen will be re-drawn, and will continue
**               to "lock" until the correct key is given. If the key has been
**               forgotten, the "tlock" process must be "killed" from another
**               terminal. On a successful key, the terminal will be reset,
**               and the user can continue what he was doing prior to the lock.
**
** Arguments:    None read.
**
** Compile:      cc tlock.c -o tlock -lcurses -ltermlib
**
** Notes:        This utility will also work on VT100 compatibles and probably
**               most other VT100 compatible terminals as well, although the
**               Set-up feature that this is designed to immitate will be
**               inaccurate.
**
*/

#include <stdio.h>
#include <signal.h>
#include <curses.h>

#define MAXNAMELEN 256			/* Max name length */
#define CLEAR "\33[H\33[J"		/* DEC ESC to clear screen */
#define BELL "\7"			/* DEC ESC to ring bell */
#define PROMPTLINE "\33[24;28f"		/* Cursor position for prompt line */
#define CLETOEOL "\33[K"		/* Clear to end of line escape */
#define ATTOFF "\33[0m"			/* Turn video attributes off */
#define BOLDON "\33[1m"			/* Turn bold attribute on */
#define CURSON "\33[?25h"		/* Make cursor visible */
#define CURSOFF "\33[?25l"		/* Make cursor invisible */

long random();				/* Random number generator */

char key[MAXNAMELEN];			/* User-input lock-key */

int ok;

/* Set-up options */

char *ops[] = {        "\33[7m Display \33[0m",
            "\33[7m General \33[0m",
            "\33[7m Comm \33[0m",
            "\33[7m Printer \33[0m",
            "\33[7m Keyboard \33[0m",
            "\33[7m Tab \33[0m",
            "\33[7m On Line \33[0m",
            "\33[7m Clear Display \33[0m",
            "\33[7m Clear Comm \33[0m",
            "\33[7m Reset Terminal \33[0m",
            "\33[7m Recall \33[0m",
            "\33[7m Save \33[0m",
            "\33[7m Set-Up=English \33[0m",
            "\33[7m North American Keyboard   \33[0m",
            "\33[7m Default \33[0m",
            "\33[7m Exit \33[0m" };

/* Cursor positions for Set-up options */

char *ps[] = {         "\33[18;1f",
            "\33[18;11f",
            "\33[18;21f",
            "\33[18;28f",
            "\33[18;38f",
            "\33[18;49f",
            "\33[20;1f",
            "\33[20;11f",
            "\33[20;27f",
            "\33[20;40f",
            "\33[20;57f",
            "\33[20;66f",
            "\33[22;1f",
            "\33[22;18f",
            "\33[22;46f",
            "\33[22;56f" };
main()

{
    int i=0, sig, sig_trap(), n=0;
    char again[MAXNAMELEN];

    initscr();

    /* Initialise interrupt routine */

    for(sig=0; sig<NSIG; sig++)
            signal(sig,sig_trap);

    /* Draw VT200 set-up screen */

    printf("\33[H\33[J\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\33#6\33[1m\33[5mSet-up Directory\33[0m     ");
    printf("         \33[4mVT220 V2.0\33[0m\n");
    printf("\n\n\n\n\n----------------------------------------");
    printf("----------------------------------------\n");
    printf("    Replace Mode");

    for(i=0; i<16; ++i)
    {
        printf("%s\33[1m%s\33[0m\n",ps[i],ops[i]);
        printf("%s%s\n",ps[i],ops[i]);
    }


    /* Turn off terminal echo */

    noecho();

    /* Get lock key */

    printf("%s%s%sKey: ",ATTOFF,PROMPTLINE,CURSON);
    gets(key);

    /* Get verificational key */

    printf("%s%s%sAgain :",ATTOFF,PROMPTLINE,CURSON);
    gets(again);

    /* Verify */

    if(strcmp(key,again))
    {
        printf("%s%s",BELL,CLEAR);
        echo();
        endwin();
        exit();
    }

    /* Re-draw upset screen */

    printf("%sPrinter: None%s",PROMPTLINE,CURSOFF);
    fflush(stdout);

    /* Continuously light up a random block on the screen */

    while(1)
    {
        n=(random() % 16);
	printf("%s%s%s%s\n",ps[n],BOLDON,ops[n],ATTOFF);
	ok = 1;
        sleep(1);
	ok = 0;
	printf("%s%s\n",ps[n],ops[n]);
    }

}

sig_trap(sig)			/* Interrupt handler */
int sig;			/* Signal number */
{
    char try[MAXNAMELEN];

    if(sig!=SIGINT)		/* Ignore anything but Control-C interrupt */
        return(0);

    if(!ok)
	return(0);

    /* Get key to unlock terminal from user */

    printf("%s%sKey :%s%s",ATTOFF,PROMPTLINE,CLETOEOL,CURSON);
    gets(try);

    /* If key is incorrect, relock screen and return */

    if(strcmp(try,key))
    {
        printf("%s%sPrinter: None%s",BELL,PROMPTLINE,CURSOFF);
        fflush(stdout);
        return(0);
    }

    /* Otherwise, clear screen and reset terminal and finally exit */

    else
    {
        printf("%s%s\n",CLEAR,CURSON);
        echo();
        endwin();
        exit();
    }
}
