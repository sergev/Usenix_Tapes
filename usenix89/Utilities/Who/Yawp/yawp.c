#define BSD
/* Program: whom, whos... many names...  call it whatever you like.

   Mods   :  John A. Lemon      7/31/82
	     Seth G. Hawthorne  9/28/82
	     Dan Heller		1984-6  all flags except for -l and 
					general rewriting...
	     (argv@sri-spam.arpa)

    Compile normally, but be aware that if your site has library routines for
    faster access of password or host files, you should use them.

    Also, note that there are #ifdef BDS statements that imply 4.2BSD. If
    you are not on a 4.2 UNIX machine, don't worry.

    see just below here for details on how this program works.
*/

char *err_msg[] = {
    "Usage: %s [-lnrdIiN",
#ifdef BDS
    "h] [-H [host] ",
#endif BDS
    "]\n",
    " -   : print this list\n",
    " -l  : prints the duration of login instead of time of login.\n",
    " -n  : print only account names listed in \".%s\"\n",
    " -r  : print only account names who are ritable\n",
    " -d  : don't substitute names from \".%s\".\n",
    " -I  : print duration of Idle time.\n",
    " -iN : don't print users idling over N mins.\n",
#ifdef BDS
    " -w  : show logins that are ptys but not rlogins (windows; for SUNs).\n",
    " -h  : also print hostnames if remotely logged in.\n",
    " -H [host] : only print remote users [from host] (must be LAST arg)\n",
#endif BDS
    "\n",
    "By default, %s will print a formatted list of users currently logged on\n",
    "the system, the time they logged on, the tty they are using, if they're\n",
    "writable (y,n,g), and a * if user has been idle for more than 15 mins.\n",
    "A file called \".%s\" in your home directory may contain aliases for\n",
    "login names. Alises may have spaces but will be trunctated to 8 chars.\n",
    "\n",
    "sample alias file format :\n",
    "login	alias\n",
    "argv	Dan\n",
    "uucp	mailman\n",
    "root 	The Man\n",
    0 };

#include <stdio.h>
#include <ctype.h>
#include <utmp.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef BDS
#include <netdb.h>
#endif BDS

#define utmpfile  	"/etc/utmp"
#define IDLE	  	5          	/* # mins idle to print a *     */
#define when 		break;case
#define otherwise	break;default

struct utmp buf[50];

struct alias {
    char a_name[80], a_login[80];
    struct alias *a_next;
    } *aliases;

struct P_struct {                           /* Info for formating the output */
    int numthings;
    int maxcol;
    int minrow;
} info;

char *day[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
     *month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
		  "Aug", "Sep", "Oct", "Nov", "Dec" },
     *lentime(), ritable(), *Time(), *index(), *rindex(), *hostopt = "";

int pr_usr(), strcmp(), ucmp(), chk_rite, names, dont_sub, idleopt,
    idle_duration, duration, show_wins;
long now;

#ifdef BDS
struct hostent *hostoptent;
#endif BDS

