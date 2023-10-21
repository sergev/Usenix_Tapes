#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <setjmp.h>
typedef struct		/* used to find builtin commands */
{
	char *cmdname;
	int (*func)();
} builtin;

extern builtin commands[];
extern char histerr[];
extern int j,hiscount;
extern char *history[];
extern int histsize;
extern int numcmds;
static int quiet = 0;

char *version = "SHELL VERSION 1.1 Kent Williams";

jmp_buf env;

char *pipename[] =
{
	"\\shtmp1",
	"\\shtmp2"
};

char cmdbuf[512];
int  currname = 0;
int result = 0;

main(argc,argv)
	char *argv[];
{
	signal(SIGINT,SIG_IGN);	/* ignore breaks */

	quiet = !isatty(0);		/* quiet = batch shell */

	/* initialize local environment */
	init_env();
	cli();
	exit(0);
}
$nodebug		/* turn off state machine debugging */
$machine cli getline () 
$endargs
/* global variables */
	int i;
	int repeat, state, inpipe = 0;
	static char localbuf[256];
	static char histbuf[256];
	static char tail[256];
	int histindex,argindex,takeline;
	char *local = localbuf;
	char *current,*curr_save;
	char *ntharg(), *argptr;
	char *savestr();

$state getline
		/* kill tmp files */
		unlink(pipename[0]); unlink(pipename[1]);

		hiscount = j % histsize; /* hiscount is current position in history */
		if(!quiet)
			fprintf(stderr,"%d%% ",j);

/*
 * The following code simply reads a line from standard input.
 * It is so complicated because when you save the standard stream
 * files and execute another program/command, standard input is
 * left in an uncertain state - the FILE stdin seems to be at EOF,
 * even when standard input is associated with the console, and
 * cr/lf combinations show up as line terminators, whereas usually
 * only linefeeds get placed in the input stream.
 * WHY? beats me.  Something could be wrong with
 *  1. AZTEC C runtime
 *  2. PCDOS
 *  3. Me
 *  4. All three, or permutations of 1-3 reducto ad absurdum.
 * All I know is this works
 */
		/* clear command buffer so string read is null terminated */
		setmem(cmdbuf,sizeof(cmdbuf),0);
		for (current = cmdbuf;;current++)
		{
			int readresult;
			if ((readresult = read(0,current,1)) == 0 ||
				readresult == -1)
			{
				$nextstate terminal
			}
			if (*current == '\r')
			{
				if ((readresult = read(0,current,1)) == 0 ||
					readresult == -1)
				{
					$nextstate terminal
				}
				*current = '\0';
				break;
			}
			else if (*current == '\n')
			{
				*current = '\0';	/* terminate string */
				break;
			}
		}
		current = cmdbuf;	/* point current at start of buffer */
/*
 * end of input weirdness
 */
		/* if we're recycling history strings, free previous one */
		if (history[hiscount])
			free(history[hiscount]);

		/* save current in history array */
		history[hiscount] = savestr(current);
		/* parse command for compound statements and pipes */
		local = localbuf;	/* set pointer to state of buffer */
		setmem(localbuf,sizeof(localbuf),0);	/* clear buffer */
		$nextstate eatwhitespace 
$endstate getline

$state charstate
	switch(*current)
	{
	case '\0':
		*local = '\0';
		current++;
		$nextstate emit
	case '"' :
		*local++ = *current++;
		$nextstate doublequotes
	case '/' :
		*local++ = '\\';
		current++;
		$nextstate charstate;
	case '\'':
		*local++ = *current++;
		$nextstate singlequotes
	case '\\':
		*local++ = *++current;
		current++;
		$nextstate charstate
	case ';':
		*local = '\0';
		current++;
		$nextstate compound
	case '|':
		*local = '\0';
		current++;
		$nextstate pipe
	case '!':
		current++;
		$nextstate histstate
	default:
		*local++ = *current++;
		$nextstate charstate
	}
$endstate charstate

$state emit
	if (inpipe)
	{
		inpipe = 0;
		strcat(localbuf," < ");
		strcat(localbuf,pipename[currname]);
	}
	command(localbuf);
	$nextstate done
$endstate emit

$state compound
	if (inpipe)
	{
		inpipe = 0;
		strcat(localbuf," < ");
		strcat(localbuf,pipename[currname]);
	}
	command(localbuf);
	local = localbuf;
	setmem(localbuf,sizeof(localbuf),0);	/* clear buffer */
	$nextstate eatwhitespace
$endstate compound

$state singlequotes
	switch (*current)
	{
	case '\0':
		write(2,"No closing quotes!!\r\n",21);
		$nextstate parserr
	case '\'':
		*local++ = *current++;
		$nextstate charstate
	default:
		*local++ = *current++;
		$nextstate singlequotes
	}
$endstate singlequotes

$state doublequotes
	switch(*current)
	{
	case '\0':
		write(2,"No closing quotes!!\r\n",21);
		$nextstate done
	case '"':
		*local++ = *current++;
		$nextstate charstate
	default:
		*local++ = *current++;
		$nextstate doublequotes
	}
$endstate doublequotes

