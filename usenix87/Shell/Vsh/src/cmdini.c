#include "hd.h"
#include "command.h"

extern date(), showerror(), showgrep(), file(), home(), grep(),
    wmake(), fmake(), callshell(), longlist(), remove(),
    create(), exec(), display(), options(), xecute(), xchdir(), rexecute(),
    chain(), find(), pagexec();

struct classstruct classtab[] = 
{
	"date",		date,		0, 1,
	"showerror",	showerror,	0, 1,
	"showgrep",	showgrep,	0, 1,
	"file",		file,		-1, 1,
	"home",		home,		0, 1,
	"grep",		grep,		0, 1,
	"wmake",	wmake,		0, 1,
	"fmake",	fmake,		0, 1,
	"exec",		exec,		-2, 1,
	"xecute",	xecute,		-2, 1,
	"rexecute",	rexecute,	0, 1,
	"shell",	callshell,	-1, 1,
	"display",	display,	-1, 1,
	"create",	create,		0, 1,
	"longlist",	longlist,	0, 0,
	"remove",	remove,		0, 0,
	"options",	options,	-1, 1,
	"null",		0,		0, 0,
	"chain",	chain,		-2, 1,
	"find",		find,		-1, 1,
	"pexec",	pagexec,	-2, 1,
	"",		0,		0, 0
} ;

static char *whoargv[]  = 
{
    "/bin/who", 0
} ;
static char *rootargv[] = 
{
    "/", 0
} ;
static char *helpargv[] = 
{
    "/usr/lib/vsh/genhelp", 0
} ;
static char *psargv[] = 
{
    "/bin/ps", 0
} ;
static char *bshargv[] = 
{
#ifdef	PWBTTY
    "/bin/v7sh", 0
#else
    "/bin/sh", 0
#endif
} ;

/* dyt */
extern char wdname0[];
static char *owdargv[] =
{
	wdname0, 0
};

char *shargv[] =
{
#ifdef	PWBTTY
	"/bin/sh", 0
#else
	"/bin/csh", 0
#endif
};

char *cnull = 0;

struct cmdstruct cmdtab[] = 
{
	'C',	create,		&cnull, 1,
	'D',	date,		&cnull, 1,
	'E',	showerror,	&cnull, 1,
	'F',	file,		&cnull, 1,
	'G',	grep,		&cnull, 1,
	'L',	longlist,	&cnull, 0,
	'M',	wmake,		&cnull, 1,
	'N',	fmake,		&cnull, 1,
	'O',	options,	&cnull, 1,
	'P',	exec,		psargv, 1,
	'R',	remove,		&cnull, 0,
	'S',	showgrep,	&cnull, 1,
	'T',	display,	&cnull, 1,
	'W',	exec,		whoargv, 1,
	'X',	xecute,		&cnull, 1,
	'Y',	rexecute,	&cnull, 1,
	'^',	home,		&cnull, 1,
	'!',	callshell,	&cnull, 1,
	'%',	chain,		shargv, 1,
	'$',	exec,		bshargv, 1,
	'/',	file,		rootargv, 1,
	'?',	display,	helpargv, 1,
/* dyt */
	'\\',	file,		owdargv, 1,
	'.',	find,		&cnull, 1,

	'A',0,0,0,'B',0,0,0,
	'H',0,0,0,'I',0,0,0,'J',0,0,0,'K',0,0,0,
	'Q',0,0,0, 'U',0,0,0,'V',0,0,0,
/* dyt */
	'Z',0,0,0,'#',0,0,0,
	'&',0,0,0,'(',0,0,0,')',0,0,0,
	'*',0,0,0,'=',0,0,0,'{',0,0,0,'}',0,0,0,'[',0,0,0,']',0,0,0,
	'~',0,0,0,',',0,0,0,';',0,0,0,
	':',0,0,0,'`',0,0,0,'<',0,0,0,'>',0,0,0,'@',0,0,0,'_',0,0,0,
	'|',0,0,0,
	'e',0,0,0,'f',0,0,0,'g',0,0,0,'h',0,0,0,'i',0,0,0,'j',0,0,0,
	'k',0,0,0,'l',0,0,0,'m',0,0,0,'n',0,0,0,'o',0,0,0,'p',0,0,0,
	'q',0,0,0,'r',0,0,0,'s',0,0,0,'t',0,0,0,
	'u',0,0,0,'v',0,0,0,'w',0,0,0,'x',0,0,0,'y',0,0,0,'z',0,0,0,

	CMD_DATE, date,		&cnull, 1,
	CMD_SE,	showerror,	&cnull, 1,
	CMD_SG,	showgrep,	&cnull, 1,
	0,0,0,0
} ;

/* If this is not EX/VI, check out nedit() in show.c
 * It is preferable to leave this alone and change EDITOR in
 * everyone's .vshrc
 */
char stdedit[] = "vi";

struct parmstruct parmtab[] = 
{
	"EDITOR", stdedit,
	"make", "/bin/make",
	"grep", "/bin/grep",
	"rmhelp", "/usr/lib/vsh/rmhelp",
	"showhelp", "/usr/lib/vsh/showhelp",
	"makerror", ".makerror",
	"grepout", ".grepout",
	"vshmode", "enter",
	"quitchar", "D",
	"pagechar", "{",
	"PATH", ":/bin:/usr/bin",
	"TERM",	"unknown",
	"HOME", "",
	"SHELL", "/bin/sh",
	"entertext", ENTEREDIT,
	"window", "24",
	"helpfile", "",
	"column", "1",
	"noargprompt", "",
	"VImotion", "",
#ifdef	V7TTY
	"MAIL", "/usr/spool/mail/~",
#else
	"MAIL", "/usr/mail/~",
#endif
	"moresize", MMAXSIZE,
	"enterpath", ":",
	0, 0
} ;
