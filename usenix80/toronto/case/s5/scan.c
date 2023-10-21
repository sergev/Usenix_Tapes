# include "sgtty.h"

/*
 *	scan(u_cmds, upper, lower, brkc, prompts)
 *
 *	A generalized command scanner for CRT's.  Taken from the command
 *	scanner INTERP of the TENEX operating system.
 *
 *	Parameters:
 *		u_cmds - char *u_cmds[], holds commands to be matched.
 *			Special commands are:
 *				WILDCARD = "<rubout>" - matches any input
 *				string entered; string is available in
 *				global string command.
 *
 *				null command = "<\0>" - matches any null
 *				input sequence.
 *		upper - index of top of list of commands to be recognized
 *			(must be > 0 by C convention).
 *		lower - index of bottom of list of commands to be recognized.
 *		brkc - char *brkc, allows scanner to pass back the break
 *			character that terminated scanning.
 *		prompts - char[], prompt string; if null in length the
 *			default prompt string is used (": ").
 *	Results:
 *		result passed back is index into array of commands passed,
 *		this value is always >= upper and <= lower. A value of -1
 *		is returned on an error condition - presently only possible
 *		if upper and lower are messed up.
 *
 *		brkc - as decribed above.
 *
 *		command - holds the string recognized (useful only with
 *		WILDCARD command).
 *	Restrictions:
 *		1. Commands must sorted!!!!!
 *		2. Care must be taken to remember the terminal is placed
 *		   in raw and no echo mode. Hence the cursor position
 *		   will always be at the end of the current line scanned.
 *	Bugs:
 *		Command sets that contain any command which is a proper
 *		substring of another cause the scanner to lock up and
 *		allow recognition only of the shorter command. For
 *		example 'glob' and 'global'.
 *	S. J. Leffler - 2/18/79
 */

# define CMAX		80	/* max characters in a command */
# define internal	static
# define entry	

typedef unsigned boolean;

internal boolean idok;

internal int	chars,	/* number of chars recognized so far */
		tpnt,	/* pointer to top of region in table where command might be */
		bpnt,	/* pointer to bottom of " */
		bot,	/* like bpnt, only temporary (used in eval) */
		top,	/* like tpnt, as above */
		TOP,	/* top of the array of commands */
		BOTTOM;	/* bottom of the array of commands */

entry char	_command[CMAX];	/* available for external use in case of
				  WILDCARD match */

internal char	*PROMPTS	"",
		**cmds;	/* internal pointer to array of commands */

# define TRUE	1
# define FALSE	0

# define min(a,b)	((a) < (b) ? (a) : (b))

# define bell		putchar('\07')

# define echoon		statb.sg_flags |= ECHO; statb.sg_flags &= ~RAW; stty(1,&statb)

# define echooff	statb.sg_flags |= RAW; statb.sg_flags &= ~ECHO; stty(1,&statb)

# define RUBOUT		'\177'
# define WILDCARD	"\177"
# define TAB		'\t'
# define CTRLR		'\022'
# define CTRLW		'\027'
# define CTRLU		'\025'
# define ALT		'\033'
# define SPACE		'\040'
# define EOL		'\n'
# define NULL		'\0'
# define BRKLEVEL	'\040'
# define BACKUP		"\b \b"

entry scan(u_cmds, upper, lower, brkc, pstring)
char	*u_cmds[],
	*brkc,
	pstring[];
int	upper,
	lower;
{

	if((upper > lower)|(upper < 0))
		return(-1);	/* error */
	TOP = upper;
	BOTTOM = lower;
	cmds = u_cmds;
	if(length(pstring)) PROMPTS = pstring;	/* set default prompt string */
	else PROMPTS = "";
	*brkc = interpret();
	return(tpnt+1);
}

