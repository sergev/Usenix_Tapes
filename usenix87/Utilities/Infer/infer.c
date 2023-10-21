/* infer --- inference engine */

# include <stdio.h>
# include <ctype.h>
# include "infer.h"

# define TRUTHVAL(E)    ((E->type & NOT) ? (E->str->val == TRUE) ? FALSE : TRUE : (E->str->val))

char *progname;
RULE_T *Rule[MAXRULE];
int nrules = 0;
RULE_T *why[MAXWHY];
int nwhy = 0;
STR *SP = 0;
int verbose;

main (argc, argv)
int argc;
char **argv;
{
    FILE *fp;

    progname = argv[0];
    while (argv[1][0] == '-') { /* check for options */
	if (argv[1][1] == 'v')  /* I know only one */
	    verbose++;
	else
	    fprintf (stderr, "%s: unknown arg \"%s\"\n", progname,
	      argv[1]);
	argc--;
	argv++;
    }

    if (argc !=2) {
	fprintf (stderr, "Usage: %s [-v] file\n", progname);
	exit (1);
    }

    if ((fp = fopen (argv[1], "r")) == NULL) {
	fprintf (stderr, "%s: can't open %s (%s)\n", progname, argv[1],
	  sys_errlist[errno]);
	exit (1);
    }

    compile (fp);
    fclose (fp);
    return (infer ());
}

/* infer --- inference engine */

infer ()
{
    register int i;
    int proved;

    for (i = 0; i < nrules; i++) {      /* verify each CON */
	if (Rule[i]->con == 0) {
	    fprintf (stderr, "%s: RULE has no THENs:\n", progname);
	    prrule (Rule[i], stderr);
	    exit (1);
	}
	if (TRUTHVAL (Rule[i]->con) == TRUE)
	    continue;

	if (verify (Rule[i]) == TRUE) {
	    register ELEMENT_T *e;
	    for (e = Rule[i]->con; e; e = e->next) {
		if (e->type & ROUTINE) {
		    if (e->str->val == TRUE)
			continue;
		    if (run (e) == TRUE) {
			e->str->val = TRUE;
			if (e->type & HYP) {
			    printf ("Conclusion\n");
			    return (0);
			}
		    }
		    else {
			e->str->val = FALSE;
		    }
		}
		else {
		    e->str->val = TRUE;
		    proved = TRUE;
		    printf ("I infer that: %s\n", e->str->p);
		    if (e->type & HYP) {
			printf ("Conclusion\n");
			return (0);
		    }
		}
	    }
	}
    }
    if (proved == FALSE) {
	printf ("I can't prove anything!!!\n");
	return (1);
    }
    return (0);
}

/* verify --- verify a CON.  May be called recursivly */

verify (rule)
register RULE_T *rule;
{
    register ELEMENT_T *e;
    register int i;

    pushwhy (rule);
    if (rule->con->str->val == TRUE) {
	popwhy ();
	return (TRUE);
    }

    if (verbose)
	prrule (rule, stdout);

    if (rule->ant == 0) {
	fprintf (stderr, "%s: RULE has no IFs:\n", progname);
	prrule (rule, stderr);
	popwhy ();
	return (TRUE);
    }

    for (e = rule->ant; e; e = e->next) {   /* for each ANT */
	for (i = 0; i < nrules; i++)
	    if (e->str == Rule[i]->con->str)    /* this ANT is a CON */
		break;
	if (i == nrules) {      /* NOT a CON */
	    if (prove (e) == TRUE)
		continue;
	    else {
		popwhy ();
		return (FALSE);
	    }
	}
	else {
	    int anytrue = 0;
	    for (i = 0; i < nrules; i++) {
		if (e->str == Rule[i]->con->str) {  /* match */
		    int ret;
		    if (Rule[i]->vfy == 1) {
			fprintf (stderr, "%s: Circular logic in Rules! Rules are:\n", progname);
			prcirc ();
			exit (1);
		    }
		    Rule[i]->vfy = 1;
		    ret = verify (Rule[i]);
		    Rule[i]->vfy = 0;
		    if (ret == TRUE) {
			register ELEMENT_T *f;
			anytrue++;
			for (f = Rule[i]->con; f; f = f->next) {
			    if (f->str->val == UNKNOWN) {
				printf ("I infer that: %s\n", f->str->p);
				f->str->val = TRUE;
			    }
			}
			if (prove (e) == TRUE)
			    break;
			else {
			    popwhy ();
			    return (FALSE);
			}
		    }
		}
	    }
	    if (!anytrue)
		if (e->type & NOT)
		    continue;
		else {
		    popwhy ();
		    return (FALSE);
		}
	    else
		if (e->type & NOT) {
		    popwhy ();
		    return (FALSE);
		}
		else
		    continue;
	}
    }               /* don't pop the why stack if TRUE */
    return (TRUE);
}

