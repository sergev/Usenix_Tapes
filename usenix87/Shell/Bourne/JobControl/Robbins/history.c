/* history.c --- interacterive history mechanism for the Bourne shell */

/*
 * Original design by Jeff Lee for the Software Tools Subsystem,
 * This implementation by Arnold Robbins, based on Jeff's, but
 * a little bit more capable.
 */

#include "defs.h"	/* defines HISTSIZE */
#include "sym.h"

#define MAXHIST		256		/* max no. saved commands */
#define MAXLINE		257
#define BIGBUF		(MAXLINE * 2)

#define HISTCHAR	'!'		/* history flag character */
#define HISTLOOK	'?'		/* history global search command */
#define HISTARG		'`'		/* history argument character */
#define HISTSUB		'^'		/* history substitution character */

#define YES	(1)
#define NO	(0)

#ifndef TAB	/* earlier version of the shell */
#define	TAB	'\t'
#endif

static char	Histbuf[HISTSIZE];	/* queue holding actual history */
static int	Histptr[MAXHIST];	/* queue of pointers into buffer */
static int	Hbuffirst = 0;		/* First pointer into Histbuf */
static int	Hbuflast = 0;		/* Last pointer into Histbuf */
static int	Hptrfirst = 0;		/* First pointer into Histptr */
static int	Hptrlast = 0;		/* Last pointer into Histptr */
static int	Histline = 0;		/* no. of cmd pointed to by Histptr[Hptrlast] */

static char h_badopt[] =	" unrecognized history option";
static char badarg[] =	" illegal argument history";
static char nohist[] =	" no history exists, yet";
static char h_illegal[] =	" illegal history construct";
static char bufover[] =	" history buffer overflow";
static char bigtok[] =	" history token too large";
static char internal[] =	" history internal error";
static char badtoken[] =	" illegal history token";
static char h_notfound[] =	" history item not found";
static char bigexp[] =	" history expansion too big";

extern int	histsub ();		/* do a history substitution */
static void	histinit ();		/* reinitialize history mechanism */
static int	histexp ();		/* do a history expansion */
static int	histque ();		/* save a command in the buffers */
static void	histfree ();		/* free up some buffer storage */
static int	histfind ();		/* find a history command */
static int	histlook ();		/* get a previous command */
static int	histget ();		/* get a string from the buffers */
static void	histarg ();		/* get individual arguments */
extern int	histrest ();		/* restore saved history */
extern int	histsave ();		/* save current history */

static int	Bquote = 0;		/* count grave accents */
static int	Dquote = 0;		/* count single quotes */
static int	Squote = 0;		/* count double quotes */

#define errmsg(x, s)	{ prs(x); prc(COLON); prs(s); newline(); return (FALSE); }

#define repeat		do	/* repeat ... until is easier to read */
#define until(cond)	while (!(cond))

/* histsub --- perform a history substitution */

int histsub (in, out, outsize)
char *in, *out;
int outsize;
{
	if ((flags&prompt) == 0 || in == 0 || *in == '\0')
		return (TRUE);	/* no history, pretend all ok */
	
	return (histexp (in, out, outsize) && histque (out));
}

/* histexp --- perform history expansion on a command line */