$state histstate
	/* handle history substitutions */
	setmem(histbuf,sizeof(histbuf),0);	/* clear buffer */

	/* save current pointer into command buffer */
	curr_save = current;

	/* copy command head */
	strncpy(histbuf,cmdbuf,(int)(current-cmdbuf)-1);

	/* takeline means take all arguments past current one */
	takeline = 0;

	/* parse history expression */
	switch (*current)
	{
	case '!':	/* last command line */
		if (j)	/* special case first time through */
		{
			histindex = hiscount ? hiscount - 1 : histsize - 1;
		}
		else
		{
			/* force error condition */
			write(2,histerr,strlen(histerr));
			$nextstate parserr
		}
		current++;	/* point to next */
		break;
	case '-':		/* negative (relative #) */
	/* a particular numbered command */
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		/* repeat numbered command */
		repeat = atoi(current);
		if (repeat < 0)	/* handle relative addressing */
			repeat += j;

		/* if command is within range */
		if ((j - repeat) <= histsize && repeat < j)
		{
			histindex = repeat % histsize;
		}
		else
		{
			$nextstate parserr
		}

		/* skip past numeric expression */
		while(isdigit(*current)||*current=='-')
			++current;
		break;
	default:
		write(2,"Bad history expression\r\n",24);
		$nextstate parserr
	}
	/* look for particular argument substitutions */
	switch (*current)
	{
	/* we want the whole enchilada */
	case '\0':
	case '\t':
	case '\r':
	case '\n':
	case ' ':
		strcat(histbuf,history[histindex]);
		break;
	case ':':
		++current;	/* point past colon */
		switch (*current)
		{
		case '^':
			argindex = 1;
			++current;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			/* index of argument */ 
			argindex = atoi(current);
			while(isdigit(*current))
				++current;
			if (*current == '*')
			{
				takeline = 1;
				current++;
			}
			break;
		case '$':
			argindex = lastarg(history[histindex]);
			current++;
			break;
		case '*':
			takeline = 1;	/* take arg 1 through arg n */
			argindex = 1;
			current++;
			break;
		default:
			$nextstate parserr
		}
		/* pick up pointer to argument in history we need */
		if (takeline == 0)
		{
			if (NULL == 
				(argptr = ntharg(history[histindex],argindex)))
			{
				$nextstate parserr
			}
			strcat(histbuf,argptr);
		}
		else
		{
			while (NULL !=
				(argptr = ntharg(history[histindex],argindex++)))
			{
				strcat(histbuf,argptr);
				strcat(histbuf," ");
			}
		}
	}
	/* history substitutions */
	/* copy command buffer tail to tail buffer */
	strcpy(tail,current);
	/* copy histbuf back to cmdbuf */
	strcpy(cmdbuf,histbuf);
	/* point current at history substitution to continue parsing */
	current = --curr_save; /* -1 to backup over first ! */
	/* copy tail in */
	strcat(cmdbuf,tail);
	free(history[hiscount]);
	history[hiscount] = savestr(cmdbuf);
	$nextstate charstate
$endstate histstate

$state pipe
	if (inpipe++)
	{
		inpipe = 1;
		strcat(localbuf," < ");
		strcat(localbuf,pipename[currname]);
	}
	strcat(localbuf," > ");
	currname ^= 1;
	strcat(localbuf,pipename[currname]);
	command(localbuf);
	local = localbuf;
	setmem(localbuf,sizeof(localbuf),0);
	$nextstate eatwhitespace
$endstate pipe

$state eatwhitespace
/* strip out leading white space */
while(isspace(*current))
		current++;
	if (!*current)
		$nextstate parserr
	else
		$nextstate charstate
$endstate eatwhitespace

$state parserr
	$nextstate getline
$endstate parserr

$state done
	j++;	/* next command # */
	$nextstate getline
$endstate done

$endmachine cli

onintr()
{
	longjmp(env,-1);
}

command(current)
	register char *current;
{
	extern do_prog();
	register int i;
	std_save();
	if (-1 == (i = findcmd(current)))
	{
		ctl_brk_setup();
		result = _Croot(current,do_prog);
		ctl_brk_restore();
	}
	else
	{
		if (-1 != setjmp(env))
		{
			signal(SIGINT,onintr);
			result = _Croot(current,commands[i].func);
		}
		signal(SIGINT,SIG_IGN);
	}
	std_restore();
}

char *
ntharg(line,index)
register char *line;
{
	register int i;
	static char buf[64];
	char *bptr;
	for (i = 0; *line;i++)
	{
		/* find start of arg[i] */
		while(*line && isspace(*line))
		{
			++line;
		}
		/* if this is start of requested arg, return pointer to it */
		if (i == index)
		{
			bptr = buf;
			while(*line && !isspace(*line))
				*bptr++ = *line++;
			*bptr = '\0';
			return buf;
		}
		/* find end of arg[i] */
		while(*line && !isspace(*line))
			++line;
	}
	return NULL;
}

lastarg(line)
register char *line;
{
	register int i;

	for (i = 0; *line;i++)
	{
		/* find start of arg[i] */
		while(*line && isspace(*line))
			++line;
		/* find end of arg[i] */
		while(*line && !isspace(*line))
			++line;
	}
	return i-1;
}