/* prove --- prove this ANT (may be already proven) */

prove (e)
ELEMENT_T *e;
{
    if (e->str->val != UNKNOWN)
	return (TRUTHVAL (e));
    if (e->type & ROUTINE)
	return (run (e));
    else
	return (askval (e));
}

/* askval --- get truth from user */

askval (e)
register ELEMENT_T *e;
{
    char line[80];
    register int value;
    
    value = UNKNOWN;
    for (;;) {
	printf ("Is the following statement true?\n");
	printf ("%s : ", e->str->p);
	if (fgets (line, sizeof (line), stdin) == NULL)
	    *line = 'q';
	if (isupper (*line))
	    *line = tolower (*line);
	switch (*line) {
	case 'y':
	case 't':
	    value = TRUE;
	    break;
	case 'n':
	case 'f':
	    value = FALSE;
	    break;
	case 'q':
	    printf ("OK, Bye!\n");
	    exit (0);
	    break;
	case 'w':
	    showwhy ();
	    continue;
	}
	if (value != UNKNOWN)
	    break;
	printf ("How about typeing t/f/y/n?\n");
    }
    e->str->val = value;
    return (TRUTHVAL (e));
}

/* run --- execute this string */

run (e)
register ELEMENT_T *e;
{
    register int value, ret;
    
    if ((ret = system (e->str->p)) < 0) {
	printf ("%s: can't execute %s (%s) returning TRUE\n", progname,
	  e->str->p, sys_errlist[errno]);
	value = TRUE;
    }
    else if (ret == 0)
	value = TRUE;
    else
	value = FALSE;

    e->str->val = value;
    return (TRUTHVAL (e));
}

/* prrule --- print this rule in a readable form */

prrule (rule, fp)
register RULE_T *rule;
FILE *fp;
{
    register ELEMENT_T *e;
    char *prval (), *prtype ();

    for (e = rule->ant; e; e = e->next)
	fprintf (fp, "%s %s (%s)\n", prtype (e->type, "IF"), e->str->p,
	  prval (e));
    for (e = rule->con; e; e = e->next)
	fprintf (fp, "%s %s (%s)\n", prtype (e->type, "THEN"), e->str->p,
	  prval (e));
    fprintf (fp, "\n");
}

/* prval --- return the char representation of this value */

char *
prval (e)
register ELEMENT_T *e;
{

    if (e->str->val == UNKNOWN)
	return ("UNKNOWN");
    else if (e->str->val == TRUE || e->str->val == FALSE) {
	if (TRUTHVAL (e) == TRUE)
	    return ("TRUE");
	else
	    return ("FALSE");
    }
    else
	return ("BADVAL");
}

/* prtype --- return the char representation of this type */

char *
prtype (type, word)
register short type;
register char *word;
{
    static char str_type[20];

    strcpy (str_type, word);
    if (type & NOT)
	strcat (str_type, "NOT");
    if (type & ROUTINE)
	strcat (str_type, "RUN");
    if (type & HYP)
	strcat (str_type, "HYP");
    return (str_type);
}

/* pushwhy --- push this rule onto the "why" stack */

pushwhy (r)
register RULE_T *r;
{
    if (nwhy >= MAXWHY) {
	fprintf (stderr, "%s: blew why stack!\n", progname);
	exit (1);
    }
    why[nwhy++] = r;
}

/* popwhy --- pop a value off of the "why" stack */

popwhy ()
{
    if (--nwhy < 0) {
	fprintf (stderr, "%s: why stack underflow!\n", progname);
	exit (1);
    }
}

/* showwhy --- print the details of the "why" stack */

showwhy ()
{
    register int i;
    
    for (i = 0; i < nwhy; i++)
	prrule (why[i], stdout);
}

/* prcirc --- print the details of the circular loop */

prcirc ()
{
    register int i;
    
    for (i = 0; i < nrules; i++)
	if (Rule[i]->vfy == 1)
	    prrule (Rule[i], stderr);
}
