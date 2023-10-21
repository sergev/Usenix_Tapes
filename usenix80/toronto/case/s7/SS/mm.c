#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/7/78						*/
/*	Description:	This file handles the task of the memory	*/
/*			display of SS.					*/
/*									*/
/************************************************************************/

/************************************************************************/
/*	Include "defines" (C macros).					*/
/************************************************************************/

#include	"df.h"
#include	"gl.h"

/************************************************************************/
/*	Begin memory.							*/
/************************************************************************/

memory()
BEGIN

register int	i,corestat;
int		im_addr,im_lgth;
register char	c;


/************************************************************************/
/*	Include external variable declarations.				*/
/************************************************************************/



gprocs(proc);

for(i = 0; i LSS 124; i++) screen[i] = sc_stat[i] = ' ';
for(i = 0; i LSS NPROC; i++)
BEGIN
corestat = proc[i].p_flag;
if((corestat = corestat & 01) EQL 0) continue;
if((proc[i].p_stat EQL 5)) continue;

/* in core */
im_addr = proc[i].p_addr >> 5;
im_lgth = proc[i].p_size >> 5;

display(im_addr,im_lgth);

switch(proc[i].p_stat)
BEGIN
	case	1: c = 'S'; break;
	case	2: c = 'W'; break;
	case	3: c = 'R'; break;
	case	4: c = 'I'; break;
	case	5: c = 'Z'; break;
END;


if((proc[i].p_flag & 04) != 0)  c = 'L';
sc_stat[im_addr + im_lgth/2] = c;

name(im_addr + im_lgth/2,proc[i].p_uid);

END;

for(i = 0; i LSS NTEXT; i++)
BEGIN
if(text[i].x_ccount EQL 0) continue;
disp(text[i].x_caddr>>5,text[i].x_size>>5);
sc_stat[(text[i].x_caddr>>5) + (text[i].x_size>>5)/2] ="0123456789ABCDEF"[text[i].x_count];
END;

curs(4,11,FD,type);
write(FD,screen,60);
curs(4,13,FD,type);
write(FD,sc_stat,60);
curs(4,21,FD,type);
write(FD,&screen[60],64);
curs(4,23,FD,type);
write(FD,&sc_stat[60],64);

for(i = 0; i < 124; i++)
BEGIN
	if(sc_stat[i] != ' ') { sc_flag[i] = 1; continue; };
	if(sc_flag[i] == 1  ) {eraze(i); continue; }
END;

/**********************END MEMORY*************************/

END


display(dsp_addr,dsp_lgth)
int dsp_addr,dsp_lgth;
BEGIN

if((dsp_addr + dsp_lgth) > 123 ) return;

if(dsp_lgth <= 1) 
BEGIN
screen[dsp_addr] = 'V';
return;
END;
screen[dsp_addr] = '<';

dsp_lgth--;
dsp_addr++;

while(--dsp_lgth )
screen[dsp_addr++] = '-';

screen[dsp_addr] = '>';

END


disp(dsp_addr,dsp_lgth)
int dsp_addr,dsp_lgth;
BEGIN
if((dsp_addr + dsp_lgth/2) > 123)  return;

if(dsp_lgth <= 1) 
BEGIN
screen[dsp_addr] = 'X';
return;
END;
screen[dsp_addr] = '[';

dsp_lgth--;
dsp_addr++;

while(--dsp_lgth )
screen[dsp_addr++] = '=';

screen[dsp_addr] = ']';

END


name(where,who)
int where,who;
BEGIN

char	buf[80];
	register int  i,j,k;

if(who EQL 99) return;
if(who EQL 0)
BEGIN
unix(who);
return;
END;


i = 0;
while(uname[i].uuid != 0)
BEGIN
	if(uname[i].uuid  EQL who)
	BEGIN
tryagain:	if (where LSS 60 ) k = 5; 
		else BEGIN k = 15; where =- 60; END;
			for(j = 0; j LSS 6; j++)
			BEGIN
				curs(4+where,k+j,FD,type);
				write(FD,&uname[i].umne[j],1);
			END;
			return;

		
	END;
	i++;
END;
if((getpw(who,buf)) != 0) printf("\nbad user id\n");
for(j = 0; j LSS 6; j++)
BEGIN
if(buf[j] == ':') buf[j] = buf[j+1] = buf[j+2] = ' ' ;
uname[i].umne[j] = buf[j];
END;
uname[i].uuid = who;
goto tryagain;

END

unix(who)
int who;
BEGIN
END

eraze(i)
int i;
BEGIN
register int  j,k;
sc_flag[i] = 0;
if(i < 60) k = 5;
else BEGIN  k = 15; i =- 60; END;
for(j = 0; j < 6; j++)
BEGIN
curs(4+i,k+j,FD,type);
write(FD, " ",1);
END;

END
