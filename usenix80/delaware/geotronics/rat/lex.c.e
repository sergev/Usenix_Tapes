281a
#define	MAXTOK	73		/* largest string token */

int	strptr[NUMSTR];		/* indexes into "tables" */
char	tables[MAXSTR];		/* hold-back string data */

strcode ()
{
	char name[MAXTOK], text[MAXTOK];
	int c, n, q, t;

	while ((c = getc ())==' ' || c=='\t')
		;		/* skip whitespace */
	peek = c;

	if (!( (c>='a'&&c<='z') || (c>='A'&&c<='Z') ))
		goto badname;	/* not alphanumeric */

	for (n = 0;  n<MAXTOK-1 && ( ((c = getc ())>='a'&&c<='z') ||
			(c>='A'&&c<='Z') || (c>='0'&&c<='9') );  n++)
		name[n] = c;	/* alphanumeric name */
	peek = c;
	name[n] = '\0';

	if (n <= 0)  {
   badname:
		error ("illegal string name");
		goto skipend;
	}

	while ((c = getc ())==' ' || c=='\t')
		;		/* skip whitespace */

	if ((q = c)!='"' && q!='\'')  {
		peek = c;
		goto badtext;
	}

	for (t = 0;  t<MAXTOK-1 && (c = getc ())!=q && c!='\n' &&
			c!='\0';  t++)
		text[t] = c;	/* literal text */
	peek = c;
	text[t] = '\0';

	if (c != q)  {
   badtext:
		error ("illegal string text");
		goto skipend;
	}

	if (lastr++>=NUMSTR || lasts+n+t>MAXSTR)  {
		lastr--;
		error ("too many strings");
		goto skipend;
	}

	strptr[lastr] = lasts;		/* start of storage */

	n = 0;
	do  {
		tables[lasts++] = name[n];
	} while (name[n++] != '\0');

	t = 0;
	do  {
		tables[lasts++] = text[t];
	} while (text[t++] != '\0');

	outcode ("\tinte ger*1 ");
	outcode (name);
	ptc ('(');
	outnum (t);		/* length of string incl. EOS */
	ptc (')');
	outcode (0);		/* end line */

   skipend:
	while ((c = getc ())!='\n' && c!='\0')
		;		/* discard rest of input line */
	peek = c;
}

endstr ()
{
	int c, i, k, lt, n, t;

	for (k = 0;  k <= lastr;  k++)  {	/* each string */
		outcode ("\tdata ");

		t = n = strptr[k];	/* -> name */
		while (tables[t++] != '\0')
			;	/* skip over name */
		for (lt = 0;  tables[t+lt] != '\0';  lt++)
			;	/* find length of text */

		for (i = 0;  i < lt;  i++)  {
			outcode (&tables[n]);	/* name */
			ptc ('(');
			outnum (i+1);
			outcode ("),");
		}
		outcode (&tables[n]);
		ptc ('(');
		outnum (lt+1);
		outcode (") /");

		for (i = 0;  i < lt;  i++)  {
			outnum (tables[t++]);	/* ASCII code */
			ptc (',');
		}
		outcode ("0/");		/* EOS */
		outcode (0);		/* end line */
	}

	lastr = -1;	lasts = 0;	/* reset for next batch */
}
.
277c
		if (c == '(')
			nlpar++ ;
		else if (c == ')')
			nlpar-- ;
	}
	if (nlpar >= 0)  {
		error ("missing ) in define");
		nameptr[index] = names[index] = 0;	/* return the slot */
		return;
	}
	str[--i] = '\0';
.
275c
	nlpar = 0;
	for( i=0; nlpar>=0 && (c=getc())!='\n' && c!='\0'; i++ )  {
.
263a
	for (nstr = 0;  c = getc (); )  {
		if (c == ' ' || c == '\t')
			continue;
		if (c == ',')
			break;
		if (c == '\n')  {
			error ("missing , in define");
			return;
		}
		str[nstr++] = c;
	}
.
262a
	while ((c = getc ()) == ' '  ||  c == '\t')
		;
.
258,261c
	if (c != '(')  {
		error ("missing ( in define");
		peek = c;
		while ((c = getc ())  &&  c != '\n')
			;
		return;
.
253c
	int c,i,index,nlpar;
.
246c
#define	MAXNAMES	200
.
217c
		&& c!=';' && c!='(' && c!=')' && c!='\0' ; )
.
208,211c
	return(NEWDO);
.
201a
	} else if (keytran[type] == PROGRAM)  {
		outcode ("c\tprog:");
		while ((c = getc ())!='\n' && c!='\0')
			ptc (c);	/* duplicate rest of line */
		peek = c;
		outcode(0);		/* end line */
		goto top;
	} else if (keytran[type] == REFERENCE)  {
		outcode ("c\tref:");
		while ((c = getc ())!='\n' && c!='\0')
			ptc (c);	/* duplicate rest of line */
		peek = c;
		outcode (0);		/* end line */
		goto top;
.
195a
	if (keytran[type] == STRING)  {
		strcode ();		/* hold back data */
		goto top;
	} else if (lastr >= 0)
		endstr ();		/* dump held-back strings */

.
150a
		/* try prepending standard directory */
		for ( ;  i >= 0;  i--)	/* make room */
			fname[i + sizeof prefix/sizeof prefix[0] - 1] =
					fname[i];
		while (prefix[++i] != '\0')
			fname[i] = prefix[i];	/* prepend */
		fd = copen (fname, 'r');	/* try again */
	}
	if (fd < 0)  {
.
148c
	dotseen = 0;
	for(i=0; (fname[i]=c=getc())!='\n' && c!=';' && c!=' ' && c!='\t'
			&& c!='\0'; i++)
		if (c == '.')
			dotseen++ ;
	if (! dotseen)  {		/* supply missing extension */
		fname[i++] = '.';  fname[i++] = 'r';
	}
.
145a

.
144c
	int c, dotseen, i;
.
142a
char prefix[] "/usr/include/";

.
103a
		if (c == '_')  {	/* continuation */
			while ((c = cgetc (fd)) != '\n'  &&
			       c != '\0')
				;	/* discard rest of line */
			if (c == '\0')  {
				gcbuf[gcp] = '\0';
				break;	/* EOF */
			} else {
				gcp--;	/* pretend nothing happened */
				continue;
			}
		}
.
65a
	while (nextchar == '\014')
		nextchar = gchar ();	/* ignore formfeeds */
.
56a
	lastr = -1;	lasts = 0;
.
40a
int	lastr	-1;		/* indexes "strptr" */
int	lasts	0;		/* indexes "tables" */

#define	MAXSTR	1000		/* "tables" size */
#define	NUMSTR	100		/* "strptr" size */

.
30a
	STRING, STRING,
	PROGRAM, PROGRAM,
	ASSERT, ASSERT,
	REFERENCE, REFERENCE,
.
14a
	"string", "STRING",
	"program", "PROGRAM",
	"assert", "ASSERT",
	"reference", "REFERENCE",
.
0a
#
/*
 * lex.c - Ratfor lexical analyzer
 *
 *	modified 28-Apr-1980 by D A Gwyn:
 *	1) string statement supported;
 *	2) define syntax changed;
 *	3) include files now get ".r" extension if needed;
 *	4) underscore continuation supported;
 *	5) MAXNAMES increased;
 *	6) form-feeds are ignored;
 *	7) program, reference statements processed.
 *
 */
.
w
q
