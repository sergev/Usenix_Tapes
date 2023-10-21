/*      Chat System Definition Header File     */
/*		->slb\@bnl.ARPA		       */
/* chat.h :-: 9/19/85 */

#define SYSTEMFILE "/usr/slb/Chat_sfile"
#define PREFIX  "/tmp/ct."              /* Storage for messages */
#define FNAME   "/etc/utmp"             /* To see who's on the system */
#define PAGER 	"/bin/mail "		/* Medium for paging */
#define LOGFILE	"/usr/slb/CHATLOG"	/* List of users in chat */
#define MAXUSERS 10			/* Max # of users in chat */

struct	logs {
	char	l_line[5];		/* User's tty */
	char	l_name[20];		/* User's name and handle */
	char	l_time[7];		/* Entered chat	*/
}
names[MAXUSERS];