static int histexp (in, out, outsize)
char *in, *out;
int outsize;
{
	int i;
	int istart, ilen, ostart;
	char buf[MAXLINE], result[BIGBUF];
	auto int bangseen = NO;

	if (in[0] == NL || in[0] == '\0')
		return (FALSE);

	istart = ostart = ilen = 0;
	while (in[istart] && in[istart] != HISTCHAR)
	{
		if (ostart >= outsize)
			errmsg (in, bigexp);
		switch (in[istart]) {
		case ESCAPE:
			out[ostart++] = in[istart++];
			if (in[istart] == HISTCHAR)
			{
				bangseen = YES;
				if (Squote)
					out[ostart++] = in[istart++];
				else
					out[ostart - 1] = in[istart++];
					/* no quotes, nuke \ */
				continue;
			}
			break;
		case '`':
			if (Dquote == 0 && Squote == 0)
				Bquote = 1 - Bquote;
			break;
		case '\'':
			if (Bquote == 0 && Dquote == 0)
				Squote = 1 - Squote;
			break;
		case '"':
			if (Bquote == 0 && Squote == 0)
				Dquote = 1 - Dquote;
			break;
		}
		if (ostart >= outsize)
			errmsg (in, bigexp);
		out[ostart++] = in[istart++];
		if (Squote && in[istart] == HISTCHAR)
			if (ostart >= outsize)
			{
				errmsg (in, bigexp);
			}
			else
				out[ostart++] = in[istart++];
	}

	if (in[istart] == '\0')
	{
		out[ostart] = '\0';
		if (bangseen)
			expanded = YES;		/* see comment below */
		return (TRUE);	/* no history to do */
	}

	expanded = NO;	/* this is a global flag */
	while (histfind (in, &istart, &ilen))	/* we found something to do */
	{
		if (ilen >= MAXLINE)
			errmsg (&in[istart], bigtok);

		/* save the history part */
		strncpy (buf, & in[istart], ilen);
		buf[ilen] = '\0';
		istart += ilen;
		if (buf[ilen-1] == HISTCHAR)
			buf[--ilen] = '\0';
		
		/* actually make the substitution */
		if (! histlook (buf, result))
			return (FALSE);
		
		/* put it into generated line */
		i = length (result) - 2;
		if (result[i] == NL)
			result[i] = '\0';
		if (ostart + i + 1 >= outsize)
			errmsg (&in[istart], bigexp);
		movstr (result, & out[ostart]);
		ostart += length (result) - 1;
		expanded = YES;
		while (in[istart] && in[istart] != HISTCHAR)
		{
			if (ostart >= outsize)
				errmsg (&in[istart], bigexp);
			switch (in[istart]) {
			case ESCAPE:
				out[ostart++] = in[istart++];
				if (in[istart] == HISTCHAR)
				{
					bangseen = YES;
					if (Squote)
						out[ostart++] = in[istart++];
					else
						out[ostart - 1] = in[istart++];
						/* no quotes, nuke \ */
					continue;
				}
				break;
			case '`':
				if (Dquote == 0 && Squote == 0)
					Bquote = 1 - Bquote;
				break;
			case '\'':
				if (Bquote == 0 && Dquote == 0)
					Squote = 1 - Squote;
				break;
			case '"':
				if (Bquote == 0 && Squote == 0)
					Dquote = 1 - Dquote;
				break;
			}
			if (ostart >= outsize)
				errmsg (&in[istart], bigexp);
			out[ostart++] = in[istart++];
			if (Squote && in[istart] == HISTCHAR)
				if (ostart >= outsize)
				{
					errmsg (&in[istart], bigexp);
				}
				else
					out[ostart++] = in[istart++];
		}
	}

	out[ostart] = '\0';

	if (expanded)
		prs (out);	/* should contain newline */
	else if (bangseen)
		expanded = YES;

	/*
	 * This is a KLUDGE, so that escaped !s work;
	 * it depends on knowledge of how readb() in word.c
	 * works, i.e., if expanded, use the generated buffer.
	 * This way, only expanded is needed as a global variable.
	 */

	return (TRUE);
}

/* histque --- place the given command in the history queue */

static int histque (command)
char *command;
{
	int c;
	char *p;
	static int Inaquote = FALSE;	/* in a quote across commands */

	for (; *command && (*command == SP || *command == TAB); command++)
		; /* skip leading white space */

	if (*command == NL && *(command+1) == '\0')
		return (TRUE);	/* don't queue empty commands */
				/* or increment event_count */

	if (Inaquote)
	{
		/* clobber trailing \0 */
		if (Hbuffirst == 0)
			Hbuffirst = HISTSIZE - 1;
		else
			Hbuffirst--;

		event_count--;
	}

	Histptr[Hptrfirst] = Hbuffirst;
	if (! Inaquote)
		Hptrfirst = (Hptrfirst + 1) % MAXHIST;

	if (Hptrfirst == Hptrlast)
		histfree ();

	p = command;
	c = *p++;
	while (c != '\0' && Hptrfirst != Hptrlast)
	{
		repeat
		{
			Histbuf[Hbuffirst] = c;
			c = *p++;
			Hbuffirst = (Hbuffirst + 1) % HISTSIZE;
		} until (c == '\0' || Hbuffirst == Hbuflast);

		if (Hbuffirst == Hbuflast)
			histfree ();
	}

	if (Hptrfirst != Hptrlast)
	{
		Histbuf[Hbuffirst] = '\0';
		Hbuffirst = (Hbuffirst + 1) % HISTSIZE;

		if (Hbuffirst == Hbuflast)
			histfree ();
	}

	Inaquote = (Bquote || Dquote || Squote);

	if (Hptrfirst == Hptrlast)
	{
		histinit ();
		errmsg (nullstr, bufover);
		/* errmsg returns FALSE */
	}

	event_count++;
	return (TRUE);
}

