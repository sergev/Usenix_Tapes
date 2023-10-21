#ifdef sysV
#include <termio.h>
#else
#include <sgtty.h>
#endif

/*
 *  Interface to termcap library.
 */

char    PC;
char    *BC, *UP;
char    *eeolseq, *cmseq;
char    *clearseq;
short   ospeed;
int     putchar();

tc_setup() {

    static  char    bp[1024];
    static  char    buffer[1024];
    char    *area = buffer;
    char    *getenv();
    char    *tgetstr();
    char    *name;
    int     retcode;

    name = getenv("TERM");

    retcode = tgetent(bp, name);

    switch(retcode) {

        case -1:
            printf("can't open termcap file.\n");
            exit(1);
            break;

        case 0:
            printf("No termcap entry for %s.\n", name);
            exit(1);
            break;

    }

    eeolseq = tgetstr("ce", &area);
    cmseq   = tgetstr("cm", &area);
    clearseq = tgetstr("cl", &area);
    BC   = tgetstr("bc", &area);
    UP   = tgetstr("up", &area);

}

eeol() {

    tputs(eeolseq, 0, putchar);

}

clear_screen() {

    tputs(clearseq, 0, putchar);

}

movecur(row, col)
int row, col;
{

    tputs(tgoto(cmseq, col, row), 0, putchar);

}

#ifdef sysV
struct termio old_term;
struct termio new_term;
#else
struct sgttyb old_term;
struct sgttyb new_term;
#endif

/*
 *  Set terminal to CBREAK and NOECHO.
 */
set_term() {
    static int  first = 1;

    if(first) {
#ifdef sysV
	ioctl(0, TCGETA, &old_term);
	new_term = old_term;
#else
        gtty(0, &old_term);
        gtty(0, &new_term);
#endif

#ifdef sysV
	new_term.c_cc[VMIN] = 6;
	new_term.c_cc[VTIME] = 1;
	new_term.c_lflag &= ~(ECHO|ICANON);
	new_term.c_oflag &= ~(TAB3);	/* HMS per bug report */
	ospeed = new_term.c_cflag & CBAUD;

#else
        new_term.sg_flags &= ~(ECHO | XTABS); /* | CRMOD); */
        new_term.sg_flags |= CBREAK;
	
	ospeed = new_term.sg_ospeed;

#endif
        first = 0;
    }

#ifdef sysV
	ioctl(0, TCSETA, &new_term);
#else
    stty(0, &new_term);
#endif

}

/*
 *  Reset the terminal to normal mode.
 */
reset_term() {

#ifdef sysV
	ioctl(0, TCSETA, &old_term);
#else
    stty(0, &old_term);
#endif

}
