#include <sys/types.h>
#include <stdio.h>
#include <setjmp.h>

#define UTMP 		"/etc/utmp"	/* name of utmp file */
#define TERMFILE	"/etc/termfile" /* name of termfile  */

#ifdef DEBUG
#define CONFIG		"/userf/doc/untamo/untamo.cf"
#define LOGFILE		"/userf/doc/untamo/untamo.log"
#else DEBUG
#define CONFIG		"/usr/local/lib/untamo.cf"
#define LOGFILE		"/usr/adm/untamo.log"
#endif DEBUG

#define DEV		"/dev/\0"

#define MAXUSERS	100		/* max people expected */
#define NAMELEN		8		/* length of login name */
#define IS_IDLE		01
#define IS_MULT		02
#define IS_LIMIT	04

struct user {
#ifdef PUCC
	int rld;		/* rld number last logged in to */
	char clust[7]; 		/* cluster */
#endif PUCC
	int idle;		/* max idle limit for this user */
	int ugroup;		/* gid obtained from getgrent call */
	int session;		/* session limit for this user */
	int warned;		/* if he has been warned before */
	int exempt;		/* what is this guy exempt from? */
	time_t next;		/* next time to examine this structure */
	time_t time_on;		/* loggin time (express terminals?) */
	char uid[NAMELEN];	/* who is this is? */
	char line[14];		/* his tty in the form "/dev/ttyxx"*/
	char site[10];		/* where */
};

/* 
   next will be cur_time+limit-idle do all we have to do is check
   the current time against the action field when the daemon comes
   around.  if >= then it's time to check the idle time again, else
   just skip him.
*/

extern struct user users[];
extern struct user *pusers[];

/*
 * records that the nodes of the linked list 
 * will have pointers too
 */
struct item {
	int name_t;	/* is it a login, group, etc... */
	char *name;	/* which login, etc */
	int num;	/* which rld (special case, rld is a #)*/
			/* or group -doc*/
	int flag;	/* what is the timeout/exemption ? */
};

/*
 * necessary structures to use the system
 * linked list stuff.  q_item will be a pointer
 * to stuct items.
 */
struct qelem {
	struct qelem *q_forw;
	struct qelem *q_back;
	struct item  *q_item;
};

extern jmp_buf env_buf;		/* where to jump on timeouts	*/
extern FILE *logfd;		/* log file file descriptor	*/
#ifdef PUCC
extern char *findcluster();
#endif PUCC

/* These items are gleaned from the configuration file...	*/

extern struct qelem *rules, 	/* list of idle timeout rules	*/
		    *exmpt, 	/* list of exemptions		*/
		    *session;	/* list of session limit rules  */
extern int sleeptime;		/* how long to sleep between scans of utmp    */
extern int m_threshold;	/* number of users for multiple login warnings        */
extern int s_threshold; /* number of users (sds-ports)  fork session limits   */
extern int warn_flags;	/* what warnings to make	                      */
