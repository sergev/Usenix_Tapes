/* answer.c -*-update-version-*-
** program to auto answer unix mail with absence notice
** HFVR VERSION=Thu Jan  2 09:13:41 1986
*/

#include 	<stdio.h>
#include	<sys/types.h>
#include	<signal.h>
#include	<ctype.h>
#include	<time.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>

#define equal		!strcmp
#define equaln		!strncmp
#define	TRUE	1
#define	FALSE	0
#define	TOSIZE		512		/* size of buffer to hold dests */
#define MAXLINES	50		/* max nr of from lines */
#define LINELENGTH	256		/* max length from line */

char 	ANSWER[]	= "Subject: Answering Service";
char 	TEMPA[]		= "/usr/tmp/AXXXXX";	/* answer file */
char	TEMPM[]		= "/usr/tmp/MXXXXX";	/* copy of $MAIL file */
char	subject[LINELENGTH];			/* msg subject line */
char	NAME[]		= "answer";
char	VERSION[]	= "Version 0.999";
char	rmtfrom[]	= " remote from ";
char	fwrdmsg[]	= " forwarded by ";
char	from[] 		= "From ";
char	because[80] ;		/* reason for not sending reply */

FILE *f1;	/* input file */
char *lp;	/* pointer for reading file */
char	line[LINELENGTH];	/* line last read from f1 */
FILE *f2;	/* anwer file */
int frcnt;	/* nr of from lines in message */
char	frlist[MAXLINES][LINELENGTH];	/* from lines in message */
int	DEBUG = FALSE;

/* TIME: return pointer to \0 terminated string with YYMMDD hh:mm:ss in it */
char *TIME()
{ extern long time();
  extern struct tm *localtime();
  struct tm *local;
  long seconds;
  static char times[15];

  seconds = time(0);
  local = localtime(&seconds);
  sprintf(times,"%.2d%.2d%.2d %.2d:%.2d:%.2d",local->tm_year
  					     ,local->tm_mon + 1
					     ,local->tm_mday
					     ,local->tm_hour
  					     ,local->tm_min
					     ,local->tm_sec);
  return(times);
}/*TIME*/

#define ERRTEMPA 1
#define ERRTEMPM 2
#define ERRNODIR 3
/* print error messages on sterr and exit*/
error(nr)
int nr;
{
  fprintf(stderr,"%s ERROR : ",TIME());
  switch (nr) {
   case ERRTEMPA : fprintf(stderr,"Cannot create answer TEMP file");
   	           break;
   case ERRTEMPM : fprintf(stderr,"Cannot create copy of mail in TEMPM");
   		   break;
   case ERRNODIR : fprintf(stderr,"$HOME/mail directory must be drwx------");
   		   break;
   default: 	   fprintf(stderr,"Something is wrong");
   	    	   break;
  }
  fprintf(stderr,"\n");
  exit(1);
}/*error*/

/*
 *	sindex looks for an occurance  of the string s2 in the string
 *	s1. If s2 does not ocurr, sindex returns -1. Otherwise, sindex
 *	returns the integer offset from s1 where s2 ocurrs.
 */
int sindex(s1,s2)
	char *s1;
	char *s2;
{
	register char *p1;
	register char *p2;
	register int flag;
	int ii;
	p1 = &s1[0];
	p2 = &s2[0];
	flag = -1;
	for(ii = 0; ; ii++) {
		while(*p1 == *p2) {
			if(flag < 0) flag = ii;
			if(*p1++ == 0) return(flag);
			p2++;
		}
		if(*p2 == 0) return(flag);
		if(flag >= 0) {
			flag = -1;
			p2 = &s2[0];
		}
		if(*s1++ == 0) return(flag);
		p1 = s1;
	}
}/*sindex*/

