#include <signal.h>
#include <stdio.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include "dispd.h"
#define BELL    007

struct  passwd *getpwuid();
struct sockaddr_in sin;
char text[200];
int s;

main(argc,argv)
char **argv;
{
	int t,n, hupped();
	char    pwbuf[10];
	char    *getpass();
	char    *crypt();
	char    *pw;
	struct passwd *pwd;
	struct hostent *hp;
	struct servent *sp;
	char text[200], host[10], mess[200];
	char *ttyname();
	if (argc < 2) {
		fprintf(stderr,"%s: usage: gone to_where\n", argv[0]);
		exit(1);
	}
	(void) gethostname(host,10); /* 4.2 llib-lc is wrong here !! */
	(void) strcat(mess, "Gone ");
	for (n = 1 ; n < argc; n++) {
		(void) strcat(mess, argv[n]);
		(void) strcat(mess, " ");
	}
	
	/* get password entry */
	pwd = getpwuid(getuid());

	(void) sprintf(text,"%s %s %s",pwd->pw_name,host,mess);
	if ((hp = gethostbyname(HOST)) == 0) {
		perror("cant find host");
		exit(1);
	}
	if ((sp = getservbyname("display", "udp")) == 0) {
		perror("can't find service");
		exit(1);
	}
	if ((s = socket(AF_INET, SOCK_DGRAM,0)) == -1) {
		perror("socket");
		exit(1);
		}
	sin.sin_family = AF_INET;
	sin.sin_port = sp->s_port;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);

	/* signal handling */
	for (t = 1; t <= 26; t++)
		(void) signal(t, SIG_IGN);
	if (sendto(s, text, 100, 0,(struct sockaddr *) &sin, sizeof(sin)) ==  -1) {
		perror("sendto");
		exit(1);
	}
	
	/* Process will die if HUP is received - good for modem lines!   */
		(void) signal(SIGHUP,hupped);
	mess[0] = '\0';
	/* Delete the message */
	(void) sprintf(text,"%s %s %s",pwd->pw_name,host, mess);


	
	/* loop here to wait for correct password */
	while (1) {
		(void) strcpy(pwbuf, getpass("Password:"));
		pw = crypt(pwbuf, pwd->pw_passwd);
		if(strcmp(pw, pwd->pw_passwd) == 0) {			
			if (sendto(s, text, 100, 0, 
			  (struct sockaddr *) &sin, sizeof(sin)) ==  -1) {
				perror("sendto");
			}
		exit(0);
		}
		(void) putchar(BELL);
		(void) fflush(stdout);
	}
}

hupped()
{
	(void) strcat(text, " Hupped");
	if (sendto(s, text, 100, 0, (struct sockaddr *)&sin, sizeof(sin)) ==  -1) {
		perror("sendto");
		exit(1);
	}
}