/* histfree --- free the next queue pointer */

static void histfree ()
{
	Hptrlast = (Hptrlast + 1) % MAXHIST;

	Hbuflast = Histptr[Hptrlast];
	Histline++;
}

/* histfind --- find the start and length of a history pattern */

static int histfind (command, start, len)
char *command;
int *start, *len;
{
	char *p, c;
	int subseen;

	p = command + *start;
	c = *p++;

	*len = 0;
	if (c == NL || c == '\0')
		return (FALSE);

	/* skip leading non-history */
	while (c && c != HISTCHAR)
	{
		if (c == ESCAPE)
		{
			c = *p++;
			*start += 1;
		}

		if (c != '\0')
		{
			c = *p++;
			*start += 1;
		}
	}

	if (c == NL || c == '\0')
		return (FALSE);
	
	*len = 1;
	c = *p++;
	if (c == HISTLOOK)	/* !?...? */
	{
		*len += 1;
		c = *p++;
		while (c && c != HISTLOOK && c != NL)
		{
			if (c == ESCAPE)
			{
				c = *p++;
				*len += 1;
			}

			if (c != '\0')
			{
				c = *p++;
				*len += 1;
			}
		}
		if (c == HISTLOOK)
		{
			c = *p++;
			*len += 1;
		}
	}
	else if (digit (c) || c == '-')	/* !<num> */
	{
		if (c == '-')
		{
			c = *p++;
			*len += 1;

			if (! digit(c))
				errmsg (command + *start, h_illegal);
		}

		while (digit (c))
		{
			c = *p++;
			*len += 1;
		}
	}
	else	/* !<str> */
		while (c && c != HISTARG && c != HISTSUB && c != SP &&
				c != TAB && c != NL && c != HISTCHAR)
		{
			if (c == ESCAPE)
			{
				c = *p++;
				*len += 1;
			}

			if (c != '\0')
			{
				c = *p++;
				*len += 1;
			}
		}

	if (c == HISTARG)
	{
		*len += 1;
		c = *p++;
		while (c && digit (c))
		{
			*len += 1;
			c = *p++;
		}
		if (c == '-')
		{
			*len += 1;
			c = *p++;
		}
		if (c == '$')
		{
			*len += 1;
			c = *p++;
		}
		else
		{
			while (c && digit (c))
			{
				*len += 1;
				c = *p++;
			}
		}
	}

	while (c == HISTSUB)
	{
		*len += 1;
		subseen = 0;
		c = *p++;

		while (subseen < 2 && c != NL && c != '\0')
		{
			if (c == ESCAPE)
			{
				c = *p++;
				*len += 1;
			}

			if (c != '\0')
			{
				c = *p++;
				*len += 1;
			}

			if (c == HISTSUB)
				subseen++;
		}

		if (c == HISTSUB)
		{
			*len += 1;
			c = *p++;
			if (c == 'g' || c == 'G')
			{
				*len += 1;
				c = *p++;
			}
		}
	}

	if (c == HISTCHAR)
		*len += 1;
	
	return (TRUE);
}

/* histlook --- lookup the value of a history string */

