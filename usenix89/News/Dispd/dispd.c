#include "dispd.h"
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/file.h>
#include <curses.h>
#include <sys/time.h>
#include <pwd.h>
#include <sys/dir.h>
#include "/usr/src/etc/rwhod/rwhod.h"
#include <strings.h>
#define	WHDRSIZE	(sizeof (wd) - sizeof (wd.wd_we))
#define LARGE 2147483647

struct sgttyb disptty = {
	B9600,B9600,
	0,0,
	ANYP|CRMOD|ECHO
};

struct user_data user_info[50];


int lineno;

/* find if a user is defined as using this system and return a 
 * pointer to his data
 */
struct user_data *user_slot(s)
char *s;
{
	struct user_data *ud;
	lineno = 0;
	ud = user_info;
	while (ud->user_name[0] != '\0') {
		if (strcmp(ud->user_name, s) == 0) return(ud);
		ud++;
		lineno++;
	}
	return((struct user_data *) 0);
}

/* read the USERS file and fill in the initial data */
fill_table()
{
	FILE *us;
	struct user_data *ud;
	struct passwd *pw;
	char name[20];
	ud = user_info;
	if ((us = fopen(USERS,"r")) == NULL) {
		perror(USERS);
		exit(1);
	}
	while (fscanf(us, "%s", ud->user_name) == 1) {
		if ((pw = getpwnam(ud->user_name)) == NULL) {
			fprintf(stderr,"No entry for %s in passwd file\n",ud->user_name);
			exit(1);
		}
		(void) sscanf(pw->pw_gecos,"%[^,]",name);
		if (name[0] == '&') {
			(void) strcpy(ud->real_name, ud->user_name);
			(void) strcat(ud->real_name, &name[1]);
			if (islower(ud->real_name[0])) ud->real_name[0] -=  32;
		}
		else (void) strcpy(ud->real_name, name);
		ud++;
	}
	ud->user_name[0] = '\0';
	(void) fclose(us);
}

WINDOW *user_win, *header_win, *glob_win, *head, *info, *instruct;

int lineno;
main()
{
	FILE *message;
	char mess[200];
	struct tm *tim;
	time_t tnow;
	int s, mf;
	struct servent *sp;
	int fres, ldisc = NTTYDISC, fd;
	struct sockaddr_in sin; 
	char u_name[20], host[20];
	struct user_data *ud;
/* define the terminal type */
	Def_term = DISTYPE;
	My_term = TRUE;

/* get the port number */
	if ((sp = getservbyname("display", "udp")) == 0) {
		perror("can't find service");
		exit(1);
	}
/* read backup data if crashed */
	if ((mf = open(MFILE, O_RDONLY, 0)) > 0) {
		if (read(mf, (char *) user_info, 
		  sizeof(user_info)) < sizeof(user_info)) {
			perror("can't read message file");
			exit(1);
		}
		(void) close(mf);
	}
	(void) fill_table();	
	
/* create and bind the socket */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Cant create socket");
		exit(1);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = sp->s_port;
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
		perror("Cant bind socket");
		exit(1);
	}
/* fork of the child and then die */
#ifndef DEBUG
	if ((fres = fork()) == -1) {
		perror("Cant fork, goodbye!");
		exit(1);
	}
	if (fres) exit(0); /* parent */
	/* child only */

	/* delete contoling terminal */
	if ( (fd = open("/dev/tty", 2)) >= 0 ) {
		(void) ioctl(fd, TIOCNOTTY, (char *) 0);
		(void) close(fd);
	}
	/* close all standard files */
	(void) close(0);
	(void) close(1);
	(void) close(2);

	if (open(DISTTY,O_RDWR) < 0) exit(1); /* stdin */
	if (open(DISTTY,O_RDWR) < 0) exit(1); /* stdout*/
	if (open(DISTTY,O_RDWR) < 0) exit(1); /* stderr*/
	
/* set speed, line disciplin etc */
	(void) ioctl(0,TIOCSETD, (char *) &ldisc); /* line discipline */
	(void) ioctl(0,TIOCSETP,(char *) &disptty); /* speed etc */
#endif

/* start up curses */
	(void) initscr();
	(void) raw();
	(void) noecho();
	