main(argc, argv)
int argc;
char **argv;
{
    char file[50], *who_file_name = *argv;
    FILE *who_fp;
    struct alias *list;

    if (rindex(who_file_name, '/'))
	who_file_name = 1 + (rindex(who_file_name, '/'));
    if (argc > 1)
	while (*(++argv))
            if (argv[0][0] == '-')
		if (!argv[0][1])
		    do_err(who_file_name);
		else while(*(++argv[0]))
		    switch(*argv[0]) {
			when 'l' :  duration = 1;
			when 'n' :  names = 1;
			when 'r' :  chk_rite = 1;
			when 'd' :  dont_sub = 1;
			when 'I' :  idle_duration = 1, idleopt = 0;
#ifdef BDS
			when 'w' :  show_wins = 1;
			when 'H' :  if (*(++argv))
					hostopt = *argv;
				    else strcpy(hostopt, "*");
				    goto done;
			when 'h' :  hostopt = "-";
#endif BDS
			when 'i' :  while(isdigit(*++argv[0]))
					idleopt = 10 * idleopt + **argv - '0';
				    --argv[0];
				    if (idleopt <= 0)
					printf("bad idle time: %d\n", idleopt),
					exit(1);
			otherwise  : fprintf(stderr,"Bad flag: %c\n",*argv[0]),
				     do_err(who_file_name);
		    }
	    else
		do_err(who_file_name);

done:    /* 'H' flag is last argument! */
    aliases = NULL;
    if (*hostopt && strcmp(hostopt, "-") && strcmp(hostopt, "*"))
	if(!(hostoptent = gethostbyname(hostopt)))
	    printf("%s: no such host.\n", hostopt), hostopt = "-";
	else {
	    hostopt = (char *) malloc (strlen(hostoptent->h_name) + 1);
	    strcpy(hostopt, hostoptent->h_name);
	}
    sprintf(file, "%s/.%s", getenv("HOME"), who_file_name);
    if(!dont_sub && (who_fp = fopen(file, "r")) != NULL) {
	aliases = (struct alias *) malloc(sizeof(struct alias));
	list = aliases;
	while(fscanf(who_fp, "%s %[^\n]s", list->a_login, list->a_name) > 0) {
	    list->a_next = (struct alias *)malloc(sizeof(struct alias));
	    list = list->a_next;
	}
	list = NULL;
    }
    else if(names)
	printf("You need a file called %s for that option.\n", file),
	names= 0;
    info.maxcol = (*hostopt) ? 2 : 3;
    info.minrow = 1;
    info.numthings = getusers(buf);  /* The # of users currently logged on */

    if (info.numthings < 0)             /* Were structures read from utmp? */
        perror(utmpfile), exit(1);
    else {
	char remote[50];
        struct tm *t;
	time(&now);
	t = localtime(&now);
	if (*hostopt) {
	    strcpy(remote, "remotely logged in");
	    if (strcmp(hostopt, "*")) /* host, "-", won't get this far */
		sprintf(&remote[strlen(remote)], " from %s", hostopt);
	}
	printf(" %d %s %s %s%c %s @ %s %s %d at %s\n", 
	    info.numthings,
	    (idleopt) ? "active" : "\b",
	    (chk_rite) ? "ritable" : "\b",
	    (names) ? "friend" : "user",
	    (info.numthings != 1) ? 's' : '\0',
	    (*hostopt && strcmp(hostopt, "-")) ? remote : "\b",
	    day[t->tm_wday], month[t->tm_mon], t->tm_mday, Time(&now));
	if(!info.numthings) exit(0);
	qsort(buf, info.numthings, sizeof (struct utmp), ucmp);
	pr_rows(pr_usr, &info);
	exit(0);
    }
}

/* Function to print entry for a specific user. */
pr_usr(i)
int i;         /* Index of a specific user in the array of user structures */
{
    char *str;
    printf("%-8.8s ", buf[i].ut_name);                 /* Print user's name */

    if (idle_duration)
	str = lentime((long) idletime(buf[i].ut_line) * 60);
    else if (duration)
	str = lentime((long) (now - buf[i].ut_time));
    else
        str = Time(&buf[i].ut_time);
    printf("%5.5s", str);                 /* Print the appropriate time */
    printf("%c ", ritable(buf[i].ut_line)); /* if they're ritable */
    printf("%s", (strcmp(buf[i].ut_line,"console")) ? buf[i].ut_line+3 : "co" );
    printf("%s ", (idletime(buf[i].ut_line) > IDLE) ? "*" : " ");
#ifdef BDS
    if(*hostopt)
	if (*buf[i].ut_host) {
	    char *temp;
	    if (temp = index(buf[i].ut_host, '.'))
		*temp = 0; /* get rid of .arpa or .udu etc.. by nullifying */
	    temp = rindex(buf[i].ut_host, '-'); /* just get important stuff */
	    if (temp && strlen(temp) > 2) /* is there enough info? */
		temp++;
	    else
		temp = buf[i].ut_host;
	    printf("%-6.6s",(strlen(temp) <= 6) ? temp : &temp[strlen(temp)-6]);
	}
	else printf("      ");
#endif BDS
    printf("    ");
}

char
ritable(line)
char *line;
{
    char ttyno[20];
    struct stat tty;
    sprintf(ttyno, "/dev/%s", line);
    if(stat(ttyno, &tty) < 0)
        perror(ttyno);
    return (tty.st_mode & 03) ? 'y' : (tty.st_mode & 030) ? 'g' : 'n';
    /* checking '3' instead of '2' because this may be setuid(0) and
       tty may only be -rw---x--x for security reasons... */
}