static int histlook (str, sub)
char *str, *sub;
{
	char c;
	char *save, *p, *sp;
	char buf[BIGBUF], rep[BIGBUF];
	char new[BIGBUF];
	int i, j, val, si, flag, len, last;
	static int ctoi();

	save = sub;
	/*
	 * first attempt to find which command on which we are to operate
	 *
	 * the entire hstory format is as follows
	 *
	 * ! [<str> | <num> | ?<str>?] [`<num> [- [<num>]]] {^<str>^<str>^ [g]}
	 */
	
	si = 0;

	if (str[si] == HISTCHAR)
		si++;

	switch (str[si]) {
	case '\0':	/* ! */
	case NL:
	case HISTARG:	/* on these, retrive previous line, then break */
	case HISTSUB:
		if (Hptrfirst == Hptrlast)
			errmsg (nullstr, nohist);

		val = (Hptrfirst - Hptrlast + MAXHIST) % MAXHIST + Histline - 1;
		if (! histget (val, sub))
			errmsg (nullstr, internal);
		break;

	case '-':
	case '0':	/* !<num> */
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		val = ctoi (str, &si) - 1;	/* for 0-based indexing */
		if (! histget(val, sub))
			errmsg (str, h_notfound);
		break;
	
	case HISTLOOK:	/* ?<str>? */
		i = 0;
		si++;
		while (str[si] && str[si] != HISTLOOK)
		{
			if (str[si] == ESCAPE)
				si++;
			
			if (str[si])
				buf[i++] = str[si++];
		}
		buf [i] = '\0';
		if (str[si] == HISTLOOK)
			si++;
		if (buf[i-1] == NL)
			buf[--i] = '\0';
		
		flag = FALSE;
		val = (Hptrfirst - Hptrlast + MAXHIST) % MAXHIST + Histline - 1;
		*sub = '\0';
		while (histget (val, sub))
		{
			p = sub;
			c = *p++;
			while (c)
			{
				i = 0;
				while (c != '\0' && buf[i] != '\0' && c != buf[i])
				{
					/* skip non matching */
					c = *p++;
					if (*p == '\0')
						break;
				}
				
				sp = p;

				while (c && buf[i] && c == buf[i])
				{
					/* possibly matching */
					c = *p++;
					i++;
				}

				if (buf[i] == '\0')
				{
					/* did match */
					flag = TRUE;
					goto out;
				}

				p = sp;
				c = *p++;
			}
			val--;	/* search further back, next time around */
			*sub = '\0';
		}

	out:
		if (flag == FALSE)
			errmsg (str, h_notfound);
		break;

	default:	/* !<str> */
		i = 0;
		while (str[si] && str[si] != HISTARG && str[si] != HISTSUB)
		{
			if (str[si] == ESCAPE)
				si++;
			
			if (str[si])
				buf[i++] = str[si++];
		}
		buf[i] = '\0';

		flag = FALSE;
		val = (Hptrfirst - Hptrlast + MAXHIST) % MAXHIST + Histline - 1;
		while (histget (val, sub))
		{
			p = sub;
			c = *p++;
			while (c == SP || c == TAB)
				c = *p++;

			i = 0;
			while (buf[i] && buf[i] == c)
			{
				c = *p++;
				i++;
			}

			if (buf[i] == '\0')
			{
				flag = TRUE;	/* found it */
				break;	/* while */
			}
			val--;
		}
		if (flag == FALSE)
			errmsg (str, h_notfound);
		break;
	} /* end switch */

	j = length (sub) - 2;
	if (sub[j] == NL)
		sub[j] = '\0';

	/*
	 * ! [<str> | <num> | ? <str> ?] has now been parsed and the command
	 * line has been placed in "sub". Now see if the next character is a
	 * legal following character
	 */
	
	if (str[si] && str[si] != HISTARG && str[si] != HISTSUB && str[si] != NL)
		errmsg (str, badtoken);

	/* if there is no more to the history string, we are done */
	if (str[si] == NL || str[si] == '\0')
		return (TRUE);
	
	/*
	 * now check for possible argument substitution. This section parses
	 * [` <num>] and turns "sub" into the appropriate argument
	 */
	
	if (str[si] == HISTARG)		/* `<num>-<num> */
	{
		si++;

		if (! digit(str[si]) && str[si] != '-' && str[si] != '$')
			errmsg (str, badarg);
		
		/* determine the last argument */
		p = sub;
		last = -1;
		/* count arguments, last will be val of $ */
		histarg (p, &len);
		while (len > 0)
		{
			last++;
			p += len;
			histarg (p, &len);
		}

		if (str[si] == '-')	/* default to arg 1 */
			val = 1;
		else if (digit(str[si]))
			val = min (ctoi(str, &si), last + 1);
		else
		{
			/* $ */
			val = last;

			if (str[si] != '$')
			{
				errmsg (str, internal);
			}
			else
				si++;
		}

		p = sub;
		for (i = val; i > 0; i--)	/* delete preceding arguments */
		{
			histarg (p, & len);
			p += len;
		}

		/* p points to beginning of first wanted arg */
		/* remove leading blanks */
		c = *p++;
		while (c == SP || c == TAB)
			c = *p++;
		
		sub = p - 1;


		if (str[si] == '-')
		{
			si++;
			if (digit(str[si]))
				val = min (ctoi (str, &si), last) - val + 1;
			else
			{
				val = last - val + 1;

				if (str[si] != '\0')
					si++;
			}

			p = sub;
			histarg (p, & len);
			while (val > 0 && len > 0)
			{
				val--;
				p += len;
				histarg (p, &len);
			}
			*p = '\0';
		}
		else
		{
			histarg (sub, & len);
			sub [len] = '\0';
		}
	}

	/* move everything to beginning of buffer */
	if (save != sub)
	{
		movstr (sub, save);
		sub = save;
	}


	/*
	 * check that the remaining characters represent
	 * legal following characters
	 */

	if (str[si] && str[si] != HISTSUB && str[si] != NL)
		errmsg (str, badtoken);
	
	/* check for no substitutions and return if we are done */
	if (str[si] && str[si] != HISTSUB)
		return (TRUE);
	
	/* keep performing substitutions until there are no more */

	while (str[si] == HISTSUB)
	{
		i = 0;
		si++;
		flag = FALSE;
		/* buf is what to look for */
		while (str[si] && str[si] != HISTSUB)
		{
			if (str[si] == ESCAPE)
				si++;
			
			if (str[si])
				buf[i++] = str[si++];
		}
		buf[i] = '\0';

		i = 0;
		if (str[si])
			si++;
		
		/* rep is replacement */
		while (str[si] && str[si] != HISTSUB)
		{
			if (str[si] == ESCAPE)
				si++;
			
			if (str[si])
				rep[i++] = str[si++];
		}
		rep[i] = '\0';

		if (str[si] == HISTSUB)
			si++;
		
		if (str[si] == 'g' || str[si] == 'G')
		{
			flag = TRUE;
			si++;
		}

		j = 0;		/* j indexes new */
		p = sub;
		c = *p++;
		sp = p;		/* save position for backing up */
		while (c != '\0')
		{
			i = 0;
			while (c && c != buf[i])
			{
				/* copy what doesn't match */
				new[j++] = c;
				c = *p++;
				sp = p;
			}

			while (c && buf[i] && c == buf[i])
			{
				/* partial matching */
				c = *p++;
				i++;
			}

			if (buf[i] == '\0')
			{
				/* successful match */
				char *cp = rep;

				while (*cp)
					new[j++] = *cp++;
					/* put in replacement text */

				if (flag == FALSE)	/* just 1 replacement */
				{
					new[j++] = c;
					while (*p)
						new[j++] = *p++;
						/* copy the rest */
					break;
				}
			}
			else if (c != '\0')
			{
				/* back up and try again */
				new[j++] = *(sp - 1);
				p = sp;
				c = *p++;
				sp = p;
			}
		}
		new[j] = '\0';
		movstr (new, sub);
		/* now look for next substitution */
	}

	if (save != sub)
	{
		movstr (sub, save);
		sub = save;
	}
	
	j = length (sub) - 2;
	if (sub[j] == NL)
		sub[j] = '\0';

	return (TRUE);
}

