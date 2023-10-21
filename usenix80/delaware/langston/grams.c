#include    <stdio.h>
/*
**      GRAMS -- Anagram exerciser
** Compile: cc -O -q grams.c -lP -lS -o grams
*/

char    *whatsccs   "@(#)grams.c	1.1  last mod 1/29/80 -- (c) psl 1979";

#define DICTFIL "/sys/games/.../bogwds"              /* default dictionary */
#define MONFIL  "/sys/games/.../grams.mon"            /* new words learned */
#define INFOFIL "/sys/games/.../gramsinfo"                 /* instructions */
#define RECFIL  "/sys/games/.../gramsrec"                       /* records */

#define BUFSIZE     128
#define MAXWDS      2048              /* max number of words to be indexed */
#define DEF_LEN     5                               /* default word length */
#define MAX_LEN     20                                  /* max word length */
#define NUM_ROUNDS  10                        /* number of words per round */

struct  recstr {
	char    r_lname[9];                           /* holder's log name */
	char    r_name[17];                               /* holder's name */
	int     r_points;                           /* highest point score */
	long    r_date;                                 /* date record set */
} rec;

char    *dictionary = DICTFIL;        /* the computer's default vocabulary */
char    lname[9];                               /* current user's log name */
char    name[16];                            /* current user's chosen name */
int     trace   = 0;                                         /* trace flag */
int     w_len   = DEF_LEN;                          /* minimum word length */
int     hpnts   = 0;                                      /* human's score */
FILE    *dfp    = NULL;                                 /* dictionary file */

long    index[MAXWDS];      /* indices to dictionary words of length w_len */

char    *getwd(), *scramble(), *uinsert();

main(argc, argv)
char    *argv[];
{
	char buf[BUFSIZE];
	char *origwd, *cp;
	int i, nwds, points, rounds, recset;
	long begin, end;

	while (--argc > 0) {
	    if (arg(argv[argc]) == -1) {
		printf("Usage: %s [dDICTFILE] [lLENGTH] \n", argv[0]);
		printf("defaults: d%s l%d\n", DICTFIL, DEF_LEN);
		exit(2);
	    }
	}
	time(&begin);
	srand((int) (begin & 077777));
	if (fork() == 0) {
	    sleep(3);
	    printf("\nWelcome to GRAMS; do you want instructions? ");
	    fgets(buf, sizeof buf, stdin);
	    if (*buf == 'y')
		instruct(INFOFIL);
	    if (getrec(w_len, &rec) != -1)
		printf("The record for %d letters is %d, set by %s, (%s), %s",
		 w_len, rec.r_points, rec.r_name, rec.r_lname,
		 ctime(&rec.r_date));
	    exit(0);
	}
	nwds = cmpindex(w_len);
	if (nwds < NUM_ROUNDS) {
	    if (nwds == 0)
		printf("I don't know any words");
	    else
		printf("I only know %d word%s", nwds, splur(nwds));
	    printf(" with %d letters!\n", w_len);
	    exit(3);
	}
	wait0();
	copy(logname(), lname);
	printf("What's your name? ");
	fgets(buf, sizeof buf, stdin);
	buf[strlen(buf) - 1] = buf[15] = '\0';
	copy(buf, name);
	recset = 0;
again:
	points = 0;
	for (rounds = 0; rounds < NUM_ROUNDS; rounds++) {
	    i = (rand() >> 2) % nwds;
	    origwd = getwd(i);
	    cp = scramble(origwd);
	    printf("Ready? ");
	    fgets(buf, sizeof buf, stdin);
	    time(&begin);
	    for (;;) {
		while (empty(stdin->_file) == 0)
		    fgets(buf, sizeof buf, stdin);
		printf("%s  :  ", cp);
		if (fgets(buf, sizeof buf, stdin) == NULL)
		    exit(0);
		if (*buf == '\n') {
		    printf("The word I had in mind was \"%s\".\n", origwd);
		    break;
		}
		buf[strlen(buf) - 1] = '\0';
		if (strcmp(buf, origwd) == 0) {
		    time(&end);
		    i = end - begin - w_len / 2;
		    if (i < 1)
			i = 1;
		    i = (16 * w_len) / (end - begin);
		    points += i;
		    if (i > 16)
			printf("Very good!  ");
		    else
			printf("Correct; ");
		    printf("You now have %d points.\n", points);
		    break;
		}
		printf("No, that's not it.  Try again with ");
	    }
	}
	printf("You got %d points in %d rounds ... ", points, rounds);
	if (points >= 15 * rounds)
	    printf("an Excellent score!\n");
	else if (points >= 10 * rounds)
	    printf("a good score!\n");
	else if (points > 7 * rounds)
	    printf("not too impressive\n");
	else
	    printf("a lousy score\n");
	if (getrec(w_len, &rec) == -1) {
	    printf("and you've set a new record!\n");
	    copy(lname, rec.r_lname);
	    copy(name, rec.r_name);
	    rec.r_points = points;
	    time(&rec.r_date);
	    putrec(w_len, &rec);
	    recset++;
	} else if (points > rec.r_points) {
	    if (strcmp(rec.r_lname, lname) != 0
	     || strcmp(rec.r_name, name))
		printf("and you've broken %s's record of %d!\n",
		 rec.r_name, rec.r_points);
	    else
		printf("and you've broken your old record of %d!\n",
		 rec.r_points);
	    copy(lname, rec.r_lname);
	    copy(name, rec.r_name);
	    rec.r_points = points;
	    time(&rec.r_date);
	    putrec(w_len, &rec);
	    recset++;
	}
	printf("Again? ");
	fgets(buf, sizeof buf, stdin);
	if (*buf == 'y')
	    goto again;
	if (recset)
	    arg("r");
	exit(0);
}