/* Figure out time duration in ascii */
char *lentime(t)
long  t;
{
    static char space[32], *s;
    int min, hours;

    hours = t/3600;
    min  = (t%3600)/60;
    s = &space[31];
    *s-- = 0;
    *s-- = min%10 + '0';
    *s-- = min/10 + '0';
    if (hours > 23) {
	hours %= 24;
	*s-- = '?';
    }
    else
	*s-- = ':';
	*s-- = hours % 10 + '0';
	*s = (hours >= 10) ? hours / 10 + '0' : ' ';
    return(s);
}  /* lentime */

getusers(users)
struct utmp users[];
{
    int usr_count = 0;
    struct utmp buf;
    struct alias *list;
    FILE *file;

    if ((file = fopen("/etc/utmp", "r")) < 0)
        perror("/etc/utmp"), exit(1);
    while (fread(&buf , sizeof (struct utmp), 1, file) > 0)
        if ( buf.ut_name[0] ) {
	    if (aliases) {
		list = aliases;
		while(list && strncmp(buf.ut_name, list->a_login, 8))
		    list = list->a_next;
		if(!list && names) continue;
		if(list) strncpy(buf.ut_name, list->a_name, 8);
	    }
	    if (chk_rite && ritable(buf.ut_line) != 'y')
		continue;
	    if (idleopt > 0 && idletime(buf.ut_line) >= idleopt)
		continue;
#ifdef BDS
	    if (!show_wins && !*buf.ut_host && buf.ut_line[3] == 'p')
		continue;
	    if (!*buf.ut_host && !strcmp(hostopt, "*"))
		continue;
	    if (*hostopt && strcmp(hostopt, "-") && strcmp(hostopt, "*") 
			 && (!*buf.ut_line || !same_host(buf.ut_host)))
		    continue;
#endif BDS
	    users[usr_count++] = buf;
	}
    close(file);
    return(usr_count);
}

char *
Time(When)
long *When;
{
    static char time_buf[10];
    struct tm *T;
    T = localtime(When);
    sprintf(time_buf, "%2d:%02d %cm.",
	(T->tm_hour) ? ((T->tm_hour <= 12) ? T->tm_hour : T->tm_hour - 12):12,
	 T->tm_min, (T->tm_hour < 12) ? 'a' : 'p');
    return time_buf;
}

/*
 * Print things out in neat columns
 *  First argument is the routine used to print one item,
 *  next is routine to do spacing at end of line,
 *  last is structure containing total # of items,
 *  min. # of columns to print, and maximum # of rows to print.
 */

pr_rows(pr_usr, e)
int (*pr_usr)();
struct P_struct *e;
{
    int i, row;
    static int numrow;

    if (e->numthings > (e->minrow * e->maxcol)) {   /* fit on minrow rows? */
	numrow = e->numthings / e->maxcol;
	if (e->numthings % e->maxcol)                  /* round up # of rows */
	++numrow;
    }
    else
	numrow = e->minrow;

    row = 1;                             /* keeps track of the row of output */
    i = 0;                                            /* Start w/ 0'th entry */

    while (row <= numrow) {                               /* Do for each row */
	(*pr_usr)(i);
	i += numrow;
	if (i >= e->numthings) {                 /* index > # entries ? */
	    i = row++; /* Starting new row, so set index to appropriate value */
	    putchar('\n');
	}
    }
}

/* Contains the comparison used by qsort to sort the list of users
   alphabetically by name */
ucmp(e1, e2)
struct utmp *e1, *e2;
{
    return (strncmp(e1->ut_name, e2->ut_name, 8));
}  /* ucmp */

idletime(line)
char *line;
{
    struct stat stbuf;
    char ttyname[20];

    sprintf(ttyname, "/dev/%s", line);
    if (stat(ttyname, &stbuf) == -1)
	perror(ttyname), exit(1);
    time(&now);
    return((now - stbuf.st_atime) / 60);
}

do_err(cmd)
char *cmd;
{
    int count = 0;
    while(err_msg[count])
	fprintf(stderr, err_msg[count++], cmd);
    exit(1);
}

#ifdef BDS
same_host(suspect)
char *suspect;
{
    struct hostent *entry;
    char *temp;
    if (temp = index(suspect, '.')) 
	 *temp = 0; /* get rid of .arpa and/or other additions */
    return (entry = gethostbyname(suspect)) && !strcmp(hostopt, entry->h_name);
}
#endif BDS
