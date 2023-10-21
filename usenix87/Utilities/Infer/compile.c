/* compile --- compile the rules into the internal form */

# include <stdio.h>
# include <ctype.h>
# include "infer.h"

int Curline;
int State;

compile (fp)
register FILE *fp;
{
    register int key;
    char line[STRSIZE];
    STR *savestr ();

    Curline = 0;
    State = ANT;
    rulealloc ();
    while (fgets (line, sizeof (line), fp)) {
	Curline++;
	squeeze (line, '\n');
	stripbl (line);
	if (verbose)
	    printf ("%4d    %s\n", Curline, line);
	key = getkey (line);
	if (key == NONE)
	    fprintf (stderr, "%s: no keyword found on line %d\n",
	      progname, Curline);
	else if (key == COMMENT)
	    ;
	else
	    push (key, savestr (line));
    }
}

/* newstate --- if switching from CON to ANT, start a new rule */

newstate (new)
register int new;
{
    if (new != ANT && new != CON) {     /* paranoia */
	fprintf (stderr, "%s: bad new val: %d\n", new);
	exit (1);
    }

    if (State != new) {
	if (State == CON)
	    rulealloc ();
	State = new;
    }
}

/* push --- add an element to this rule */

push (type, ptr)
register int type;
register STR *ptr;
{
    register ELEMENT_T *e, *last;

    if (ptr == 0)   /* keyword only, ignore */
	return;
    e = (ELEMENT_T *) emalloc ((unsigned) sizeof (ELEMENT_T));
    e->str = ptr;
    e->type = type;
    e->next = 0;
    if (State == CON) {
	if (Rule[nrules-1]->con == 0) /* first element */
	    Rule[nrules-1]->con = e;
	else                          /* place on end */
	    for (last = Rule[nrules-1]->con; last; last = last->next)
		if (last->next == 0) {
		    last->next = e;
		    break;
		}
    }
    else {
	if (Rule[nrules-1]->ant == 0)
	    Rule[nrules-1]->ant = e;
	else
	    for (last = Rule[nrules-1]->ant; last; last = last->next)
		if (last->next == 0) {
		    last->next = e;
		    break;
		}
    }
}

/* savestr --- skip 1st word, save rest of line in str buffer */

STR *
savestr (line)
char *line;
{
    register char *s;
    register STR *sp;

    for (s = line; *s; s++)     /* skip to 1st space */
	if (*s == ' ')
	    break;

    while (isspace (*s))        /* skip to 1st non-space */
	s++;

    if (*s == 0) {
	fprintf (stderr, "%s: line %d has nothing but a keyword\n",
	  progname, Curline);
	return (0);
    }

    for (sp = SP; sp; sp = sp->next)    /* is string already present? */
	if (strcmp (sp->p, s) == 0)
	    return (sp);

    sp = (STR *) emalloc ((unsigned) sizeof (STR)); /* new string */
    sp->p = emalloc ((unsigned) strlen (s)+1);
    strcpy (sp->p, s);
    sp->val = UNKNOWN;
    sp->next = SP;      /* place at head */
    SP = sp;
    return (sp);
}

/* getkey --- determine the keyword on this line */

getkey (line)
char *line;
{
    register int i;
    register char *s, *p;
    char word[20];
    static struct {
	char    *name;
	short   type;
	short   newstate;
    } keytab[] = {              /* these are sorted based on frequency */
	"THEN", STRING, CON,    /* of use in 3 files I had access to */
	"AND", STRING, ANT,     /* change at will */
	"IF", STRING, ANT,
	"ANDRUN", ROUTINE, ANT,
	"ANDNOT", STRING|NOT, ANT,
	"THENHYP", STRING|HYP, CON,
	"IFRUN", ROUTINE, ANT,
	"ANDIF", STRING, ANT,
	"IFNOT", STRING|NOT, ANT,
	"THENRUNHYP", ROUTINE|HYP, CON,
	"THENRUN", ROUTINE, CON,
	"ANDIFRUN", ROUTINE, ANT,
	"ANDNOTRUN", ROUTINE|NOT, ANT,
	"IFNOTRUN", ROUTINE|NOT, ANT,
	"ANDTHEN", STRING, CON,
	"ANDTHENHYP", STRING|HYP, CON,
	"ANDTHENRUN", ROUTINE, CON,
	"ANDTHENRUNHYP", ROUTINE|HYP, CON,
	0,  0, 0
    };

    if (line[0] == COMMENT_CHAR)
	return (COMMENT);

    for (p = word, s = line; *s; s++) /* copy chars into word */
	if (isupper (*s))
	    *p++ = *s;
	else
	    break;
    *p = 0;
    
    for (i = 0; keytab[i].name; i++)    /* look for match */
	if (strcmp (word, keytab[i].name) == 0) {
	    newstate (keytab[i].newstate);
	    return (keytab[i].type);
	}

    return (NONE);
}

/* squeeze --- squeeze all c from s */

squeeze (s, c)
register char *s;
register char c;
{
    register char *p;

    for (p = s; *s; s++)
	if (*s != c)
	    *p++ = *s;
    *p = '\0';
}

/* stripbl --- remove trailing blanks from s */

stripbl (s)
register char *s;
{
    register char *p;

    for (p = s; *s; s++)
	if (*s != ' ')
	    p = s+1;

    *p = 0;
}

/* rulealloc --- allocate a new rule, and advance the pointer */

rulealloc ()
{
    if (nrules >= MAXRULE) {
	fprintf (stderr, "%s: too many rules!\n", progname);
	exit (1);
    }

    Rule[nrules] = (RULE_T *) emalloc ((unsigned) sizeof (RULE_T));
    Rule[nrules]->ant = 0;
    Rule[nrules]->con = 0;
    Rule[nrules]->vfy = 0;
    nrules++;
}

/* emalloc --- call malloc and check return */

char *
emalloc (n)
unsigned n;
{
    char *p, *malloc ();

    if ((p = malloc (n)) == NULL) {
	fprintf (stderr, "%s: Out of memory!\n", progname);
	exit (1);
    }
    return (p);
}
