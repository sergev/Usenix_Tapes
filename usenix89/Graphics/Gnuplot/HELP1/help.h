
	/*
         *	help.h: header file for the VMS-help emulator
	 */

#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/dir.h>


#define iswhite(_c)       (_c ==' ' || _c =='	' || _c =='\n' || _c =='\r')
#define PROMPT "Topic? "
#define SUBPROMPT "subtopic? "
#define HELPEX	".HLP"				/* help extension */
#define	MANEX	".MANUAL"			/* manual filename*/
#define MAN_SUBEX "/.MANUAL"                    /* other man filename */
#define INFOHEAD "file "
#define INFOTAIL " | sed 's/^/     /'"

#define	MAXLINELEN	128
#define	MAXNAMELEN	 20		/* max length of NAME of topix */
#define	MAXNAMES	256		/* maximum number of subtopics */
#define COLUMNSPACE	  4
#define TERMWID		 76
#define HELPDIR "/usr/help"
#define VIEWPROGRAM  "/usr/ucb/more"    /* program to look at text */
#define VIEWPROGOPTS1 "-s"
#define VIEWPROGOPTS2 "-18"
/* #define VIEWPROGRAM  "/usr/ucb/more"    /* program to look at text */
/* #define VIEWPROGOPTS1 "-d" */
/* #define VIEWPROGOPTS1 "-s" */
/* #define VIEWPROGOPTS2 "-18" */
#define SHELLPROG    "/bin/sh"		/* program to execute text */
#define SHELLOPTS    "-c"

char 	progname[MAXNAMELEN];
char	olddir[MAXNAMELEN + 68];
char	newdir[MAXNAMELEN + 68];
char	*helpdir;
char	**environ;	/* environment, for forks */

int	dumb_flag;
int	list_flag;
int	col_flag;
int	frst_flag;

char	*helpcmds[] =
	{ "Return - Exit this level of help",
	  "*	  - Print this message",
	  "?	  - Reprint this help and its subtopics",
	  "#	  - Reprint just subtopics of this help",
	  ".	  - Look at current manual page, if any",
	  "$<topic>	- Get information on topic files",
	  ".<topic>	- Look at topic manual page, if any",
	  "<topic> 	- Look at help for subtopic `topic'" ,
	  NULL
	};

