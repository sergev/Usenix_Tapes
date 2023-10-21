/*
 * Send terminal messages locally and over the Internet
 */

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <utmp.h>
#include <pwd.h>
#include <sysexits.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define	MSG_SIZE 5000

struct	sockaddr_in  hisctladdr;
struct	utmp utmp, *utmpp;
struct	stat statb;
char    msg[MSG_SIZE], buffer[BUFSIZ];
char	tpath[] = "/dev/";
char	term[sizeof(tpath)+sizeof(utmp.ut_line)];
int	netfd, msglen;

char	*from,			  /* Remote user@host */
	*mytimestr,		  /* time string */
	*myname,		  /* Local username */
	*mytty,			  /* Local TTY */
	myhost[100];		  /* Local host name */

int	alarmed;
int	sendmail;
time_t	mytime;

char	*getlogin(), *index(), *ctime(), *malloc(), *ttyname();
int	nothing();
time_t	time();

main (argc, argv)
int argc;
char **argv;
{
    register char *p, *q;
    int ufd;
    char person[80];
    struct hostent *hp;

    argc--, argv++;
    while (argc > 0 && **argv == '-')
	switch (*++*argv) {

	case 'r':
		argc--, argv++;
		if (argc > 0)
		    from = *argv;
		else
		    goto usage;
		argc--, argv++;
		break;

	case 'd':
		sendmail++;
	case NULL:
		argc--, argv++;
		break;			/* we can ignore this */

	default:
		goto usage;
	}

    if (argc < 1) {
usage:
	fprintf (stderr, "usage: to address[,...,address] [message]\n");
	exit (EX_USAGE);
    }
    signal(SIGALRM, nothing);
    if (stat("/etc/utmp", &statb) < 0) {
	perror("to: /etc/utmp");
	exit(EX_OSFILE);
    }
    utmpp = (struct utmp *) malloc((unsigned)statb.st_size);
    if ((ufd = open("/etc/utmp", 0)) < 0) {
	perror("to: /etc/utmp");
	exit(EX_OSFILE);
    }
    read(ufd, (char *)utmpp, (unsigned) statb.st_size);
    close(ufd);
    if ((myname = getlogin()) == NULL)
	myname =  "UNKNOWN";
    if ((mytty = ttyname(0)) != NULL)
	mytty += 5;
    mytime = time((time_t *) 0);
    mytimestr = ctime(&mytime);
    gethostname(myhost, sizeof myhost);
    if ((hp = gethostbyname(myhost)) != NULL)
	strcpy(myhost, hp->h_name);
    if (argc == 1) {
	fputs ("Msg:\n", stdout);
	p = msg;
	while (p < &msg[MSG_SIZE] && gets(p) != NULL) {
	    p += strlen(p);
	    *p++ = '\r'; *p++ = '\n';
	}
	msglen = p - msg;
	p = *argv;
    } else {
	p = *argv;
	q = msg;
	while (--argc && q < &msg[MSG_SIZE]) {
	    strcpy(q, *++argv);
	    q += strlen(*argv);
	    *q++ = ' ';
	}
	*q++ = '\r';
	*q++ = '\n';
	msglen = q - msg;
    }
    for (; *p;) {
	for(q = person; *p && *p != ',';)
		*q++ = *p++;
	*q = '\0';
	if ((q = index(person, '@')) != NULL) {
	    *q++ = '\0';
	    netsend(person, q);
	} else
	    if (!locsend(person)) {	/* user not logged in */
		if (!sendmail)
			fprintf(stderr, "to: %s: not logged in\n", person);
		exit(EX_UNAVAILABLE);
	    }
    }
    exit(EX_OK);
}

/*
 * Send a message to a remote user
 */
