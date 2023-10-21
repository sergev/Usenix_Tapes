/* 
 * alert - client
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>
#include <time.h>
#include <ctype.h>
#include "defs.h"

#define COMMA ','
char buf[512];
char *wheels[] = { "root"};
int  wizardp = 0;

FILE *fin=NULL;			/* for reading fromthe daemon */
FILE *fout=NULL;		/* for writing to the daemon */
extern int errno;
struct sockaddr_in sin = { AF_INET };

int cleanup();
main(argc, argv)
char *argv[];
{
    int s, pid;
    struct servent *sp;
    struct hostent *hp;
    char host[32];

    if ((sp = getservbyname("alert", "tcp")) == 0) {
	fprintf(stderr,"alert: /tcp: unknown service\n");
	exit(1);
    }

    /* contact local host */
    if ( gethostname(host,sizeof(host)) < 0) {
	perror("alert: gethostname");
	exit(1);
    }

    hp = gethostbyname(host);
    if (hp == NULL) {
	fprintf(stderr,"Unknown host? \n");
	exit(1);
    }
    s = socket(hp->h_addrtype, SOCK_STREAM, 0, 0);
    if (s < 0) {
	perror("alert/socket");
	exit(2);
    }
    sin.sin_family = hp->h_addrtype;
    if(bind(s, &sin, sizeof (sin)) < 0) {
	perror("alert/bind");
	exit(3);
    } 

#ifdef DEBUG
    printf("hostname = %s\n", hp->h_name);
    printf("hostaddr type = %d\n", hp->h_addrtype);
    printf("hostaddr len  = %d\n", hp->h_length);
    printf("hostaddr -%s-\n",inet_ntoa((int *)(hp->h_addr)));
#endif

    bcopy(hp->h_addr,(char *)&sin.sin_addr, hp->h_length);

    sin.sin_port = sp->s_port;

    if(connect(s, &sin, sizeof(sin)) < 0) {
	perror("alert/connect");
	exit(5);
    } 

    signal(SIGSTOP, cleanup);
    signal(SIGTSTP, cleanup);
    signal(SIGINT, cleanup);
    signal(SIGQUIT, cleanup);
    signal(SIGHUP, cleanup);
      
    doit(s,argc,argv);
}


doit(fd,argc,argv)
int fd,argc;
char *argv[];
{
    char *user,getlogin();
    struct passwd *pwd;
    register int i=0;

    fin = fdopen(fd, "r");
    fout = fdopen(fd, "w");
    if (fin == NULL || fout == NULL ) {
	fprintf(stderr, "alert/doit: fdopen failure?\n");
	exit(1);
    }

    /* who is invoking us */
    user = (char *)getlogin();
    if(user == NULL || strlen(user) == 0) {
	if((pwd = getpwuid(getuid())) == NULL) {
	    fprintf(stderr,"Wowee! Who can you be??\n");
	    exit(1);
	}
	user = pwd->pw_name;
    }

    /* check if wheel */
    i= sizeof(wheels)/sizeof(wheels[0]);
    wizardp = 0;
    while(--i >= 0) {
	if(!strcmp(wheels[i],user)) wizardp = 1;
    }

    /*    parse args 
     *    -u foo,10,bar,20,baz,30  .. user list
     *    -t time msg     .. time alert spec
     */
    if (argc < 2) interactive(user);
    else {
	register int i;
	for(i=1; i< argc; i++) {
	    if(argv[i][0] == '-') {
		switch(argv[i][1]) {
		  case 'C':
		    {
			int newrate=0,j=0;
			char c;
			i++;
			if(!wizardp) 
			    fatal("Nope. Sorry! You lose!\n");
			while(c = argv[i][j]) {
			    if(isdigit(c)) 
			      newrate = (newrate * 10) + (c - '0');
			    else 
			      fatal("Garbage character in new rate\n");
			    j++;
			}
			printf("Setting rate to %d\n", newrate);
			fputc(CONTROL_TUNE, fout);
			fflush(fout);
			if(newrate < 60) newrate = 60;
			fprintf(fout, "%s %d\n", user, newrate);
			fflush(fout);
			fgets(buf,512,fin);
			printf("%s\n", buf);
		    }
		    break;

		  case 'f':
		    print_formats();
		    break;

		  case 'F':
		    if(wizardp) {
			i++;
			printf("Pass on, O' mighty one!\n");
			set_userwatch(user,argv[i],FLUSH_USER);
			break;
		    }
		    else printf("Nope, Sorry! You lose!\n");
		    break;

		  case 'u':
		    i++;
		    set_userwatch(user,argv[i],ADD_USER);
		    break;

		  case 'i':
		    i++;
		    set_userwatch(user,argv[i], DELETE_USER);
		    break;

		  case 'D':
		    {
			int id;
			if(!wizardp) break;
			i++;
			sscanf(argv[i],"%d", &id); 
			delete_time_alarm(user,id,1);
		    }
		    break;
		
		  case 'd':
		    {
			int id;
			i++;
			sscanf(argv[i],"%d", &id); getchar();
			delete_time_alarm(user,id,0);
		    }
		    break;

		  case 's':
		    show_time_alarms(user,0);
		    break;

		  case 'w':
		    print_status(user,0);
		    break;

		  case 'A':
		    if(!wizardp) break;
		    show_time_alarms(user,1);
		    break;

		  case 'S':
		    print_status(user,1);
		    break;

		  default:
		    usage();
		    break;
		}
	    }
	    else usage();
	}
    }
    cleanup();
}

