/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include <curses.h>
#include <ctype.h>
FILE	*xopen ();
char	*getargs (), *copy (), *interpolate (), *getresponse ();
extern	char	menudir[100];
extern	char	maildir[100];
extern	char	*mailfile;
extern	int	anchored;
extern	int	newmenu;
extern	int	ttyspeed;
#define	MAXOPTION 15
#define	MAXCOM 10
#define	COMMAND 0
#define MAXVAR 100
#define	OUT_OF_RANGE -2
#define	INDENT     5
#define MENUTOP    1
#define MENUBOTTOM 23
#define HISTORY    (MAXOPTION+1)
#define TIMELINE   (MENUBOTTOM)
#define INFOLINE   (MENUBOTTOM-2)
#define RESPLINE   (MENUBOTTOM-1)
#define	NOTICES    (HISTORY-1)
#define RIGHTMENU  (COLS/2)
#define GETRETURN  {printf("press RETURN to continue");while(getchar()!='\n');}
extern char	response[BUFSIZ];
extern char	*command[MAXCOM];
extern char	commandbuffer[MAXCOM][BUFSIZ];
extern int	docmode;
extern int	progmode;
struct	VAR
	{
	char	*name;
	char	*value;
	};
extern struct VAR variables[MAXVAR];
extern int	nvar;
extern WINDOW *lmenu, *timewin, *history;
struct MENU
	{
	char	*menuname;
	char	*display[MAXOPTION];
	char	*program[MAXOPTION];
	char	*arguments[MAXOPTION];
	char	selector[MAXOPTION];
	int	noptions;
	struct MENU *nextmenu[MAXOPTION]; 
	struct MENU *parent;
	char	nowait[MAXOPTION];
	};
extern struct MENU *menu;
extern struct MENU *rootmenu;
extern struct MENU *stdmenu;
extern struct MENU *savemenu;
extern struct MENU *readmenu ();

extern short	uid, gid;
#define	ESC       ''
#define	BACKSPACE ''
#define ESCAPE    '\\'
#define RETURN    '\n'
#define MODECHANGE -2
#define RUN	   -1
extern char	escapechar;
extern char	varchar    ;
extern char	shellchar  ;
extern char	popchar    ;
extern char	unixchar   ;
extern char	dotdotchar ;
extern char	quitchar   ;
extern char	modechar   ;
extern int	flipped    ;
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#define MAXDEPTH 10
#define	NAMELENGTH DIRSIZ+2
extern char	dirpath[MAXDEPTH][NAMELENGTH];
extern int	pathlength ;
extern char	pwdname[BUFSIZ];
extern WINDOW	*filewin;
#define MAXENTRIES 250
extern int	nonames;
extern int	page ;
#define	DIRFILE 3
#define	PLAINFILE      1
#define PROGFILE   2
#define	PAGESIZE  9
#define	NAMESIZ  DIRSIZ+2
struct FILENT
	{
	char	f_name[NAMESIZ];
	char	f_protect[12];
	off_t	f_size;
	};
extern struct FILENT filent[MAXENTRIES];
#define begin(c) (c == '[')
#define end(c)  (c == ']' || c == '\n' || c == NULL)
#define separator(c) (c == ':')
#define skipspace(ptr) while (isspace (*ptr)) ptr++;