/* define the windows and put initial data into them */
	header_win = newwin(3,80,0,0);
	user_win = newwin(15,80,3,0);
	glob_win = newwin(3,80,19,0);
	instruct = newwin(1,79,23,0);
	head = newwin( 3,80,0,0);
	info = newwin(15,80,3,0);
	leaveok(user_win,TRUE);
	(void) wstandout(header_win);
	(void) mvwprintw(header_win,0,0,TITLE);
	(void) mvwprintw(header_win,0,45, UPDATE);
	(void) mvwprintw(header_win,1,0,"Name");
	(void) mvwprintw(header_win,1,19,"Message");
	(void) wstandend(header_win);
	inst("Hit 1 for login info");
	lineno = 0;
	ud = user_info;
	while (ud->user_name[0] != '\0') {
		(void) mvwprintw(user_win, lineno, 0, "%-10s", ud->real_name);
		(void) mvwprintw(user_win, lineno++, 18, "%-61s", ud->user_message);
		ud++;
	}
/* read the global message file */
	if ((message = fopen(GLOB_MESS, "r")) != NULL) {
		int co;
		co = 3;
		lineno = 0;
		while (co-- > 0 && getline(mess,80,message) == 0) {
			(void) mvwprintw(glob_win,lineno++,0,"%-80s",mess);
		}
		(void) fclose(message);
	}
/* display the windows */
	(void) wrefresh(header_win);
	(void) wrefresh(user_win);
	(void) wmove(glob_win,3,0);
	(void) wrefresh(glob_win);
	
/* loop forever (will hang on select when nothing happens ) */
	while(1) {
		char text[100], termchar;
		int readfds;
		readfds = 1 | 1 << s; 
		/* wait for some input */
		while (select(s+1, &readfds,(int *) 0,(int *) 0,
		  (struct timeval *) 0) == 0);
		if (readfds & 1 << fileno(stdin)) { /* data from the keyboard */
			(void) read(0,&termchar,1);
			switch (termchar) {
				case '\022' : (void) wrefresh(curscr); /* ^R */
				break;
				case '1' : login_info(); /* display login info */
				break;
#ifdef DEBUG
/* nice to have these when debugging */
				case 'x' : endwin(); /* exit - debugging only */
					exit(1);
				case 's' : tstp(); /* stop - debugging only */
					break;
#endif
				case ' ' : break; /* do nothing */
				default: termchar = '\007';
					(void) write(1,&termchar,1); /* Bell for incorrect input */
			}
			inst("Hit 1 for login info");
/* touch since we have windows on top of each other */
			touchwin(header_win);
			touchwin(user_win);
			touchwin(glob_win);
/* rewrite the windows */
			(void) wrefresh(header_win);
			(void) wrefresh(user_win);
			(void) wmove(glob_win,3,0);
			(void) wrefresh(glob_win);
			
			continue;
		}
		if (!(readfds & 1 << s)) continue; /* this shouldn't happen */
		(void) recvfrom(s,text,100, 0, (struct sockaddr *) 0, (int *) 0);
		
/* read the global message */
		(void) werase(glob_win);
		if ((message = fopen(GLOB_MESS, "r")) != NULL) {
			int co;
			co = 3;
			lineno = 0;
			while (co-- > 0) {
				if (getline(mess,80,message) == 0) {
					(void) mvwprintw(glob_win,lineno, 0,"%-80s",mess);
				}
				else (void) mvwprintw(glob_win,lineno,0, "%80s", " ");
				lineno++;
			}
			(void) fclose(message);
		}
		
/* read the message from the (remote) user */
		if (sscanf(text,"%s%s%[^\0]", u_name, host, mess) < 3)
			continue;
		lineno = 0;
		
/* update the time */
		tnow = time((time_t *) 0);
		tim = localtime(&tnow);
		(void) mvwprintw(header_win,0,65,"%02d-%02d-%02d:%02d:%02d\n",
		tim->tm_year,tim->tm_mon + 1, tim->tm_mday,
		tim->tm_hour,tim->tm_min);
		
/* find the user slot */
		ud = user_slot(u_name);
		if ( ud == (struct user_data *) 0) { /* user not defined */
			(void) wrefresh(header_win);
			(void) wrefresh(user_win);
			(void) wmove(glob_win,3,0);
			(void) wrefresh(glob_win);
			continue;
		}
		
/* copy the message and update the window */
		(void) strncpy(ud->user_message,mess,60);
		(void) mvwprintw(user_win, lineno, 0, "%-18s", ud->real_name);
		(void) mvwprintw(user_win, lineno, 18, "%-61s", ud->user_message);
		
/* open the message file and update it. This stores all information
 * so that messages survive a crash - this is messy */
		if ((mf = open(MFILE,O_CREAT|O_RDWR,0770)) <= 0) {
			perror("can't open/create message file");
			exit(1);
		}
		if (write(mf, (char *) user_info, 
		  sizeof(user_info)) < sizeof(user_info)) {
			perror("Can't write message file");
			exit(1);
		}
		
/* refresh the windows */
		(void) close(mf);
		(void) wrefresh(header_win);
		(void) wrefresh(user_win);
		(void) wmove(glob_win,3,0);
		(void) wrefresh(glob_win);
	}
}



