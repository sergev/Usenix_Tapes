/*
 * uucpanz.c
 *	program to print out stats about uucp usage.
 *
 *	By:
 *	Bradley Smith
 *	Bradley University
 *	uiucdcs!bradley!brad
*/
#include	<stdio.h>
#include	<ctype.h>

char *getfield();

#define	SYSLOG	"/usr/spool/uucp/SYSLOG"
#define	LOGFILE	"/usr/spool/uucp/LOGFILE"
#define	MAXSYS	5	/* maxuim number of systems you talk to
				 * should be made to use malloc but I didn't
				 * oh well
				 */

struct	dayrec {
	int used;	/* used */
	long recv;	/* bytes recv */
	long sent;	/* bytes sent */
	long trecv;	/* seconds spent rec */
	long tsent;	/* seconds spent sending */
};

struct	sys1 {  /* sturct for each system you talk to */
	char sname[9];
	long bbsent;
	long brec;
	long btsent;
	long btrec;
	int suc;
	int fail;
	int ugot;
	int lock;
	int usent;
};

struct	users {
	char *name;	/* login name */
	long bsent;	/* bytes sent */
	long utim;
	struct	users	*nuser;
};

struct	call {
	int times;
	char *cname;
	struct call *ncall;
};

struct	dayrec	dayacc[13] [32];
struct	sys1	sysacc[MAXSYS];
struct	call	*cmd, *tcall();
struct	users	*usage;
struct	users	*tree();
char	*malloc(), *strsave();

long	byt, tim, atol();
int	cmdcount;
long	hour, min,second, hourtmp;
FILE *fpin;