/* histget --- get a specified string from the history buffers */

static int histget (hp, sub)
int hp;
char *sub;
{
	char buf[BIGBUF];
	int i, j, maxinx, hval;

	*sub = '\0';
	maxinx = (Hptrfirst - Hptrlast + MAXHIST) % MAXHIST + Histline - 1;
	if (hp < Histline || hp > maxinx)		/* out of range */
		return (FALSE);
	
	hval = ((hp - Histline + Hptrlast - 1) % MAXHIST) + 1;
	for (i = Histptr[hval]; Histbuf[i] != '\0'; )
	{
		int k; 

		j = 0;
		while (Histbuf[i] != '\0' && j < sizeof(buf) - 1)
		{
			buf[j] = Histbuf[i];
			i = (i + 1) % HISTSIZE;
			j++;
		}

		buf[j] = '\0';
		/* strcat (sub, buf); */
		movstr (buf,
			sub + (((k = length (sub) - 1) <= 0 ? 0 : k)));
	}

	return (TRUE);
}

/* histarg --- return the last position of the next argument */

static void histarg (ptr, len)
char *ptr;
int *len;
{
	char *p;
	char c;
	int bracket, paren, brace, squote, dquote, bquote, skip;

	p = ptr;
	*len = 0;
	skip = FALSE;
	bracket = paren = brace = squote = dquote = bquote = 0;

	repeat
	{
		*len += 1;
		c = *p++;
		while (skip == FALSE && (c == SP || c == TAB))
		{
			c = *p++;
			*len += 1;
		}

		skip = TRUE;
		switch (c) {
		case ESCAPE:
			c = *p++;
			*len += 1;
			break;

		case '[':
			if (squote == 0 && dquote == 0 && bquote == 0)
				bracket++;
			break;

		case ']':
			if (squote == 0 && dquote == 0 && bquote == 0)
				bracket--;
			break;

		case '(':
			if (squote == 0 && dquote == 0 && bquote == 0)
				paren++;
			break;

		case ')':
			if (squote == 0 && dquote == 0 && bquote == 0)
				paren--;
			break;

		case '{':
			if (squote == 0 && dquote == 0 && bquote == 0)
				brace++;
			break;

		case '}':
			if (squote == 0 && dquote == 0 && bquote == 0)
				brace--;
			break;

		case '\'':
			if (dquote == 0 && bquote == 0)
				squote = 1 - squote;
			break;

		case '"':
			if (squote == 0 && bquote == 0)
				dquote = 1 - dquote;
			break;

		case '`':
			if (dquote == 0 && squote == 0)
				bquote = 1 - bquote;
			break;

		}
	} until (c == '\0' ||
		((c == SP || c == TAB) && paren == 0 && brace == 0 &&
		bracket == 0 && squote == 0 && dquote == 0 && bquote == 0));

	*len -= 1;
	return;
}

