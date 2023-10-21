/*  globvar.h
*
*	All the global variables are listed here.  Globals were used in case
*	a later modification to m7 was to enable the user to change the special
*	characters from the standard set to his own set.
*
*			Programmer: Tony Marriott
*/
char	ESCAPE = '\\';
char	EOS = '\0';
char	LT = '\n';
char	LETN = 'n';
char	LETT = 't';
int	NEWLINE = 10;
int	TAB = 9;
 
 
 
int	OK = 1;
int	ERR = 0;
 
char	ANY = '?';
char	BOL = '^';
char	EOL = '$';
char	CCL = '[';
char	CCLEND = ']';
char	DASH = '-';
char	NCCL = '`';
char	CLOSURE = '*';
char	NOT = '~';
char	CHAR = '_';
 
int	COUNT = 1;
int	PREVCL = 2;
int	START = 3;
int	CLOSIZE = 4;
 
int	NO = 0;
int	YES = 1;
 
int	ON =  1;
int	OFF = 0;
 
int	TRUE = 1;
int	FALSE = 0;
 
int	STDIN = 0;
int	STDOUT = 1;
 
int	EOF = -1;
int	NOTFOUND = -1;
 
int	RD = 0644;
int	WT = 0644;
int	RW = 2;
 
char	DITTO = '&';
char	TAG = '{';
char	TAGEND = '}';
char	GETTAG = '@';
 
char	MACTERM = ';';
char	MACDELIM = '\'';
char	REPLACE = '=';
char	SCANOFF = '<';
char	LINECONT = '/';
char	*VARTABLE[3];
int	EOT;
char	CC = '%';
int	TRACE = 0;
int	RESCAN = 1;
char	STACK = '&';
char	CNTR = '#';
char	STCKEND = ')';
char	CNTREND = ')';

int	RESCANMAC = 1;
int	DONTRESCAN = 0;
int	MACNUM = 0;