fatal(msg)
char *msg;
{
    printf("Error: %s", msg);
    cleanup();
}

cleanup()
{
    /* die gracefully */
    fputc(CONTROL_QUIT, fout);
    fflush(fout);
    fclose(fout);
    exit(0);
}

print_formats()
{
    printf("\nThe following formats are reasonable for specifying times\n");
    printf("\toct 19, 1986 \n");
    printf("\toct 19, 1986 12:43 \n");
    printf("\toct 19, 1986 12:43:44 \n");
    printf("\toct 19  1986 ... \n");
    printf("\t10/19/86 12:43:44 \n");
    printf("\t19-OCT-86 12:43:44 \n");
    printf("\toct. 19, 86 1:44 pm est \n");
    printf("\t861019124333 \n\n");
}

usage()
{
    printf("\t Usage: \n");
    printf("\t -u user1,time1,user2,time2 ...\n");
    printf("\t\twatch user1,user2 etc. times are optional\n");
    printf("\t -i user1,user2 ... stop watching users\n");
    printf("\t -w show status of the alert daemon\n");
    printf("\t -f print all formats acceptable for timestrings\n");
    printf("\t -s show alerts that you have set\n");
    printf("\t -d <id> delete alert with id <id>\n");
    if(wizardp) {
	printf("\t -C time in secs, change daemon's rate\n");
	printf("\t -F user1,user2 ... flush these users\n");
	printf("\t -S show status of alert daemon\n");
	printf("\t -A show all time alarms\n");
	printf("\t -D <id> delete any alarm\n");
    }
    cleanup();
}

interactive(user)
char *user;
{
    char cmd[32];
    char *temp;
    int  quit =0;
    printf("\nAlert Client Program - Version %s (%s)\n", 
	   VERSION, SYSNAME);
    printf("Hello, %s!\n\n",user);

    do {
	printf("%s> ", (wizardp==1) ? "Control":
	       "Alert");
	gets(cmd);
	temp = cmd;
	while(*temp == ' ' || *temp == '\t') temp++;
	switch(*temp) {
	  case '?':
	  case 'h':
	    printf("\tImplemented Commands\n");
	    if(wizardp) {
		printf("\tA\tshow all time alarms\n");
		printf("\tC\tchange daemon clock rate\n");
		printf("\tD\tdelete any time alarm\n");
		printf("\tF\tflush user who is watching others\n");
		printf("\tS\tshow overall state of alert daemon\n");
	    }
	    printf("\t?\tprint this help message\n");
	    printf("\tw\tprint status of alert daemon\n");
	    printf("\tf\tshow formats possible for times\n");
	    printf("\ti\tinhibit watching for a user\n");
	    printf("\tq\tquit alert program\n");
	    printf("\tt\tset alert at specified time\n");
	    printf("\td\tdelete alert set at some time\n");
	    printf("\ts\tshow alerts that you have set up\n");
	    printf("\tu\twatch specified users\n");
	    break;
	    
	  case 'D':
	    {
		int id;
		printf("Id of alarm to delete: ");
		scanf("%d", &id); getchar();
		delete_time_alarm(user,id,1);
	    }
	    break;
		
	  case 'd':
	    {
		int id;
		printf("Id of alarm to delete: ");
		scanf("%d", &id); getchar();
		delete_time_alarm(user,id,0);
	    }
	    break;
	    
	  case 's':
	    show_time_alarms(user,0);
	    break;

	  case 'A':
	    if(!wizardp) break;
	    show_time_alarms(user,1);
	    break;
		
	  case 'C':
	    {
		int newtime;
		if(!wizardp) break;
		printf("New clock rate in seconds (min 60): ");
		scanf("%d", &newtime); getchar();
		fputc(CONTROL_TUNE, fout);
		fflush(fout);
		if(newtime < 60) newtime = 60;
		fprintf(fout,"%s %d\n",user,newtime);
		fflush(fout);
		fgets(buf, 512, fin);
		printf("%s\n", buf);
	    }
	    break;
		
	  case 'F':
	    {
		char userbuf[127];
		char yorn[20];
		if(!wizardp) break;
		printf("Others will be annoyed if you do this to them\n");
		printf("Continue? (y or n) ");
		gets(yorn);
		if(yorn[0] == 'y' || yorn[0] == 'Y') {
		    printf("Userlist to flush: ");
		    gets(userbuf);
		    set_userwatch(user,userbuf, FLUSH_USER);
		    printf("\n");
		}
		else break;
	    }
	    break;

	  case 'i':
	    {
		char userbuf[127];
		printf("Userlist: ");
		gets(userbuf);
		if(!strcmp(userbuf, "")) break;
		set_userwatch(user,userbuf, DELETE_USER);
		printf("\n");
	    }
	    break;
	    
	  case 'w':
	    print_status(user,0);
	    break;

	  case 'f':
	    print_formats();
	    break;

	  case 'S':
	    print_status(user,1);
	    break;
	    
	  case 't':
	    {
		char timebuf[80];
		char msgbuf[127];
		char *msgptr=msgbuf;
		char cmd[20];
		char *cp,*gdate();
		struct tm tm;
		long tim;

		printf("Time  : ");
		fflush(stdout);
		gets(timebuf);
		if((cp = gdate(timebuf,&tm)) != NULL) {
		    printf("Error: %s\n", cp);
		    printf("Use the \"f\" command to see acceptable formats\n");
		    break;
		}
		tim = totime(&tm);
		printf("Msg   : "); 
		fflush(stdout);
		gets(msgbuf);
		fputc(ADD_TIME_ALARM,fout);
		fflush(fout);
		fprintf(fout,"%s %ld\n", user, tim);
		fflush(fout);
		while(*msgptr) {
		    fputc(*msgptr,fout);
		    fflush(fout);
		    msgptr++;
		}
		fputc('\0',fout);
		fflush(fout);
		fgets(buf,512,fin);
		printf("%s", buf);
	    }
	    break;

	  case 'u':
	    {
		char userbuf[127];
		printf("Specify username,idletime,username,idletime...\n");
		printf("Userlist: "); 
		fflush(stdout);
		gets(userbuf);
		set_userwatch(user,userbuf, ADD_USER);
	    }
	    break;

	  case 'q':
	    quit = 1;
	    break;

	  default:
	    printf("Please try a '?' or 'h' for help\n");
	    break;
	}
    } while (!quit);

    fputc(CONTROL_QUIT_INTERACTIVE, fout);
    fprintf(fout,"Bye Bye, %s.\n", user);
    fflush(fout);
    fgets(buf, 512, fin);
    printf("%s", buf);
}