/* read a line from a file */
getline(s, n, iop)
char *s;
register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;
	while (--n>0 && (c = getc(iop))>=0 && c != '\n') *cs++ = c;
	if (c<0 && cs==s) return(-1);
	*cs++ = '\0';
	return(0);
}

/* display the login info, gets the info from the same files a rwho, if
 * you want it to work you will have to run /etc/rwhod
 */
login_info()
{
	struct whod wd;
	struct whoent *we;
	DIR *dp;
	struct direct *dirent;
	int cc, fd, m,n, machno, gotone, readfds = 1;
	char fname[100], machines[15][15];
	struct user_data *ud;
	struct timeval timeout;
	timeout.tv_usec = 0;
	timeout.tv_sec = 10;
	machno = 0;
/* search through the files in spool directory for rwhod */
	if ((dp = opendir(WHDIR)) == NULL) {
		perror("can't open");
		return;
	}
	for (n = 0; n < 50; n++) {
		for (m =0; m < 15; m++) user_info[n].machtty[m][0] = '\0';
		user_info[n].idle = LARGE;
		user_info[n].lastmach = -1;
	}
	while ((dirent = readdir(dp)) != NULL) {
		if (strncmp(dirent->d_name,"whod.",5) != 0) continue;
		(void) strcpy(fname,WHDIR);
		(void) strcat(fname,"/");
		(void) strcat(fname,dirent->d_name);
		if ((fd = open(fname, O_RDONLY, 0)) <= 0) {
			perror("Can''t open spool file");
			continue;
		}
/* read the file - might just fail if you clash with rwhod - however this
 * would only mean that this machine was ignored */
		cc = read(fd, (char *) &wd, sizeof(wd));
		if (cc < WHDRSIZE) { /* we have clashed with rwhod !!! */
			fprintf(stderr,"Can't read whod\n");
			(void) close(fd);
			continue;
		}
		(void) strcpy(machines[machno], wd.wd_hostname);
		gotone = 0;
		cc -= WHDRSIZE;
		we = wd.wd_we;
/* check each user in turn */
		for (n = cc / sizeof (struct whoent); n > 0; n--) {
			if (((ud = user_slot(we->we_utmp.out_name)) !=
			  (struct user_data *) 0) 
			  && (strncmp(we->we_utmp.out_line, "ttyp", 4) != 0)) {
			  /* ignore pseudo ttys */
			  	int swidle;
			  	gotone++;
/* only the terminal with the lowest idle time will be dispayed */
				if (we->we_idle < ud->idle) swidle = 1; else swidle = 0;
			  	if (!(ud->lastmach == machno) ||  (swidle == 1))
				  (void) strcpy(ud->machtty[machno], we->we_utmp.out_line);
				if (swidle) {
					ud->idle = we->we_idle;
					ud->lastmach = machno;
					}
			}
			we++;
		}
		if (gotone) machno++;
		(void) close(fd);
	}
	(void) closedir(dp);
	lineno = 0;
/* display standard text */
	(void) werase(head);
	(void) werase(info);
	(void) wstandout(head);
	(void) mvwprintw(head,0,0,"Name");
	
/* print the machine names */
	for (n = 0; n < machno; n++)
	  (void) mvwprintw(head,0,(18 + 8 * n), "%-8s", machines[n]);
	(void) wstandend(head);
	
/* print the login ttys - a * shows the last used tty */
	ud = user_info;
	while (ud->user_name[0] != '\0') {
		(void) mvwprintw(info,lineno,0,"%-18s", ud->real_name);
		for(n = 0; n < machno; n++) if (ud->lastmach == n)
			(void) mvwprintw(info,lineno,(18 + 8 * n), "*%-7s", ud->machtty[n]);
		else (void) mvwprintw(info,lineno,(18 + 8 * n), " %-7s", ud->machtty[n]);
		ud++; lineno++;
	}
/* display the data */
	inst("Hit space to return to display window");
	touchwin(head);
	touchwin(info);
	(void) wrefresh(head);
	(void) wmove(info,3,0);
	(void) wrefresh(info); 
/* wait here for input but time out after 10 seconds */
	(void) select(1,&readfds,(int *) 0,(int *) 0,(struct timeval *) &timeout);
}

/* display some instructions at the bottom of the screen */
inst(str)
char *str;
{
	(void) wstandout(instruct);
	(void) mvwprintw(instruct,0,0,"%-79s", str);
	(void) wstandend(instruct);
	(void) wrefresh(instruct);
}