/* returns pointer to string with sender address in it
** input is frlist[] with lines like:
**
** [>]From USER DATE 				or
** [>]From USER DATE forwarded by WHOCARES	or
** [>]From USER DATE remote from MACHINE
**
** some lines might be duplicated
** we save only the last USER and string together all MACHINEs
** except if the previous machine equals the current one
** forwarded by lines are simply ignored
** address format is mach!mach!....!user or user
** if flg TRUE then it is also printed to stdout
*/
char *frflsh(flg)
{
 static char address[TOSIZE] ;
 char user[80];
 char mach1[80]  ;	/* previous machine name */
 char mach2[80]  ; 	/* current machine name */
 register int i;
 int j;
 register char *p;
 register char *q;
 
 strcpy(address,"");
 strcpy(mach1,"");

 for (i = 0 ; i < frcnt ; i++) { /* examine every From line */
  p = frlist[i] ;
  if ( *p == '>' ) p++;	/* skip leading > */
  if ( equaln(p, from , 5 ) ) {
   p += 5;			/* skip FromSP */
  } else {
   continue; /* does not match FromSP , try next line */
  }

/* get user from line and skip past it */
/* but first skip past spaces and tabs */
  while ( *p == ' ' || *p == '\t' ) p++;
  q = user;
  while ( *p != 10 && *p != '\0' &&  *p != ' ' && *p != '\t' ) {
    *q = *p;
    p++; q++;
  }
  *q = '\0';

/* get machine name */
 strcpy(mach1,mach2);	/* save previous name */

/* see if 'remote from' in line */
 j = sindex(frlist[i], rmtfrom) ; 
 if ( j == -1 ) continue ; /* no so try next line */

 p = &frlist[i][j+sizeof(rmtfrom) -1];	/* skip past remote from */
 while ( *p == ' ' || *p == '\t' ) p++; /* skip spaces */
 q = mach2;
 while ( *p != 10 && *p != '\0' &&  *p != ' ' && *p != '\t' ) {
  *q = *p;
  p++; q++;
 }
 *q = '\0';

/* append to address if different from previous machine mach1 */
 if ( equal(mach1,mach2) ) {
  continue;	/* equal so skip it */
 } else {
  strcat(address,mach2);
  strcat(address,"!");
 }
 }/* for */

 strcat(address,user);
 if (flg) printf("Sender=%s\n",address);
 return(address);
}/*frflsh*/

/*
 *	Determine if line is genuine article.
 *	Should be of 2 forms:
 *		From.*[0-9][0-9]:[0-9][0-9].*
 *			or
 *		>From.*[0-9][0-9]:[0-9][0-9].*
 *
 *	The [0-9] stuff is the date representation by ctime.
 *	Some systems use only the 09:43 part of the time (i.e.
 *	no seconds representation).
 */
Isfrom(line)
register char *line;	/* line to parse */
{
	register char *p;
	extern char *strchr();

	if(equaln(line, from, 5))
	{
		if((p=strchr(line, ':')) != NULL)
		{
colontest:
			if(	isdigit(p[-2]) &&
				isdigit(p[-1]) &&
				isdigit(p[1]) &&
				isdigit(p[2]))      return(1);
		}
		/*
		 *	If more colons, try again.
		 */
		if(p=strchr(++p, ':'))
			goto colontest;
	}
	return(0);
}/*Isfrom*/

/*fillfrom: fills frlist[] with FROM lines and sets frcnt to nr of lines */
int fillfrom()
{ 
  frcnt = 0;
  strcpy(subject,"");

/* skip until From is found */
  while ( lp != NULL && !Isfrom(line)) {
    lp = fgets(line, LINELENGTH, f1);
  }
  if ( NULL == lp ) return(FALSE);

/* copy in first From line */
  strcpy(frlist[frcnt] , line);
  frcnt++;

/* copy in >From lines */
  lp = fgets(line, LINELENGTH, f1);
  while ( lp != NULL && Isfrom(line+1)) {
   strcpy(frlist[frcnt] , line);
   frcnt++;
   lp = fgets(line, LINELENGTH, f1);
  }
  if  ( NULL == lp ) return(FALSE);

/* now look through text until ANSWER or start next message */
  while ( lp != NULL && !Isfrom(line)) {
   line[strlen(line)-1] = '\0';	/* take of CR */
   if (equal(line,ANSWER)) break;
   lp = fgets(line, LINELENGTH, f1);
  }
  strcpy(subject,line);	/* save last line as dummy subject */
  return(TRUE);
}/*fillfrom*/

/* return pointer to string with node name in it */
char *NODE()
{ extern int uname();
  static struct utsname name;

  uname(&name);
  return(name.nodename);
}/*NODE*/

/* make answer file in TEMPA and append $HOME/I_am_out file if exists */
void mkanswer()
{ FILE *f4;
  char buf[1024];
  register int len;

 mktemp(TEMPA);
 unlink(TEMPA);
 f2 = fopen(TEMPA,"w");
 if ( NULL == f2 ) error(ERRTEMPA);
 fprintf(f2,"%s\n",ANSWER);
 fprintf(f2,"\n");
 fprintf(f2,"This is an automatic answering service for Unix mail\n");
 fprintf(f2,"Your mail was received at %s ",TIME());
 fprintf(f2,"(local time on %s),\n",NODE());
 fprintf(f2,"and will be answered by me as soon as I return.\n\n");
 fflush(f2);

/* see if we must add $HOME/I_am_out */
 sprintf(buf,"%s/I_am_out",getenv("HOME"));
 f4 = fopen(buf,"r");
 if ( NULL != f4 ) {
   do {
    len = read (fileno(f4), buf, 1024);
    write(fileno(f2), buf, len);
   } while (len > 0);
 }
 fclose(f4);
 fclose(f2);  
}/*mktemp*/

/* copy mail file to TEMPM so we can process it */
void readmail()
{ char cmd[80];

 mktemp(TEMPM);
 unlink(TEMPM);
 sprintf(cmd,"/bin/mail -p -r >%s 2>/dev/null",TEMPM);
 if (DEBUG) printf("cmd=%s\n",cmd);
 system(cmd);
}/*readmail*/