/* ctoi --- character to integer conversion, updates indices ala Fortrash */

static int ctoi (str, inx)
register char *str;
register int *inx;
{
	register int ret = 0;
	int neg = 0;

	if (str[*inx] == '-')
	{
		neg = 1;
		*inx += 1;
	}

	while (digit (str[*inx]))
	{
		ret = 10 * ret + str[*inx] - '0';
		*inx += 1;
	}

	return (neg ? -ret : ret);
}

/* min --- real function to return min of two numbers */

static int min (a, b)
register int a, b;
{
	return (a < b ? a : b);
}

/* histinit --- reinitialize history buffers */

static void histinit ()
{
	Hbuffirst = Hbuflast = Hptrfirst = Hptrlast = Histline = 0;
	event_count = 1;
}

/* histsave --- save history command lines */

histsave (file)
char *file;
{
	int fd, status, junk;

	if ((flags&nohistflg) != 0)
		return (FALSE);

	if ((flags&prompt) == 0)
		return (FALSE);

	if ((fd = creat (file, 0600)) < 0)	/* delete previous contents */
		return (FALSE);
	
	status = TRUE;

	junk = MAXHIST;
	if (write (fd, & junk, sizeof (junk)) != sizeof (junk))
		status = FALSE;

	junk = HISTSIZE;
	if (status == TRUE &&
		write (fd, & junk, sizeof (junk)) != sizeof (junk))
		status = FALSE;

	if (status == TRUE &&
		write (fd, &Hbuffirst, sizeof(Hbuffirst)) != sizeof (Hbuffirst))
		status = FALSE;

	if (status == TRUE &&
		write (fd, & Hbuflast, sizeof (Hbuflast)) != sizeof (Hbuflast))
		status = FALSE;

	if (status == TRUE &&
		write (fd, &Hptrfirst, sizeof(Hptrfirst)) != sizeof (Hptrfirst))
		status = FALSE;

	if (status == TRUE &&
		write (fd, & Hptrlast, sizeof (Hptrlast)) != sizeof (Hptrlast))
		status = FALSE;

	if (status == TRUE &&
		write (fd, Histptr, sizeof(Histptr)) != sizeof (Histptr))
		status = FALSE;

	if (status == TRUE &&
		write (fd, Histbuf, sizeof(Histbuf)) != sizeof (Histbuf))
		status = FALSE;
	
	close (fd);

	if (status == FALSE)
		unlink (file);	/* remove entirely */
	
	return (status);
}

/* histrest --- restore a history save file */

