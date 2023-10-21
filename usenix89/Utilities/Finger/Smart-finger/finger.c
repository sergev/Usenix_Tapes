static char* id = "@(#)finger.c - main, strsav; mcvoy@rsch.wisc.edu";

/* finger - provide front end for finger that loads aliases.
 *
 * Look in ~/.fingerc for aliases.  You could link this to .mailrc.
 *
 * An alias is the regular expression:   
 *
 *       ^alias[ <tab>]+name[ <tab>]+full_name
 *
 * and finger name gets translated to finger fullname.  
 * Exception: any fullname that contains a "!" is ignored (can't do uucp,
 * only internet).
 *
 * Options: -I ignores the dotfiles; a good way to finger a local john instead
 * of an aliased john.
 */

# include      <stdio.h>
# include      <ctype.h>
# define       FINGER         "/usr/ucb/finger"

main(ac, av, ev)
    char** av;
    char** ev;
{
    char fingerc[255];
    char buf[500];
    register i;
    FILE* f = (FILE*)-1;
    char* strsav();

    sprintf(fingerc, "%s/.fingerc", getenv("HOME"));

    if (!strcmp(av[1], "-I")  ||  !(f = fopen(fingerc, "r"))) {
	if (f == (FILE*)-1) {  /* shift av down. */
	    register i;

	    for (i=1; i<ac; i++)
		av[i] = av[i+1];
	}
	execve(FINGER, av, ev);
	perror(FINGER);
    }

    /* stupid alg: scan the file for each av, but there's usually only one. */
    for (i=1; i<ac; i++) {
	rewind(f);
	while (fgets(buf, sizeof(buf), f)) {
	    register char* s;
	    register char* t;
	    register len = strlen(av[i]);

	    if (strncmp(buf, "alias", 5))
		continue;
	    for (s=buf + 5; *s && isspace(*s); s++)
		;
	    if (!strncmp(s, av[i], len)  &&  isspace(*(s + len))) {
		s += len;
		for ( ; *s && isspace(*s); s++)
		    ;
		for (t=s; *t && !isspace(*t); t++)
		    ;
		*t = NULL;
		if (!index(s, '!')) {
		    fprintf(stderr, "(%s: %s)\n", av[i], s);
		    av[i] = strsav(s);
		    break; /* while, get next i */
		}
	    }
	}
    }
    execve(FINGER, av, ev);
    perror(FINGER);
}

char* 
strsav(s)
    register char* s;
{
    char* malloc();
    char* strcpy();
    register char* t = malloc(strlen(s) + 1);
    return strcpy(t, s);
}