cmpindex()
{
	register int i;
	char buf[BUFSIZE];
	long addr;
	extern long ftell();

	if (dfp == NULL && (dfp = fopen(dictionary, "r")) == NULL) {
	    fprintf(stderr, "Unable to open word memory  ");
	    perror(dictionary);
	    exit(3);
	}
	fseek(dfp, 0l, 0);
	i = 0;
	while (fgets(buf, sizeof buf, dfp) != NULL && i < MAXWDS)
	    if (strlen(buf) == w_len + 1)
		index[i++] = ftell(dfp) - w_len - 1;
	return(i);
}

char    *
getwd(n)
{
	static char buf[BUFSIZE];

	fseek(dfp, index[n], 0);
	fgets(buf, sizeof buf, dfp);
	buf[strlen(buf) - 1] = '\0';
	return(buf);
}

char    *
scramble(string)
char    *string;
{
	register int i, j, len;
	static char buf[BUFSIZE];

	for (len = 0; buf[len] = string[len]; len++);
	while (--len > 0) {
	    i = (rand() >> 2) % len;
	    j = buf[len];
	    buf[len] = buf[i];
	    buf[i] = j;
	}
	return(buf);
}

arg(string)     /* set global parameters from argument */
char    *string;
{
	register int i;
	char buf[32], *cp;

	switch (*string) {
	case 'd':                                       /* dictionary file */
	    dictionary = &string[1];
	    if (dfp != NULL) {
		fclose(dfp);
		dfp = NULL;
	    }
	    break;
	case 'l':                                   /* minimum word length */
	    w_len = atoi(&string[1]);
	    if (w_len < 2 || w_len > MAX_LEN) {
		printf("Length must be between 2 and %d\n", MAX_LEN);
		exit(2);
	    }
	    break;
	case 'r':                                         /* print records */
	    printf("Length Points  ________________Player_  Date\n");
	    for (i = 2; i <= MAX_LEN; i++) {
		if (getrec(i, &rec) == -1)
		    continue;
		printf("%4d  %5d", i, rec.r_points);
		cp = copy(rec.r_name, buf);
		cp = copy(" (", cp);
		cp = copy(rec.r_lname, cp);
		cp = copy(")", cp);
		printf(" %27.27s %s", buf, ctime(&rec.r_date));
	    }
	    exit(0);
	case 'T':                                                 /* trace */
	    trace = atoi(&string[1]);
	    break;

	default:
	    printf("?%s? unrecognized.\n", string);
	    return(-1);
	}
	return(0);
}

char    *
uinsert(string)
char    *string;
{
	register char *cp, *bp;
	static char buf[BUFSIZE];

	for (cp = string, bp = buf ; *bp++ = *cp; )
	    if (*cp++ == 'q')
		*bp++ = 'u';
	return(buf);
}

getrec(len, rp)
struct  recstr  *rp;
{
	register int retval;
	FILE *rfp;

	if ((rfp = fopen(RECFIL, "r")) == NULL) {
	    fprintf(stderr, "Unable to open record file for read  ");
	    perror(RECFIL);
	    return(-1);
	}
	fseek(rfp, (long) (len * sizeof *rp), 0);
	retval = 0;
	if (fread(rp, sizeof *rp, 1, rfp) != 1 || rp->r_lname[0] == '\0')
	    retval = -1;
	fclose(rfp);
	return(retval);
}

putrec(len, rp)
struct  recstr  *rp;
{
	FILE *rfp;

	if ((rfp = fopen(RECFIL, "a")) == NULL) {
	    fprintf(stderr, "Unable to open record file for append  ");
	    perror(RECFIL);
	    return;
	}
	fseek(rfp, (long) (len * sizeof *rp), 0);
	fwrite(rp, sizeof *rp, 1, rfp);
	fclose(rfp);
}
