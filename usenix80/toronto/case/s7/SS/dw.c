#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/5/78						*/
/*	Description:	This function draws the static characters on the*/
/*			terminal screen. As sometimes the display gets	*/
/*			messed up, hitting cntl B will signal this	*/
/*			function, and redraw the screen.		*/
/*									*/
/************************************************************************/

/************************************************************************/
/*	Include "defines" (C macros).					*/
/************************************************************************/

#include	"df.h"
#include	"gl.h"

/************************************************************************/
/*	Begin draw.							*/
/************************************************************************/

draw()
BEGIN

register int	i,j;
char		buffer[80];

/************************************************************************/
/*	Include external variable declarations.				*/
/************************************************************************/



curs(DATETITLE,FD,type);
printf("Date: ");

curs(TIMETITLE,FD,type);
printf("Time: ");

curs(MESGTITLE,FD,type);
i = open("/etc/sysmesg",0);
if(i LSS 0) 
BEGIN
	printf("Can't open sysmesg");
	return;
END; 
j = read(i,buffer,80);
write(FD,buffer,j-1);
close(i);

curs(STATTITLE,FD,type);
printf("  User:       System:       Inter:       Swap:       Idle:       Didle:  ");

curs(WHO1TITLE,FD,type);
printf("Who");

curs(ADDR1TITLE,FD,type);
printf("  00********10********20********30********40********50********60   Address");

curs(SIZE1TITLE,FD,type);
printf("Size");

curs(STATUS1TITLE,FD,type);
printf("Status");

curs(WHO2TITLE,FD,type);
printf("Who");

curs(ADDR2TITLE,FD,type);
printf("  61********70********80********90********100*******110*******120* Address");

curs(SIZE2TITLE,FD,type);
printf("Size");

curs(STATUS2TITLE,FD,type);
printf("Status");

/************************************************************************/
/*	End draw.							*/
/************************************************************************/

END