netsend (person, host)
char *person, *host;
{
    register int n;
    char newhost[100];
    struct servent *sp;
    struct hostent *hp;

#ifdef DEBUG
    printf("net user = %s@%s\n",person, host);
#endif
    hp = gethostbyname(host);
    if (hp == NULL) {
	fprintf(stderr, "to: %s: no such host\n", host);
	exit(EX_NOHOST);
    }
    strcpy(newhost, hp->h_name);
    sp = getservbyname("smtp", "tcp");
    if (sp == NULL) {
	fprintf(stderr, "to: smtp/tcp: service not found\n");
	exit(EX_UNAVAILABLE);
    }
    if ((netfd = socket(hp->h_addrtype, SOCK_STREAM, 0, 0)) < 0) {
	perror("to: socket");
	exit(EX_UNAVAILABLE);
    }
    if (bind(netfd, &hisctladdr, sizeof hisctladdr, 0) < 0) {
	perror("to: bind");
	exit(EX_UNAVAILABLE);
    }
    bcopy(hp->h_addr, (caddr_t)&hisctladdr.sin_addr, hp->h_length);
    hisctladdr.sin_family = hp->h_addrtype;
    hisctladdr.sin_port = sp->s_port;
    if (connect(netfd, &hisctladdr, sizeof hisctladdr, 0) < 0) {
        perror("to: connect");
	exit (EX_UNAVAILABLE);
    }
    n = getrply ();
    if (n != 220)		/* Got the site */
	goto error;
    sprintf (buffer, "HELO %s\r\n",myhost);
#ifdef DEBUG
    printf("buffer = %s\n",buffer);
#endif
    write(netfd, buffer, strlen (buffer));
    n = getrply ();
    if (n != 250)
	goto error;
    sprintf (buffer, "SEND FROM:<%s@%s>\r\n", myname, myhost);
#ifdef DEBUG
    printf("buffer = %s\n",buffer);
#endif
    write (netfd, buffer, strlen (buffer));
    n = getrply ();
    if (n != 250)
	goto error;
    sprintf (buffer, "RCPT TO:<%s@%s>\r\n", person, newhost);
#ifdef DEBUG
    printf("buffer = %s\n",buffer);
#endif
    write (netfd, buffer, strlen (buffer));
    n = getrply ();
    if (n != 250)
	goto error;
#ifdef DEBUG
    printf("buffer = DATA\n");
#endif
    write (netfd, "DATA\r\n", 6);
    n = getrply ();
    if (n != 354)
	goto error;
    write (netfd, msg, msglen);
#ifdef DEBUG
    printf(".\n");
#endif
    write (netfd, ".\r\n", 3);
    n = getrply ();
    if (n != 250)
	goto error;
#ifdef DEBUG
    printf("buffer = QUIT\n");
#endif
    write (netfd, "QUIT\r\n",6);
    n = getrply ();
    if (n != 221 && n != 220)
	goto error;
done:
    disconnect ();
    return;
error:
    fprintf(stderr, "to: network error: %s\n", buffer);
    goto done;
}

/*
 * Send a message to a local user
 */
locsend(person)
char   *person;
{
    char tbuf[MSG_SIZE+BUFSIZ];
    int count, found;
    FILE *tf;
    register struct utmp *up;

    count = statb.st_size / sizeof(struct utmp);
    found = 0;
    for (up = utmpp; up < &utmpp[count]; up++) {
	if (up->ut_name[0] == '\0' || strncmp(person, up->ut_name,
							sizeof(utmp.ut_name)))
	    continue;
	strcpy(term, tpath);
	strncat(term, up->ut_line, sizeof(utmp.ut_line));
	alarmed = 0;
	alarm(3);
	if ((tf = fopen(term, "w")) != NULL) {
	    alarm(0);
	    setbuf(tf, tbuf);
	    fprintf(tf, "\r\n\007%s,", from ? from : myname);
	    if (mytty)
		fprintf(tf, " %s,", mytty);
	    fprintf(tf, " %.7s%.4s%.9s\r\n%s",
		&mytimestr[4],
		&mytimestr[20],
		&mytimestr[10],
		msg);
	    alarm(5);
	    fflush(tf);
	    fclose(tf);
	    alarm(0);
	    if (!alarmed)
		found++;
	}
    }
    return(found);
}

/*
 * Disconnect from SMTP server
 */
disconnect ()
{
    write(netfd, "QUIT\r\n", 6);
    close(netfd);
}

/*
 * Read reply code from SMTP server
 */
getrply ()
{
    char temp[BUFSIZ];
    register int i, n;

    while((i = read(netfd, temp, sizeof temp)) == 0)
	;
    temp[i] = NULL;
#ifdef DEBUG
    printf("temp=\"%s\"\n",temp);
#endif
    for (i = 0, n = 0; temp[i] != NULL; i++)
	if (temp[i] != '\n' && temp[i] != '\r')	{
	    buffer[n++] = temp[i];
	}
    buffer[n] = NULL;
    n = 0;
    for (i = 0; i < strlen (buffer); i++) {
	if (isdigit (buffer[i]))
	    n = (n * 10) + (buffer[i] - '0');
	else
	    break;
    }
#ifdef DEBUG
    printf("n=%d temp=\"%s\"\n",n, temp);
#endif
    return (n);
}

nothing()
{
    alarmed++;
}