int histrest (file)
char *file;
{
	int fd, status, junk;

	if (flags&nohistflg)
		return (FALSE);

	if ((flags&prompt) == 0)
		return (FALSE);

	if ((fd = open (file, 0)) < 0)	/* open for reading */
		return (FALSE);

	status = TRUE;
	if (read (fd, & junk, sizeof (junk)) != sizeof (junk) ||
			junk != MAXHIST)
		status = FALSE;

	if (status == TRUE && read (fd, & junk, sizeof (junk)) != sizeof (junk)
			|| junk != HISTSIZE)
		status = FALSE;

	if (status == TRUE &&
		read (fd, &Hbuffirst, sizeof (Hbuffirst)) != sizeof (Hbuffirst))
		status = FALSE;

	if (status == TRUE &&
		read (fd, & Hbuflast, sizeof (Hbuflast)) != sizeof (Hbuflast))
		status = FALSE;

	if (status == TRUE &&
		read (fd, &Hptrfirst, sizeof (Hptrfirst)) != sizeof (Hptrfirst))
		status = FALSE;

	if (status == TRUE &&
		read (fd, & Hptrlast, sizeof (Hptrlast)) != sizeof (Hptrlast))
		status = FALSE;

	if (status == TRUE &&
		read (fd, Histptr, sizeof(Histptr)) != sizeof (Histptr))
		status = FALSE;

	if (status == TRUE &&
		read (fd, Histbuf, sizeof(Histbuf)) != sizeof (Histbuf))
		status = FALSE;
	
	Histline = - (Hptrfirst - Hptrlast + MAXHIST) % MAXHIST;
	event_count = 1;
	close (fd);
	return (status);
}

/* history --- print history buffer, or save or restore buffer to file */

int history (argc, argv)
int argc;
char **argv;
{
	int i;
	char *hf;
	int (*hfp)();

	if ((flags&nohistflg) != 0)
	{
		if (flags&prompt)
			prs ("history processing not enabled\n");
		return 1;	/* failure */
	}

	if ((flags & prompt) == 0)	/* shell file */
		return 1;

	if (argc == 1)
	{
		int j = Histline + 1;
		int k, l, m; 
		int neg;
		char *cp;

#define outstr(s)	for (cp = s; *cp; cp++) \
				if (*cp == NL && *(cp+1))  \
					prs_buff ("\\n"); \
				else \
					prc_buff (*cp)


		for (i = Hptrlast; i != Hptrfirst; i = (i + 1) % MAXHIST)
		{
			neg = FALSE;

			k = j++;
			if (k < 0)
			{
				neg = TRUE;
				k = -k;
			}
			itos (k);
			l = length (numbuf) - 1;
			for (m = 3 - l; m > 0; m--)
				prc_buff (SP);
			prc_buff (neg ? '-' : SP);
			prs_buff (numbuf);
			prc_buff (COLON);
			prc_buff (SP);
			/*
			 * make sure that what we're printing
			 * doesn't wrap around the history buffer.
			 */
			k = (i % MAXHIST) + 1;
			if ((k != Hptrfirst && Histptr[i] < Histptr[k]) ||
				(k == Hptrfirst && Histptr[i] < Hbuffirst))
				outstr (& Histbuf[Histptr[i]]);
			else
			{
				/* saved text wraps around */
				for (l = Histptr[i]; l <= HISTSIZE - 1 &&
						Histbuf[l] != '\0'; l++)
					if (Histbuf[l] == NL
			&& Histbuf[l + 1 <= HISTSIZE - 1 ? l + 1 : 0] != '\0')
						prs_buff ("\\n");
					else
						prc_buff (Histbuf[l]);
				
				if (Histbuf[HISTSIZE - 1] != '\0')
					outstr (Histbuf);
			}
		}
		return 0;
	}
	else if (eq (argv[1], dashi))
	{
		histinit ();
		return 0;
	}
	else if (eq (argv[1], dashr))
		hfp = histrest;
	else if (eq (argv[1], dashs))
		hfp = histsave;
	else
	{
		prs(argv[1]);
		prc(COLON);
		prs(h_badopt);
		newline();
		return 1;
	}

	if (argc >= 3)
		hf = argv[2];
	else
		hf = histfnod.namval;
	
	return ((*hfp)(hf) != 0);	/* do a save or restore */
}