internal interpret()
{
register char nextchar;
int	i,
	tchars,
	bmax;
struct sgttyb statb;

	gtty(1,&statb);
	idok = FALSE;
	bmax = BOTTOM;

	/*
	 * check for WILDCARD matches (if there is one it defines the
	 * top of the array)
	 */
	while(!(bmax < TOP) && equ(cmds[bmax],WILDCARD)){
		idok = TRUE;
		bmax--;
	}
	echooff;	/* set terminal mode */
restart:
	strout(PROMPTS);
	chars = 0;
	*_command = NULL;
	tpnt = TOP-1;
	bpnt = bmax+1;
	do{
		nextchar = getchar();
		if(nextchar <= BRKLEVEL)
			switch(nextchar){

		case CTRLR:	/* retype the line */
			tchars = chars;
			while(--tchars > -1)
				strout(BACKUP);
			strout(_command);
			break;
		case CTRLW:	/* erase the last word */
			while(--chars > -1)
				strout(BACKUP);
			chars = 0;
			*_command = NULL;
			tpnt = TOP-1;
			bpnt = bmax+1;
			break;
		case CTRLU:	/* kill the line */
			strout("^U\n");
			goto restart;
		case ALT:	/* force command recognition */
			recognize();
			if(!(bpnt-tpnt-2))
				goto finished;
			break;
		case EOL:	/* force command recognition */
			if((chars == 0) && length(cmds[TOP])){
				putchar('\r');
				goto restart;
			}else
				goto eolaction;
		case SPACE:
		case TAB:	/* command recognition */
		eolaction:
				if(idok){
					tpnt = bmax;
					goto finished;
				}else
					switch(min(bpnt-tpnt-1,2)){

				case 0:
					tpnt = bmax;
					goto finished;
				case 1:
					recognize();
					goto finished;
				case 2:
					if(chars == length(cmds[tpnt+1]))
						goto finished;
					else
						recognize();
					}
			}/* end switch */
		else{	/* normal character */
			if(nextchar == '?'){	/* show possible choices */
				putchar(nextchar);
				strout("\nOne of the following:\n");
				if((chars == 0) && (length(cmds[TOP]) == 0))
					strout("<carriage return>");
				for(i = tpnt+1; i < bpnt; i++){
					strout(cmds[i]);
					putchar(EOL);
				}
				for(i = bmax+1; i <= BOTTOM; i++){
					strout(cmds[i]);
					putchar(EOL);
				}
				strout(PROMPTS);
				strout(_command);
			}else if(nextchar == RUBOUT){	/* kill last character */
				_command[--chars] = NULL;
				tpnt = TOP-1;
				bpnt = bmax+1;
				if(chars){
					eval();
					bpnt = bot;
					tpnt = top;
				}
				strout(BACKUP);
			}else{	/* character recognition */
				_command[chars++] = nextchar;
				_command[chars] = '\0';
				eval();
				if((bot-top > 1)|(idok)){	/* allowable by table of commands or WILDCARD */
					tpnt = top;
					bpnt = bot;
					putchar(nextchar);
				}else{	/* unallowed character */
					chars--;
					_command[chars] = '\0';
					bell;
				}
			}
		}
	}while(TRUE);
finished:
	echoon;	/* return terminal mode */
	return(nextchar);
}

/*
 *	eval() - binary table search algorithm
 *
 *	Takes global table pointers tpnt and bpnt and selects
 *	region in table where partially read command could possibly
 *	be.  If no entry is possible the pointers cross.
 */

internal eval()
{
int	middle,
	trial,
	i;

	top = tpnt;
	middle = bot = bpnt;
	while((middle-top) > 1){
		trial = (middle+top) >> 1;
		i = compare(cmds[trial],_command);
		if(i < 0)
			top = trial;
		else{
			middle = trial;
			if(i)
				bot = trial;
		}
	}
	while((bot-middle) > 1){
		trial = (bot+middle) >> 1;
		if(compare(_command,cmds[trial]) < 0)
			bot = trial;
		else
			middle = trial;
	}
}

/*
 *	recognize() - interpret table pointers set by eval()
 *
 *	According to value of table pointers bpnt and tpnt either,
 *		0) prompt with a bell for a new character.
 *		1) finish printing out the command.
 *		2) print out as much of the common portion of
 *		   the ambiguous command as possible then prompt for
 *		   more information.
 */

internal recognize()
{
char	*stop,
	*help,
	*sbot;
int	i;
register j;

	switch(min(bpnt-tpnt-1,2)){

	case 0:
		bell;
		return;
	case 1:
		help = stop = cmds[tpnt+1];
		i = length(stop);
		break;
	case 2:
		stop = cmds[tpnt+1];
		sbot = cmds[bpnt-1];
		help = stop;
		i = chars;
		while(stop[i] == sbot[i])
			i++;
	}
	for(j=chars;j < i; j++)
		putchar(help[j]);
	for(i=0; i < j; i++) _command[i] = help[i];
	_command[j] = NULL;
	chars = j;
	if(bpnt-tpnt-2)
		bell;
}

internal length(s)
register char *s;
{
register i;

	i = 0;
	while(*s++)i++;
	return(i);
}

internal equ(s1,s2)
register char *s1, *s2;
{
register char c1;

	while((c1 = *s1++) == *s2++)
		if(c1 == NULL)
			return(TRUE);
	return(FALSE);
}

internal compare(s1,s2)
register char *s1, *s2;
{
register c1,c2;
register i;
int l, l1, l2;

	l1 = length(s1);
	l2 = length(s2);
	l = min(l1,l2);
	for(i=0; i < l; i++)
		if((c1 = *s1++) != (c2 = *s2++))
			return(c1-c2);
	if(l == 0){
		if(l1 == l2) return(0);
		return(l1 > l2 ? 1 : -1);
	}
	return(0);
}

internal strout(s)
register char *s;
{
	while(*s) write(1,s++,1);
}
