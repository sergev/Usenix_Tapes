#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/6/78						*/
/*	Description:	This function updates status information	*/
/*			about percent processor usage. It was taken	*/
/*			from Bill Shannon's stat program.		*/
/*									*/
/************************************************************************/

/************************************************************************/
/*	Include "defines" (C macros).					*/
/************************************************************************/

#include	"df.h"
#include	"gl.h"

/************************************************************************/
/*	Begin status.							*/
/************************************************************************/

status()
BEGIN

register int	i;
long		sum;
static long	sk_old[6]   {0,0,0,0,0,0};

/************************************************************************/
/*	Include external variable declarations.				*/
/************************************************************************/



seek(mem, nl[0].value, 0);
read(mem, sk_cnt, sizeof sk_cnt);

seek(mem,nl[1].value,0);
read(mem,text, sizeof text);
sum = 0;

for(i = 0; i LSS 6; i++ )
	sk_old[i] = sk_cnt[i] - sk_old[i];

for(i = 0; i LSS 6; i++)  
	sum =+ sk_old[i];

for(i = 0; i LSS 6; i++)
	mon_out[i] = 100 * sk_old[i] / sum;

curs(USERDATA,FD,type);
printf(" %3d",mon_out[0]);

curs(SYSTEMDATA,FD,type);
printf(" %3d",mon_out[1]);

curs(INTERDATA,FD,type);
printf(" %3d",mon_out[2]);

curs(SWAPDATA,FD,type);
printf(" %3d",mon_out[3]);

curs(IDLEDATA,FD,type);
printf(" %3d",mon_out[4]);

curs(DIDLEDATA,FD,type);
printf(" %3d",mon_out[5]);

for(i = 0; i LSS 6; i++)
	sk_old[i] = sk_cnt[i];

/************************************************************************/
/*	End status.							*/
/************************************************************************/

END
