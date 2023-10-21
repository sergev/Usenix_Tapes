/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* Get the next system, and other support routines  */
#include "dcp.h"
/*<FF>*/
/***************************************************************/
/***            Sub Systems             */
/*
**
**getsystem
** Process an "systems" file entry (like L.sys)
*/
getsystem()
{
int i;
char line[132];
if(getline(fsys,line)) return('A');
sscanf(line,"%s %s %s %s %s %s %s",rmtname,cctime,device,type,speed,proto,loginseq);
if(debug > 2) printmsg("rmt= %s ctm= %s dev= %s",rmtname,cctime,device);
if(debug > 2) printmsg("typ= %s spd= %s pro= %s",type,speed,proto);
if(debug > 2) printmsg("logseq= %s",loginseq);
if(strcmp(cctime,"Slave")==0) return('G');        /*skip this system*/
if(checktime(cctime)) return('G');/* "" wrong time */
return('S');    /*startup this system*/
}

/*<FF>*/
/*
**
**checkname
** Do we know the guy ?
*/
checkname(name)
char name[];
{
int ffd;
char line[132],tmp[20];
if((ffd=open(SYSTEMS,0))== -1) return(-1);
while(getline(ffd,line)==FALSE)
{
        sscanf(line,"%s ",tmp);
        if(debug > 3) printmsg("rmt= %s sys= %s",name,tmp);
        if(strncmp(name,tmp,7)==0) 
                               {
                      close(ffd);
                     return(0); /*OK I like you */
         }
}
close(ffd);
return(-1); /* Who are you ? */
}

/*<FF>*/
/*
**
**checktime
** check if we may make a call at this time 
**------------>to be implemented. Again. Didnt think it crucial
*/
checktime(xtime)
char xtime[];
{
return(0); /* OK go to it */
}

/*<FF>*/
/*
 *
 *      mindex() True if c is in st
 *
*/
mindex(st,c)
char c,st[];
{
int i;
i=0;
for(;;)
{
if(st[i] == '\0') return(-1);
if(st[i] == c) return(i);
i++;
}
}

/*<FF>*/
/*
**
**sysend
** end UUCP session negotiation
*/
sysend()
{
char msg[80];
msg[1] = '\0';
msgtime = 2*MSGTIME;
while(msg[1] != 'O')
{
wmsg("OOOOOO",2);
if(rmsg(msg,2)== -1) goto hang;
}
hang:
        wmsg("OOOOOO",2);
        closeline();
if(! remote) return('I');
return('A');
}


/*<FF>*/
/*
**
**      delay
**
*/
ddelay(dtime)
int dtime;
{
int i,j;
for(i=0;i<dtime;i++) { }
}

/*<FF>*/
/*
**
**      getstring
** Look for a string - useful for send-expect operation
*/
getstring(st)
char *st;
{
int len,j;
char c[40];
len=strlen(st);
c[len]='\0';
len--;
if(debug > 2) printmsg("wanted %s",st);
while(strcmp(st,c) !=0) {
        for(j=0;j<len;j++) c[j]=c[j+1];
        if(sread(&c[len],1,msgtime) < 1) return(-1);
        c[len] &= 0x7f;
        }
if(debug >2) printmsg("got that ","");
return 0;
}


/*<FF>*/
/*
**
**wmsg
** write a ^P type msg to the remote uucp
*/
wmsg(msg,syn)
int syn;
char msg[];
{
int len;
len=strlen(msg);
if(syn==2) swrite("\0\020",2);
swrite(msg,len);
if(syn==2) swrite("\0",1);
}

/*<FF>*/
/*
**
**rmsg
** read a ^P msg from UUCP
*/
rmsg(msg,syn)
int syn;
char msg[];
{
int ii;
char c,cc[5];
/* *msg0;*/
/*msg0 = msg;*/
c = 'a';
if(syn==2) {
	while((c & 0x7f) != '\020') {
		if(sread(cc,1,msgtime) < 1) return(-1); 
		c=cc[0]; /* Dont ask. MSC needs more than a byte to breathe */
/*		printf("Hello im in rmsg c=%x\n",c); */
	}
}
for(ii=0;ii<132 && c ;ii++) {
	if(sread(cc,1,msgtime) < 1) return(-1);
	c = cc[0] & 0x7f;
	if(c == '\r' || c == '\n') c = '\0';
	msg[ii]=c;
	/*if(c == '\020') msg = msg0; */
}
return(strlen(msg));
}


/*<FF>*/
/*
**
**      getline
** get a \n terminated string from whatever
*/
getline(lfp,line)
int lfp;
char line[];
{
int ii;
char c;
c = 'a';
for(ii=0;ii<132 && c;ii++) {
	if(read(lfp,&c,1) == 0) return(TRUE);
	c &= 0xff;
    if(c == '\n') c = '\0';
	line[ii] = c;
}
return(FALSE);
}

/*<FF>*/
/*
 *  p r i n t m s g
 *
 *  Print error message on standard output if not remote.
 */
/*VARARGS1*/
printmsg(fmt, a1, a2, a3, a4, a5)
char * fmt;
{
        int len;
		char msg[256];
        sprintf(msg,fmt, a1, a2, a3, a4, a5);
		msg[256]='\0';
		len=strlen(msg);
		msg[len]='\n'; len++;
		msg[len]='\0';
	    if (! remote) printf("%s",msg);
	    write(flog,msg,len);
}