main()
{
	char line[512], field[128], date[10], cx[128],*cp, *c;
	char sysname[9], username[9];
	register int i,j,k;
	int d, m;

/* intialize */
	usage = NULL;
	cmdcount = 0;
	cmd = NULL;
	for(i = 1; i <= 12; i++)
		for(j = 1; j <= 31; j++) {
			dayacc[i][j].recv= 0L;
			dayacc[i][j].used= 0;
			dayacc[i][j].trecv= 0L;
			dayacc[i][j].sent= 0L;
			dayacc[i][j].tsent =0L;
		}
/* lets do SYSLOG first */

	if((fpin = fopen(SYSLOG,"r")) == NULL)
		error("Can't open SYSLOG");

	while(fgets(line,512,fpin) != NULL) {
		/*
		 * lets find the date
		 */
		strcpy(cx,getfield(2,line,' '));
		cp = &cx;
		cp++;	/* puts at first number */
		c = cp;
		cp++;
		if(isdigit(*cp))
			cp++;
		*cp = '\0';
		m = atoi(c);
		cp++;
		c = cp;
		cp++;
		if(isdigit(*cp))
			cp++;
		*cp = '\0';
		d = atoi(c);
		strcpy(sysname, getfield(1,line,' '));
		byt = atol(getfield(6,line,' '));
		tim = atol(getfield(8,line,' '));
		strcpy(username, getfield(0,line,' '));
		strcpy(field,getfield(4,line,' '));

		if(tindex(field,"sent") != -1) { /* ah we are sending stuff */
			for(i = 0;i < MAXSYS;i ++) {
			  if(strlen(sysacc[i].sname) <= 0) {
				strcpy(sysacc[i].sname, sysname);
				sysacc[i].bbsent = byt;
				sysacc[i].btsent = tim;
				break;
			  }
			  else if(strcmp(sysacc[i].sname, sysname) == 0) {
				sysacc[i].bbsent += byt;
				sysacc[i].btsent += tim;
				break;
			  }
			}
			usage = tree(usage, username);
			dayacc[m][d].sent += byt;
			dayacc[m][d].tsent += tim;
			dayacc[m][d].used = 1;
		}
		else { /* recieving stuff */
			dayacc[m][d].recv += byt;
			dayacc[m][d].trecv += tim;
			dayacc[m][d].used = 1;
			for(i=0;i< MAXSYS; i++) {
			  if(strlen(sysacc[i].sname) <= 0) {
				strcpy(sysacc[i].sname, sysname);
				sysacc[i].brec = byt;
				sysacc[i].btrec = tim;
				break;
			  }
			  else if(strcmp(sysacc[i].sname, sysname) == 0) {
				sysacc[i].brec += byt;
				sysacc[i].btrec += tim;
				break;
			  }
			}
		}
	}
	fclose(fpin);

	if((fpin = fopen(LOGFILE,"r")) == NULL )
		error("Can't open LOGFILE");

	while(fgets(line,512,fpin) != NULL) {
		c = getfield(4,line,' ');
		if(strcmp(c,"XQT") == 0) {
			strcpy(field,getfield(1,line,';'));
			field[strlen(field)-4] = '\0';
			cmd = tcall(cmd,field);
		}
		else if(tindex(c,"call") != -1) {
			
			cp = getfield(3,line,' ');
			if(strcmp(cp,"SUCCEEDED") == 0) {
				for(i=0;i< MAXSYS;i++)
				  if(strcmp(sysacc[i].sname,getfield(1,line,' ')) == 0)
					sysacc[i].suc++;
			}
			else if(strcmp(cp,"FAILED") == 0) {
				for(i=0;i< MAXSYS;i++)
				  if(strcmp(sysacc[i].sname,getfield(1,line,' ')) == 0)
					sysacc[i].fail++;
			}
			else if(strcmp(cp,"LOCKED") == 0) {
				for(i=0;i< MAXSYS;i++)
				  if(strcmp(sysacc[i].sname,getfield(1,line,' ')) == 0)
					sysacc[i].lock++;
			}
		}
		cp = getfield(3,line,' ');
		if(strcmp(cp,"REQUEST") == 0) {
			for(i=0;i< MAXSYS;i++)
			  if(strcmp(sysacc[i].sname,getfield(1,line,' ')) == 0)
				sysacc[i].usent++;
		}
		else if(strcmp(cp,"COPY") == 0) {
			for(i=0;i< MAXSYS;i++) {
			  if(strcmp(sysacc[i].sname,getfield(1,line,' ')) == 0)
				sysacc[i].ugot++;
			}
		}
	}
	fclose(fpin);
	printf("UUCP ANALYZER:\n");
	printf("%5sBy system:\n","");
	for(i=0;i < MAXSYS;i++) {
	    if(strlen(sysacc[i].sname) > 0) {
		printf("%10s%s\n", "", sysacc[i].sname);
		hourtmp = sysacc[i].btsent / 60;	/* gives interger min */
		second = sysacc[i].btsent - ( hourtmp * 60); /* seconds */
		hour =  hourtmp / 60;	/* gives integer hour */
		min = hourtmp - ( hour * 60);
		printf("%15ssent       %10ld bytes %5stime ", "", sysacc[i].bbsent,"");
		printf("%02ld:%02ld:%02ld\n", hour, min, second);
		printf("%15srecieved   %10ld bytes %5stime ","",sysacc[i].brec,"");
		hourtmp = sysacc[i].btrec / 60;	/* gives interger min */
		second = sysacc[i].btrec - ( hourtmp * 60); /* seconds */
		hour =  hourtmp / 60;	/* gives integer hour */
		min = hourtmp - ( hour * 60);
		printf("%02ld:%02ld:%02ld\n", hour, min, second);
		printf("%15s# of files %10d sent   %5s %5d recieved\n",
			"",sysacc[i].usent, "",sysacc[i].ugot);
		printf("%15s# of calls %10d suc     %10d fail   %10d lock\n",
			"", sysacc[i].suc, sysacc[i].fail, sysacc[i].lock);
		/* next do total */
		hour = sysacc[i].bbsent - sysacc[i].brec;
		if(hour < 0) {	/* means we rec more */
			hour *= -1;
			printf("%15srecieved %ld bytes more than sent\n",
				"", hour);
		}
		else if(hour > 0)	/* means we sent more */
			printf("%15ssent %ld bytes more that recieved\n",
				"", hour);
		else
			printf("%15ssent the same amount as recieved\n","");
		hourtmp = (sysacc[i].btrec + sysacc[i].btsent)  / 60;
		second = (sysacc[i].btrec + sysacc[i].btsent) 
				- ( hourtmp * 60); /* seconds */
		hour =  hourtmp / 60;	/* gives integer hour */
		min = hourtmp - ( hour * 60);
		printf("%15stotal connect time %02ld:%02ld:%02ld\n",
			"", hour, min, second);
	    }
	}
	printf("\n%5sBy user:\n", "");
	treeprint(usage);
	printf("\n%5sBy commands:\n", "");
	trsort();
	tcallpr(cmd);
	printf("\n%5sBy day:\n","");
	for(i = 1; i <= 12; i++)
		for(j = 1; j <= 31; j++) {
			if(dayacc[i][j].used) {
				hourtmp = dayacc[i][j].trecv / 60;
				second = dayacc[i][j].trecv - ( hourtmp * 60);
				hour =  hourtmp / 60;	/* gives integer hour */
				min = hourtmp - ( hour * 60);
				printf("%5s%2d/%02d ", "", i,j);
				printf("recieved %8ld bytes in ", dayacc[i][j].recv);
				printf("%02ld:%02ld:%02ld/sent %8ld bytes in ",
					hour,min,second, dayacc[i][j].sent);
				hourtmp = dayacc[i][j].tsent / 60;
				second = dayacc[i][j].tsent - ( hourtmp * 60);
				hour =  hourtmp / 60;	/* gives integer hour */
				min = hourtmp - ( hour * 60);
				printf("%02ld:%02ld:%02ld\n", hour,min,second);
			}
		}
	exit(0);
}
error(s)
char *s;
{
	fprintf(stderr,"%s\n", s);
	exit(1);
}
tindex(s,t) /* great function from 'C' book, to lazy to use pointers */
char s[], t[];
{
	register int j,k,i;
	for(i=0;s[i] != '\0'; i++) {
		for(j=i,k=0;t[k] != '\0' && s[j]== t[k]; j++, k++)
			;
		if(t[k] == '\0')
			return(i);
	}
	return(-1);
}
char *strsave(s)	/* save string s somewhere */
char *s;
{
	char *p;

	if((p = malloc(strlen(s)+1)) != NULL)
		strcpy(p,s);
	else {
		error("strsave: out of mem");
	}
	return(p);
}
struct users *tree(p,w)
struct users *p;
char *w;
{
	if(p == NULL) { /* new word */
		p = (struct users *) malloc (sizeof(struct users));
		p->name = strsave(w);
		p->bsent = byt;
		p->utim = tim;
		p->nuser = NULL;
	}
	else if(strcmp(w,p->name) == 0) {
		p->bsent += byt;
		p->utim += tim;
	}
	else {
		p->nuser = tree(p->nuser,w);
	}
	return(p);
}
struct call *tcall(p,w) /* basically same as above */
struct call *p;
char *w;
{
	if(p == NULL) { /* new cmd */
		p = (struct call *) malloc (sizeof(struct call));
		if(p == NULL)
			error("tcall out of Mem");
		p->ncall = NULL;
		p->cname = strsave(w);
		p->times = 1;
		cmdcount++;
	}
	else if(strcmp(w,p->cname) == 0) {
		p->times++;
	}
	else {
		p->ncall = tcall(p->ncall,w);
	}
	return(p);
}
treeprint(p)
struct users *p;
{
	if(p != NULL) {
		printf("%10s%10s ", "", p->name);
		printf("sent %10ld bytes ", p->bsent);
		hourtmp = p->utim /60;
		second = p->utim - ( hourtmp * 60 );
		hour = hourtmp / 60;
		min = hourtmp - (hour * 60);
		printf("%02ld:%02ld:%02ld\n", hour,min,second);
		treeprint(p->nuser);
	}
}
tcallpr(p)
struct call *p;
{
	if(p != NULL ) {
		printf("%10d %s\n", p->times, p->cname);
		tcallpr(p->ncall);
	}
}
trsort()
{
	struct call *q;
	struct call *p;
	register int i, sw, n,m;
	char *c;
	char *d;

loop:
	p = cmd;
	sw = 0;
	for(i=0;i< cmdcount-1; i++) {
		q = p->ncall;
		if(p->times < q->times) { /* switch */
			c = p->cname;
			n = p->times;
			d = q->cname;
			m = q->times;
			p->cname = d;
			p->times = m;
			q->cname = c;
			q->times = n;
			sw = 1;
		}
		p = p->ncall;
	}
	if(sw)
		goto loop;
}
/* my great function, if read this far I commend you */
#define NULLP ""
char *
getfield(field,buffer,separator)
char separator;
char buffer[];
int field;
{
	register int i;
	char *bp, *p, buff[512];
	int sht;
	sht = 0;
	strcpy(buff,buffer);
	p = &buff[0];
	i = 0;
	if((*p == separator) && (field != 0)) {
		field -= 1;
		sht = 1;
	}
	while( i != field) {
		for(++p; *p != separator; p++) {
			if (*p == '\0') {
				return(NULLP);
			}
		}
		i++;
	}
	if(sht)
		field += 1;
	if(field != 0) p++;
	bp =p;
	for (; *p != separator; p++)
		if(*p == '\0')
			return(bp);
	*p = '\0';
	return(bp);
}