print_status(user,allp)
char *user;
int allp;
{
    register int quit=0;
    if(!allp) fputc(CONTROL_STATUS, fout);
    else {
	if(!wizardp) return;
	fputc(OVERALL_STATUS, fout);
    }
    fflush(fout);
    fprintf(fout,"%s\n",user);
    fflush(fout);
    buf[0] = '\0';
    while (!quit) {
	fgets(buf, 512, fin);
	if(!strncmp(buf, "QUIT", 4)) quit = 1;
	else printf("%s",buf);
    }
}

delete_time_alarm(user,id,anyp)
char *user;
int id;
int anyp;
{
    if(anyp == 1) fputc(DELETE_ANY_ALARM,fout);
    else fputc(DELETE_TIME_ALARM,fout);
    fflush(fout);
    fprintf(fout, "%s %d\n", user,id);
    fflush(fout);
    fgets(buf,512,fin);
    printf("%s",buf);
}

show_time_alarms(user,all)
char *user;
int all;
{
    int quit=0;
    if(all == 1) fputc(SHOW_ALL_ALARMS,fout);
    else fputc(SHOW_TIME_ALARMS,fout);
    fflush(fout);
    fprintf(fout,"%s\n", user);
    fflush(fout);
    fgets(buf, 512,fin);
    printf("%s",buf);
    while (!quit) {
	fgets(buf, 512, fin);
	if(!strncmp(buf, "QUIT", 4)) quit = 1;
	else printf("%s",buf);
    }
}

set_userwatch(user,users,opcode)
char *user;
char *users;
unsigned char opcode;
{
    struct passwd *pwd;
    register char temp[20];
    char buf[512];
    register unsigned char request;
    register char c;
    register int i;
    register int idle=0;
    while(*users) {
	i = idle = 0;
	while((c = *users) == ' ' || c == '\t') users++;
	while(isalpha(c) && (c != COMMA || c != '\0')) {
	    temp[i++] = c;
	    c = *++users;
	}
	temp[i] = '\0';
	/* look ahead one char */
	if(opcode == ADD_USER) {
	    if(isdigit(*(users+1))) c = *++users;

	    while(isdigit(c) && (c != COMMA || c != '\0')) {
		idle = (idle*10) + (c - '0');
		c = *++users;
	    }
	}

	if((pwd = getpwnam(temp)) == NULL) {
	    printf("Error: User %s is unknown?\n", temp);
	    goto check;
	}
	else {
	    /* by now everthing should be ok */
	    request = opcode;
	    fputc(request,fout);
	    fflush(fout);
	    fprintf(fout, "%s %s %d\n", temp, user, idle);
	    fflush(fout);
	    fgets(buf, 512, fin); 
	    printf("%s", buf); 
	}
	    
      check:
	if(c != COMMA && c != '\0') {
	    printf("%o (octal) is not a valid delimiter?\n", c);
	}
	if(c != '\0') users++;
    }
}

