#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/6/78						*/
/*	Description:	This function is called to get the users	*/
/*			terminal type, which is put in variable type.	*/
/*			The routines which use signal calls (fix and    */
/*			abort), are at the end of this file.		*/
/*									*/
/************************************************************************/

/************************************************************************/
/*	Include "defines" (C macros).					*/
/************************************************************************/

#include	"df.h"
#include	"gl.h"


/************************************************************************/
/*	Begin initialize.						*/
/************************************************************************/

initialize()
BEGIN

register int	i,j;
int		fd;
register char	c;
char		buffer[TTYSIZE];

/************************************************************************/
/*	Include external declared variables.				*/
/************************************************************************/


/************************************************************************/
/*	Get tty name ( last character ).				*/
/************************************************************************/

c = ttyn(FD);
fd = open("/etc/ttys",0);
if(fd LSS 0)
BEGIN
	printf("\ncannot open /etc/ttys\n");
	exit();
END;

j = read(fd,buffer,TTYSIZE);
for(i = 0; i LSS j; i++)
BEGIN
	if(buffer[i] EQL c) break;
END;

type = buffer[i+1] - '0';


if(type NEQ 2 AND type NEQ 4)
BEGIN
	printf("\ncannot operate on this terminal\n");
	exit();
END;

if(type EQL 4) type = 3;

close(fd);

/************************************************************************/
/*	Erase screen and draw static characters.			*/
/************************************************************************/

eras(FD,type);
draw();

/************************************************************************/
/*	Initialize nl for status, and open memory image.		*/
/************************************************************************/

nlist("/unix", nl);
mem = open(coref,0);

/************************************************************************/
/*	End initialize.							*/
/************************************************************************/

END


/************************************************************************/
/*	Begin fix.( called as ISR for cntl b ).				*/
/************************************************************************/

fix()
BEGIN
signal(INTERRUPT,fix);
eras(FD,type);
draw();
END

/************************************************************************/
/*	Begin abort. (called as ISR for cntl c).			*/
/************************************************************************/

abort()
BEGIN
eras(FD,type);
exit();
END

