/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* This program implements a uucico type file transfer and remote 
execution type protocol. 
*/
#include "dcp.h"
/*<FF>*/
 int              pktsize;                /* packet size for pro */
 int              flog;                   /* system log file */
 int              fw;                     /* cfile pointer */
 int              fpr,fpw;                /* comm dev pointer */
 char			  state;					/*system state*/
 char             cfile[80];              /* work file pointer */
 int              remote;                 /* -1 means we're remote*/
 int              debug;                  /* debugging level */
 int              msgtime;                /* timout setting */
 char             fromfile[132];
 char             tofile[132];
 int              fp;                     /* current disk file ptr */
 int              size;                   /* nbytes in buff */
 int              fsys;
 char             tty[20];
 char             myname[20];
 char             username[20];
 char             spooldir[80];
 char             rmtname[20];
 char             cctime[20];
 char             device[20];
 char             type[10];
 char             speed[10];
 char             proto[5];
 char             loginseq[132];
 unsigned int     checksum();

/*<FF>*/
/* usage dcp (master/slave(D)) (debug level 0=none(D)) (system name(MYNAME))*/
/* (console device)       defaults come from "userfile"                   */
main(argc,argv)
int argc;
char *argv[];
{
int ftmp;
char line[132];

flog=creat(SYSLOG,0775);
close(flog); flog=open(SYSLOG,2);

remote=TRUE;
debug=3;        
fp = -1;
fw = -1;

ftmp=open(USERFILE,0);
getline(ftmp,line);
sscanf(line,"%s %s %s",myname,username);
getline(ftmp,line);
sscanf(line,"%s %s",tty,speed);
strcpy(spooldir,".");  /* default spooldir is current dir */
/* last line of userfile is meant to be this. modify if you wish */
close(ftmp);

if(argc > 1 && strcmp(argv[1],"master") == 0) remote=FALSE;
if(argc > 2) sscanf(argv[2],"%d",&debug);
if(argc > 3) strcpy(myname,argv[3]);
if(argc > 4) strcpy(tty,argv[4]);

if(! remote) 
{
if((fsys=open(SYSTEMS,0)) ==-1) return(FALSE);
state='I';
while(TRUE)
{
if(debug>0) printmsg("Mstate = %c",state);
switch(state)
{
case 'I': state=getsystem();    break;
case 'S': state=callup();       break;
case 'P': state=startup();      break;
case 'D': state=master();       break;
case 'Y': state=sysend();       break;
case 'G': state='I';            break;
}
if(state=='A') break;
}
close(fsys);
}
else
{
if(openline(tty,speed) == -1) return(FALSE);
state='L';
while(TRUE)
{
if(debug > 0) printmsg("Sstate = %c",state);
switch(state)
{
case 'L':state=login();     break;
case 'I':state=startup();   break;
case 'R':state=slave();     break;
case 'Y':state=sysend();    break;
}
if(state=='A') break;
}
closeline();
}

if(dcxqt()) if(debug) printmsg("ERROR in DCXQT");
	/* scan and process any recieved files */


close(flog);
}

/*<FF>*/
/*
**
**
**master
**
**
*/
master()
{
state='I';
while(TRUE)
{
if(debug>1) printmsg("Top level state (master mode) %c",state);
switch(state)
{
case 'I':state=sinit();      break;
case 'B':state=scandir();    break;
case 'S':state=send();       break;
case 'Q':state=sbreak();     break;
case 'G':state=receive();    break;
case 'C':state='Y';          break;
case 'Y':state=endp();       break;
case 'P':return('Y');
case 'A':return('A');
default:return('A');
}
}
}

/*<FF>*/
/*
**
**
**slave
**
**
*/
slave()
{
state='I';
while(TRUE)
{
if(debug>1) printmsg("Top level state (slave mode) %c",state);
switch(state)
{
case 'I':state=rinit();      break;
case 'F':state=receive();    break;
case 'C':state=schkdir();    break;
case 'T':state='B';          break;
case 'B':state=scandir();    break;
case 'S':state=send();       break;
case 'Q':state=sbreak();     break;
case 'G':return('Y');
case 'Y':state=endp();       break;
case 'P':return('Y');
case 'A':return('A');
default:return('A');
}
}
}

/*<FF>*/
/*
 *  r e c e i v e
 *
 *  This is the state table switcher for receiving files.
 */

receive()
{

state = 'F';/* Receive-Init is the start state */

while(TRUE)
{
if (debug > 2) printmsg(" receive state: %c",state);
switch(state)/* Do until done */
{
case 'F':state = rfile(); break; /* Receive-File */
case 'D':state = rdata(); break; /* Receive-Data */
case 'C':return('C');/* Complete state */
case 'A':return('Y');/* "Abort" state */
default:return('Y');
}
}
}

/*<FF>*/
/*
 *  s e n d 
 *
 *  Sendsw is the state table switcher for sending files.  It loops until
 *  either it finishes, or an error is encountered.  The routines called
 *  by sendsw are responsible for changing the state.
 *
 */
send()
{
fp = -1;                /* reset file getter/opener */
state = 'F';/* Send initiate is the start state */
while(TRUE)/* Do this as long as necessary */
{
if (debug > 2) printmsg("send state: %c",state);
switch(state)
{
case 'F':state = sfile();  break; /* Send-File */
case 'D':state = sdata();  break; /* Send-Data */
case 'Z':state = seof();  break; /* Send-End-of-File */
case 'B':return ('B'); /* Complete */
case 'A':return ('Y'); /* "Abort" */
default:return ('Y'); /* Unknown, fail */
}
}
}

/*<FF>*/
/*
 *
 *      login (for slave in PC mode)
 * Real dumb login handshake
*/
login()
{
char logmsg[132];
#ifdef PC
lretry:
msgtime=9999;
rmsg(logmsg,0); /* wait for a <CR> or <NL> */
msgtime=2*MSGTIME;
wmsg("Username:",0);
rmsg(logmsg,0);
if(debug > 0) printmsg("Username = %s",logmsg);
wmsg("Password:",0);
rmsg(logmsg,0);
if(debug > 0) printmsg("Password = %s",logmsg);
if(strcmp(logmsg,"uucp") != 0) goto lretry;
#endif
return('I');
}
