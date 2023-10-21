/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* mail handler like unix "rmail".  */
#include <stdio.h>
#include "dcps.h"
/*#define LCLMAILER /**/   /* define it if you have a local mail handler */
/* high modified 8/5/86 for fixed spool directory usage */
/* 8/5/86 mod for PC */
/* usage: rmail remotesystem!remoteuser user mysystem   */
/* rmail a!b <NL> takes user,mysystem from "userfile" */
/* rmail<NL> reads the local file "mboxuser", <NL> to see next msg */
/* This obviously can be improved upon easily. RH Lamb */
/* rmail looks for "From " lines in mbox for delimiting */
/*#define DATETIME        "dow mo day time year"        */
#ifndef DG		/* PC VMS file munging */
#define FPATTERN 		"%c%.4s%03d"
#else			/* unix type files */
#define FPATTERN		"%c.%.7s%03d"
#endif
#define PC_CREAT (0775)
main(argc,argv)
int argc;
char *argv[];
{
int i,ii,seq,fcfile,fxfile,fdfile,flg,fpip;
char cfile[40],dfile[40],xfile[40],rmtname[40],rmtuser[80],myname[40];
char rmtdfile[40],rmtxfile[40],user[40],tmp[132],spooldir[132],datetime[50];
long temptime;
flg = 0;
fcfile=open(USERFILE,0);
rmsg(fcfile,tmp);
sscanf(tmp,"%s %s ",myname,user);
rmsg(fcfile,tmp);
rmsg(fcfile,tmp);
sscanf(tmp,"%s ",spooldir);
close(fcfile);
if(argc <= 1 ) goto readmbox;
if(argc > 2 ) strcpy(user,argv[2]);
if(argc > 3 ) strcpy(myname,argv[3]);
if((i=mindex(argv[1],'!')) < 0) flg = 1;
/*if(mindex(argv[1],'@') > -1) flg=2; */
/*un-comment it if you want ARPANET to handle ALL "@" recipients */
/* else "!"'s will be processed first.*/
/* till we figure out the rest of the message format do !'s first */
time(&temptime);
sprintf(datetime,"%s",ctime(&temptime)); ii=strlen(datetime);
datetime[ii-1] = '\0'; sprintf(&datetime[ii-1]," EDT");
seq=random(); /* pick the time as a random file seq num and take our chances*/
if(flg==0)  /* has only !'s in it --->UUCP */
{
strncpy(rmtname,argv[1],i);
rmtname[i] = '\0';
strcpy(rmtuser,&argv[1][i+1]);
sprintf(dfile,FPATTERN,'D',rmtname,seq);
sprintf(rmtdfile,"D.%.7s%03d",rmtname,seq);
fdfile=creat(dfile,PC_CREAT);
}
if(flg)    /* else pass it along to --->ARPANET,and Local mailer */
{
/********* MAKE the data file for your local mail handler *****/
#ifdef  LCLMAILER   /** pass it thru **/ 
/*set up a local file with the right header info for local mailer */
sprintf(dfile,":udd:uucp:mail:uucp%03d",seq);
fdfile=creat(dfile,PC_CREAT);
sprintf(tmp,"To:%s\n\n",argv[1]);
wmsg(fdfile,tmp);
#else    /** final destination **/ /*put it in an mbox */
sprintf(dfile,"Mbox%.4s",user);
if((fdfile=open(dfile,1)) < 0) fdfile=creat(dfile,PC_CREAT);
lseek(fdfile,0L,2);
sprintf(tmp,"From %s %s remote from %s\n",user,datetime,myname);
wmsg(fdfile,tmp);
#endif
/***************************************************************/
}
sprintf(tmp,"Received: by %s on %s\n",myname,datetime);
wmsg(fdfile,tmp);
while(-1)
{
        if(rmsg(0,tmp) == 0) break;     /* EOF */
#ifndef VMS
        if(tmp[0] == 0x1a ) break;      /* ctrl-Z */
#endif
        if(tmp[0] == '.' && tmp[1] == '\n')  break;
        if(strncmp(tmp,"From ",5) == 0) wmsg(fdfile,">");
        wmsg(fdfile,tmp);
}
/*wmsg(fdfile,"\n\n");*/
close(fdfile);
if(flg) {
#ifdef  LCLMAILER
/******* PUT your local mail handler here *****/
sprintf(tmp,"control :util:smile:mailer:mail$ uucp%03d uucp",seq);
system(tmp); /* this ones for our DG mailer */
/***********************************************/
#endif
}
if(flg) return(0); /* if ARPA or Local or final dest Im finised */
/***************************************/
sprintf(xfile,FPATTERN,'B',myname,seq);
fxfile=creat(xfile,PC_CREAT);
sprintf(tmp,"U %s %s\n",user,myname);
wmsg(fxfile,tmp);
sprintf(tmp,"F %s\n",rmtdfile);
wmsg(fxfile,tmp);
sprintf(tmp,"I %s\n",rmtdfile);
wmsg(fxfile,tmp);
sprintf(tmp,"C rmail %s\n",rmtuser);
wmsg(fxfile,tmp);
close(fxfile);
/***************************************/
sprintf(cfile,FPATTERN,'C',rmtname,seq);
fcfile=creat(cfile,PC_CREAT);
sprintf(rmtxfile,"X.%.7s%03d",myname,seq);
sprintf(tmp,"S %s %s %s - %s 0666\n",dfile,rmtdfile,user,dfile);
wmsg(fcfile,tmp);
sprintf(tmp,"S %s %s %s - %s 0666\n",xfile,rmtxfile,user,xfile);
wmsg(fcfile,tmp);
close(fcfile);
/****************************************/
return(0);
/*****************************************/
readmbox:
sprintf(cfile,"Mbox%.4s",user);
if((fcfile=open(cfile,0)) < 0) fcfile=creat(cfile,PC_CREAT);
while(-1)
{
        if(rmsg(fcfile,tmp) == 0) break;        if(strncmp(tmp,"From ",5)==0)
rmsg(0,dfile);
        wmsg(1,tmp);
}
close(fcfile);
return(0);
/***************************************/
}

random()
{ /* get a random number for names and other stuff */
int it;
long tmp;
it = time(&tmp);
it &= 0xff;
return(it);
}

rmsg(fp,msg)
int fp;
char msg[];
{
char c;
c = 'a';
while(c)
{
        if(read(fp,&c,1) == 0) return(0);
        c &= 0x7f;
        *msg = c;
        msg++;
        if(c == '\r' || c == '\n') {*msg = '\0'; c = '\0';}
}
return(-1);
}


wmsg(fp,msg)
int fp;
char msg[];
{
int len;
len=strlen(msg);
write(fp,msg,len);
}

mindex(st,c)
char c,st[];
{
int i;
for(i=0;st[i] != '\0';i++)
{
if(st[i] == c) return(i);
}
return(-1);
}
