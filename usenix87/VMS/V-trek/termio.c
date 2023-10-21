/* terminal I/O */

#include "vtrek.h"
#include curses
#include descrip
#include iodef
extern short chan;

/* get a character from the terminal */
get_a_char()
{
char getchartt();

	return ((int)(getchartt() & 0177));
}

/* initialize the termimal mode */
terminit()
{
ttopen();
initscr();
crmode();
noecho();
nonl();
}

/* reset the terminal mode */
termreset()
{
endwin();
}
char getchartt()
        {
        int status;
        short iosb[4];
        char buf[1];
        status = sys$qiow(0,chan,IO$_READLBLK | IO$M_NOECHO,
                iosb,0,0,buf,1,32767,0,0,0,0);
        if (status != 1)
                LIB$STOP(status);
        if (iosb[0] != 1)
                LIB$STOP(iosb[0]);
        if (buf[0] == '\r')
                buf[0] = '\n';  /* emulate UNIX */
        return(buf[0]);
        }

/*
 * returns a channel
 */

 ttopen()
        {
        register int status;
        struct dsc$descriptor dev_name;

        dev_name.dsc$w_length = 4;
        dev_name.dsc$a_pointer = "TT:";
        dev_name.dsc$b_class = DSC$K_CLASS_S;
        dev_name.dsc$b_dtype = DSC$K_DTYPE_T;
        status = sys$assign(&dev_name,&chan,0,0,0);
        if (status != 1)
                LIB$STOP(status);
        return;
        }




/* write a character */
putch(ch)
int ch;
{
	insch(ch);
	refresh();
}

/* move cursor */
moveyx(ypos,xpos)
int ypos,xpos;
{
move(ypos-1,xpos-1);
refresh();
}

/* clear screen */
cls()
{
	clear();
	refresh();
}