/* save message and append to stack_of_mail then delete it */
void savemsg()
{ char cmd[80];

 sprintf(cmd,"echo 's %s/mail/stack_of_mail' | /bin/mail -r >/dev/null 2>&1",
             getenv("HOME"));
 if (DEBUG) printf("cmd=%s\n",cmd);
 system(cmd);
}/*savemsg*/

/* check if mail dir has correct access modes x*/
void chkmaildir()
{ char name[80];
 
 sprintf(name,"%s/mail",getenv("HOME"));
 if ( access(name, 07) != 0 ) error(ERRNODIR); /* no rwx */
}/*chkmaildir*/

void init()
{
 chkmaildir();
 mkanswer();
 readmail();
/* open mail and read first line */
 f1 = fopen(TEMPM,"r");
 if ( NULL == f1 ) error(ERRTEMPM);
 lp = fgets(line, LINELENGTH, f1);
}

/* return false if needs no answer */
#define MAXIGNORE	16
int needsanswer(FROM)
char	FROM[];
{ char *ptr;
  FILE *f3;
  char *lp3;
  extern char *strrchr();
  char	ignore[MAXIGNORE];
  char	name[256];
  
/* check subject if answering service then ignore */
 if (equal(subject,ANSWER)) {
  strcpy(because,"message from answering service");
  return(FALSE);
 }

/* ptr must point past last ! */
  ptr = strrchr(FROM,'!');
  if ( NULL == ptr )  {
    ptr = FROM;
  } else {
    ptr++;
  }
  
/* check for daemons */
  if ( (equal (ptr , "**NSC**" ))  ||
       (equaln(ptr , "NUSEND",6))  ||
       (equal (ptr , "uucp"    ))  ||
       (equal (ptr , "**RJE**" ))  ||
       (equal (ptr , "root"    ))  ||
       (equal (ptr , "daemon"  ))
     ) {
   strcpy(because,"it is not a real person");
   return(FALSE);
  }

/* check if it is you */
  if (equal(ptr , getenv("LOGNAME"))) {
   strcpy(because,"it is you");
   return(FALSE);
  }
  
/* check $HOME/mail/.noanswer file */
  sprintf(name,"%s/mail/.noanswer",getenv("HOME"));
  f3 = fopen(name,"r");
  if ( NULL == f3 ) {
   fclose(f3);
   return(TRUE);
  }

/* now read all entries until EOF or found */
  lp3 = fgets(ignore, MAXIGNORE, f3);
  while ( lp3 != NULL) {
   ignore[strlen(ignore)-1] = '\0';	/* take off CR */
   if (equal(ignore,ptr)) {
    fclose(f3);
    strcpy(because,"it appears in $HOME/mail/.noanswer");
    return(FALSE);
   }
   lp3 = fgets(ignore, MAXIGNORE, f3);
  }
  fclose(f3);

  return(TRUE);
}/*needsanswer*/

void finit()
{ 
 unlink(TEMPA);	/* remove TEMPA file */
 unlink(TEMPM);
}/*finit*/

/* send reply to TO */
void reply(TO)
char TO[];
{ char cmd[80];

 printf("%s anwering %s\n",TIME(),TO);
 sprintf(cmd,"/bin/mail '%s' < %s", TO, TEMPA);
 if (DEBUG) {
  printf("UNEXECUTED cmd=%s\n",cmd);
 } else {
  system(cmd);
 }/*fi*/
}/*reply*/

/* do not send answer to TO */
void noreply(TO)
char TO[];
{
 printf("%s No reply to %s because %s\n",TIME(),TO, because);
}/*noreply*/

void usage()
{
  printf("Usage: %s [-d] [-v] [-V] [-H]\n",NAME);
}/*usage*/

/* parse options */
void options(argc,argv)
int	argc;
char	*argv[];
{ register int i;
  for ( i = 1 ; i < argc ; i++ ) {
    if ( argv[i][0] = '-' ) {
       switch (argv[i][1]) {
        case 'H':	usage();
			exit(1);
			break;
	case 'd':	DEBUG = TRUE;
			break;
	case 'v':
	case 'V':	printf("%s %s\n",NAME,VERSION);
			exit(1);
			break;
	default:	usage();
			exit(1);
			break;
       }/*switch*/
    }/*fi*/
  }
}/*options*/

int main(argc,argv)
int	argc;
char	*argv[];
{ char FROM[TOSIZE];

 options(argc,argv);
 init();

/* now for each message */
 while (fillfrom()) {
  savemsg();	/* save and then delete message */
  strcpy(FROM,frflsh(DEBUG));	/* get FROM */
  if (needsanswer(FROM)) {
   reply(FROM);
  } else {
   noreply(FROM);
  }/*fi*/
 }/*while*/

 finit();
 return(0);
}/*main*/
