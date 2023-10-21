#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sgtty.h>
#include <netdb.h>

int	reapchild();
extern	int errno;
extern  char nowbuf[];
char    startupmsg[128];
/*
 * alertdaemon: does something useful nowadays
 */
int onalarm();			/* repeatedly exec */
struct	sockaddr_in sin = { AF_INET };

char LOGFILE[] = "/usr/src/local/alert/alert.log";
FILE *logfp;
FILE *sockin;
FILE *sockout;
int  daemon_granularity = 60;

main()
{
    int s, pid;
    struct servent *sp;
    long  nowtime;
    sp = getservbyname("alert", "tcp");
    if(sp == 0) {
	fprintf(stderr, "alertd: tcp/alert: unknown service\n");
	exit(1);
    }
    sin.sin_port = sp->s_port;
#ifndef DEBUG
    if(fork()) exit(0);
	for (s = 0; s < 10; s++)
		(void) close(s);
	(void) open("/", O_RDONLY, 0);
	(void) dup2(0, 1);
	(void) dup2(0, 2);
	{ int tt = open("/dev/tty", O_RDWR, 0);
	  if (tt > 0) {
		ioctl(tt, TIOCNOTTY, 0);
		close(tt);
	  }
	}
#endif
  again:
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("alertd: socket");
	sleep(5);
	goto again;
    }
    while (bind(s, (caddr_t)&sin, sizeof (sin)) < 0) {
	perror("alertd: bind");
	sleep(5);
    }
    alertd_initialize();
    signal(SIGCHLD, reapchild);
    signal(SIGALRM,  onalarm);
    onalarm();
    time(&nowtime);
    sprintf(startupmsg,"restarted at %s\n", ctime(&nowtime));
    logerror(startupmsg);
    listen(s, 10);
    for(;;) {
	int s2;
	s2 = accept(s, 0, 0);
	if(s2 < 0) {
	    if(errno == EINTR) continue;
	    perror("alertd: accept");
	    sleep(1);
	    continue;
	}

	/* begin hackery */
	sockin = fdopen(s2,"r");
	sockout = fdopen(s2, "w");
	if(sockin == NULL || sockout == NULL){
	    logerror("couldnt fdopen socket?\n");
	    continue;
	}
	parse_requests();
	close(fileno(sockout));
	fclose(sockin);
	fclose(sockout); 
	close(s2);
    }
}

reapchild()
{
	union wait status;

	while (wait3(&status, WNOHANG, 0) > 0)
		;
}

doit(f)
int f;
{
    int i;
    if ((i = fork()) < 0)
      fatalperror(f, "fork", errno);
    if (i) {
	close(f);
	exit(0); /* parent booogies out */
    }
    dup2(f, 0);
    dup2(f, 1); 
    close(f);
    parse_requests();
    exit(0);
}


logerror(msg)
char *msg;
{
    FILE *logfp;
    if((logfp = fopen(LOGFILE,"a+")) == NULL) return;
    fprintf(logfp,"alertd: %s", msg);
    fflush(logfp);
    fclose(logfp);
}

fatalperror(f, msg, errno)
int f;
char *msg;
int errno;
{
	char buf[BUFSIZ];
	extern char *sys_errlist[];

	(void) sprintf(buf, "%s: %s", msg, sys_errlist[errno]);
	fatal(f, buf);
}


fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "alertd: %s.\n", msg);
	(void) write(f, buf, strlen(buf));
	exit(1);
}


onalarm()
{
    hunt();
    alarm(daemon_granularity);
}
