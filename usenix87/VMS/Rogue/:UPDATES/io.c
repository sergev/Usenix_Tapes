/*
Hi Rich.  Here is the new io.c I told you about.  Now every character is
returned straight from the keyboard (except ^S and ^Q) so you don't need
the interrupt catching routines.  What you *do* need is to make sure that
every exit from the program is covered by the ttclose() routine, otherwise
you will end up in DCL without echo (you can just do a set term, but why?)
Anyway, I am slowly (after work) making some progress in improving the
game.  First, I wanted to split up the modules more naturally to make future
expansion easier (all potion effects in one module, all scroll effects in
another, etc.).  Anyway, pass any ideas along this way.  BTW, this account
will expire on 6/30/86 and will be replaced by 'donovan'.  My *own*
account is u557676751ea and will be up after 7/1.  Good luck!

                                                    - Mark Nagel

------------------------------- cut here ----------------------------------

*/

#include stsdef.h
#include ssdef.h
#include descrip.h
#include iodef.h
#include ttdef.h
#include tt2def.h
#include jpidef.h


#define	NIBUF	128			/* Size of input buffer */
#define	EFN	0			/* Event flag number */

char	ibuf[NIBUF];			/* Input buffer */
int	nibuf = 0;			/* # of bytes in above */
int	ibufi = 0;			/* Read index */
int	oldmode[3];			/* Old TTY mode bits */
int	newmode[3];			/* New TTY mode bits */
short	iochan;				/* TTY I/O channel */


void ttopen()
{
    struct dsc$descriptor	idsc;
    struct dsc$descriptor	odsc;
    char			oname[40];
    int				iosb[2];
    int				status;

    odsc.dsc$a_pointer = "SYS$INPUT";
    odsc.dsc$w_length  = strlen(odsc.dsc$a_pointer);
    odsc.dsc$b_dtype   = DSC$K_DTYPE_T;
    odsc.dsc$b_class   = DSC$K_CLASS_S;
    idsc.dsc$b_dtype   = DSC$K_DTYPE_T;
    idsc.dsc$b_class   = DSC$K_CLASS_S;
    do {
        idsc.dsc$a_pointer = odsc.dsc$a_pointer;
        idsc.dsc$w_length  = odsc.dsc$w_length;
        odsc.dsc$a_pointer = &oname[0];
        odsc.dsc$w_length  = sizeof(oname);
        status = LIB$SYS_TRNLOG(&idsc, &odsc.dsc$w_length, &odsc);
        if (status != SS$_NORMAL && status != SS$_NOTRAN)
            exit(status);
            if (oname[0] == 0x1B) {
                odsc.dsc$a_pointer += 4;
                odsc.dsc$w_length  -= 4;
        }
    } while (status == SS$_NORMAL);
    status = SYS$ASSIGN(&odsc, &iochan, 0, 0);
    if (status != SS$_NORMAL)
        exit(status);
    status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
                      oldmode, sizeof(oldmode), 0, 0, 0, 0);
    if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
        exit(status);
    newmode[0] = oldmode[0];		/* Only in version 4.	*/
    newmode[1] = oldmode[1] | TT$M_NOECHO | TT$M_TTSYNC;
    newmode[2] = oldmode[2] | TT2$M_PASTHRU;
    status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                      newmode, sizeof(newmode), 0, 0, 0, 0);
    if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
        exit(status);
}


char getchartt()
{
    int status;
    int iosb[2];
    int term[2];

    term[0] = 0;
    term[1] = 0;
    while (ibufi >= nibuf) {
        ibufi = 0;
        status = SYS$QIOW(EFN, iochan, IO$_READLBLK|IO$M_TIMED,
                          iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0);
        if (status != SS$_NORMAL)
            continue;
        status = iosb[0] & 0xFFFF;
        if (status != SS$_NORMAL && status != SS$_TIMEOUT)
            continue;
        nibuf = (iosb[0]>>16) + (iosb[1]>>16);
        if (nibuf == 0) {
            status = SYS$QIOW(EFN, iochan, IO$_READLBLK,
                              iosb, 0, 0, ibuf, 1, 0, term, 0, 0);
        if (status != SS$_NORMAL)
            continue;
        if ((iosb[0]&0xFFFF) != SS$_NORMAL)
            continue;
        nibuf = (iosb[0]>>16) + (iosb[1]>>16);
        }
    }
    if (ibuf[ibufi] == '\r')
        ibuf[ibufi] = '\n';
    return (ibuf[ibufi++]);
}


void ttclose()
{
    int status;
    int iosb[1];

    status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                      oldmode, sizeof(oldmode), 0, 0, 0, 0);
    if (status != SS$_NORMAL || (iosb[0]&0xFFFF) != SS$_NORMAL)
        exit(status);
    status = SYS$DASSGN(iochan);
    if (status != SS$_NORMAL)
        exit(status);
}


spawn_command(command)
char *command;
{

    struct dsc$descriptor descr_command;
               
    /*
     * make the character strings
     */
               
    char the_command[40];

    if((!strcmp("sh",command)) || command[0]=='!' || command[0]=='\0'){

        lib$spawn(0,0,0,0,0,0,0,0,0,0,0,0);
	return;
    }

    sprintf(the_command,"%s",command);
                
    /*
     * Give descriptors the number of printable ascii  
     * characters
     */
                
    descr_command.dsc$w_length=strlen(the_command);
            
    /*
     * Give descriptors the addresses of the strings
     */
            
    descr_command.dsc$a_pointer=the_command;
                    
    /*
     * Define the string descriptor class
     */
             
    descr_command.dsc$b_class=DSC$K_CLASS_S;
                    
    /*
     * Explain that it is an ascii string.
     */
             
    descr_command.dsc$b_dtype=DSC$K_DTYPE_T;


    lib$spawn(&descr_command,0,0,0,0,0,0,0,0,0,0,0);

}


/*
 * this is here because vms curses sucks.
 * the curser was following hidden monsters about.
 */

#include curses
refresh_vms()
{
    static int old_x,old_y;
    static int odd=0;

    getyx(stdscr,old_y,old_x);
    move(LINES -2,COLS-1);
    if (odd == 0){
        odd = 1;
        addch(' ');
    } else {
        odd = 0;
        addch('.');
    } 
    refresh();
    move(old_y,old_x);
}



