#include    <stdio.h>
#include    <cursor.h>
/*
**      BOG -- Word game
** Compile: cc -O -q bog.c -lS -lP -o bog
*/

char    *whatsccs "@(#)bog.c	1.3  last mod 1/2/80 -- (c) psl 1979";

#define DEF_SECS    180
#define DEF_SIZE    4

#define SCRN_HEIGHT 24                        /* number of lines on screen */
#define LIST_HEIGHT (SCRN_HEIGHT - 3)   /* number of lines to use in lists */

#define ALARM_SIG   14
#define DICTFIL "/sys/games/.../bogwds"              /* default dictionary */
#define MONFIL  "/sys/games/.../bog.mon"              /* new words learned */
#define CTMPFIL "cbog.tmp"              /* file used to store comp's words */
#define HTMPFIL "hbog.tmp"             /* file used to store human's words */
#define INFOFIL "/sys/games/.../boginfo"                   /* instructions */

#define B_SIZE      8                                /* maximum board size */
#define R_CUBES     (16)                     /* number of nonvirtual cubes */
#define N_CUBES     (B_SIZE * B_SIZE)           /* number of virtual cubes */

#define WAIT    0                     /* used by accwds(), `non-busy wait' */
#define NOWAIT  1                         /* used by accwds(), `busy wait' */

char    *cubes[R_CUBES] {
	"moqabj", "sehinp", "ntdkuo", "lukegy", "enadzv", "edamcp",
	"iboxfr", "aioatc", "efihey", "sdwneo", "hosarm", "viteng",
	"baliyt", "arelsc", "ptlseu", "wilurg",
};

char    *dictionary = DICTFIL;      /* the computer's default vocabulary */
int     trace   = 0;                /* trace flag */
int     pflag   = 1;                /* play flag */
int     tflag   = DEF_SECS;         /* default secs / board */
int     b_size  = DEF_SIZE;         /* default board size */
int     n_cubes = 16;               /* default number of letter cubes */
int     w_len   = DEF_SIZE;         /* default minimum word length */
int     timeout = 0;                /* flag to indic time has run out */
int     nbr[N_CUBES][9];            /* indices of neighboring cells */
int     cpnts   = 0;                /* computer's score */
int     hpnts   = 0;                /* human's score */
int     wy;                         /* line for human input */
int     inpeof;                     /* human input ended */
FILE    *ctfp;                      /* temp file for computer words */
FILE    *htfp;                      /* temp file for human words */
FILE    *dfp;                       /* dictionary file */
FILE    *mfp;                       /* monitor, (new words), file */

char    *udelete();

main(argc, argv)
char    *argv[];
{
	char junk[64], up[N_CUBES + 1];
	int i;
	long now;
	extern int beep();

	while (--argc > 0) {
	    if (arg(argv[argc]) == -1) {
		printf("Usage: %s [tSECS][sSIZE] [lLENGTH] [dDICTFILE]\n",
		 argv[0]);
		printf("defaults: t%d s%d l%d d%s\n",
		 DEF_SECS, DEF_SIZE, DEF_SIZE, DICTFIL);
		exit(2);
	    }
	}
	printf("\nWelcome to BOG; do you want instructions? ");
	fgets(junk, 64, stdin);
	if (*junk == 'y')
	    instruct(INFOFIL);
	time(&now);
	srand((int) now);
	if ((mfp = fopen(MONFIL, "a")) != NULL) {
	    fprintf(mfp, "uid:%d %s", getuid(), ctime(&now));
	    fclose(mfp);
	}
	signal(ALARM_SIG, &beep);
	printf("%s", cursor(CLEAR, 0));
	for (;;) {
	    timeout = 0;
	    if (dfp == NULL) {
		if ((dfp = fopen(dictionary, "r")) == NULL) {
		    fprintf(stderr, "I can't open my word memory! ");
		    perror(DICTFIL);
		    exit(3);
		}
	    }
	    n_cubes = b_size * b_size;
	    setup();
	    rattle(up);         /* shake cubes & display */
	    if (pflag)
		alarm(tflag);
	    timeout = 0;
	    if (pflag) {
		if ((htfp = fopen(HTMPFIL, "w")) == NULL) {
		    fprintf(stderr, "Can't open %s.\n", CTMPFIL);
		    exit(3);
		}
                while (empty(stdin->_file) == 0)
                    fgets(junk, 32, stdin);
		printf("%sEnter words:\n> ", cursor(1, 0));
		wy = 1;
	    }
	    inpeof = 0;
	    findwds(up);
	    if (pflag) {
		while (timeout == 0 && accwds(WAIT) != -1);
		alarm(0);
		awclose();
		dumpwds(up);
	    }
	    do {
		printf("%s", cursor(50, LIST_HEIGHT + 1));
		i = read(0, junk, 32);
		if (i > 1) {
		    arg(junk);
		    i = -1;
		}
	    } while (i < 0);
	    if (i == 0)
		exit(0);
	    printf("%s", cursor(CLEAR, 0));
	}
}

beep()
{
	signal(ALARM_SIG, &beep);
	printf("%s", cursor(60, 0));
	printf("Time is up!\n");
	timeout = 1;
}

arg(string)     /* set global parameters from argument */
char    *string;
{
	register int i;

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
	    break;
	case 's':                                            /* board size */
	    i = atoi(&string[1]);
	    if (i < 2 || i > B_SIZE) {
		printf("Size must be in the range of 2 to %d\n",
		 B_SIZE);
		return(-1);
	    }
	    b_size = i;
	    w_len = b_size;
	    break;
	case 'T':                                                 /* trace */
	    trace = atoi(&string[1]);
	    pflag = 0;
	    break;
	case 't':                                          /* time / round */
	    i = atoi(&string[1]);
	    if (i < 0 || i > 600) {
		printf("Time must be between 0, (untimed), and 600 secs.\n");
		return(-1);
	    }
	    tflag = i;
	    break;
	default:
	    printf("?%s? unrecognized.\n", string);
	    return(-1);
	}
	return(0);
}

rattle(up)
char    *up;
{
	register int i, j, k;
	char q[N_CUBES];

	for (i = n_cubes; --i >= 0; q[i] = i);
	printf("%s", cursor(40, 2 + 2 * b_size));
	up[n_cubes] = '\0';
	for (i = n_cubes; i > 0; ) {
	    j = (rand() >> 2) % i;
	    k = q[j];
	    q[j] = q[--i];
	    q[i] = k;
	    j = (rand() >> 2) % 6;
	    up[i] = cubes[k % R_CUBES][j];
	    printf("%c", up[i] & ~040);
	    printf("%c   ", up[i] == 'q'? 'u' : ' ');
	    if (i % b_size == 0)
		printf("%s", cursor(40, 2 + 2 * (i / b_size)));
	}
}

findwds(up)
char    *up;
{
	register int i, j;
	char  wbuf[64];
	char ulf[26], wlf[26];
	char upf[26][26];
	char wpf[26][26];
	int next;

	if (pflag && (ctfp = fopen(CTMPFIL, "w")) == NULL) {
	    printf("Can't open %s.  Switching to non-play mode...\n",
	     CTMPFIL);
	    pflag = 0;
	}
	lfreq(up, ulf);
	dpfreq(up, upf);        /* calc pair freqs including diagonals */
	fseek(dfp, 0l, 0);
	while (fgets(wbuf, 64, dfp) != NULL && timeout == 0) {
            strcpy(wbuf, udelete(wbuf));
	    if (pflag)
		accwds(NOWAIT);
	    if (ulf[*wbuf - 'a'] < 1)
		continue;
	    i = strlen(wbuf);
	    if (i <= w_len || i > n_cubes)
		continue;
	    lfreq(wbuf, wlf);
	    for (i = 26; --i >= 0; )
		if (wlf[i] > ulf[i])
		    goto noway;
	    pfreq(wbuf, wpf);
	    for (i = 26; --i >= 0; )
		for (j = 26; --j >= 0; )
		    if (wpf[i][j] > upf[i][j])
			goto noway;
	    if (exists(wbuf, up)) {
		if (pflag)
		    fputs(wbuf, ctfp);
		else
		    printf("%s%s       \r",
		     cursor(1, next++ % LIST_HEIGHT), uinsert(wbuf));
	    }
noway:      ;
	}
	if (pflag) {
	    fputs("~~~~~\n", ctfp);
	    fclose(ctfp);
	}
}

accwds(mode)
{
	char buf[64], *cp;

	if (inpeof)
	    return(-1);
	if (mode == NOWAIT && empty(stdin->_file))
		return(0);
	if (fgets(buf, 64, stdin) == NULL || buf[0] == '\n') {
	    printf("%s  ~~~~~", cursor(0, wy % LIST_HEIGHT));
	    printf("%s", cursor(20, LIST_HEIGHT + 1));
	    inpeof++;
	    return(-1);
	}
	fputs(udelete(buf), htfp);
	printf("%s  ", cursor(0, wy++ % LIST_HEIGHT));
	printf("%s        \r> ", cursor(2, wy % LIST_HEIGHT));
	return(0);
}

awclose()
{
	char buf[64];

	fputs("~~~~~\n", htfp);
	fclose(htfp);
	sprintf(buf, "sort %s | uniq | into %s", HTMPFIL, HTMPFIL);
	system(buf);
}

dumpwds(up)
char    *up;
{
	register int i, y, wmax;
	char cbuf[64], hbuf[64];
	int x, cpt, hpt;

	printf("%s The words I found were:", cursor(1, 0));
	ctfp = fopen(CTMPFIL, "r");
	x = wmax = 0;
	y = 1;
	while (fgets(cbuf, 64, ctfp) != NULL) {
	    i = strlen(cbuf);
	    if (i > wmax)
		wmax = i;
	    printf("%s%s",
	     cursor(x + 12, y), uinsert(cbuf));
	    if (y++ > LIST_HEIGHT) {
		x += wmax;
		wmax = 0;
		y = 1;
	    }
	}
	htfp = fopen(HTMPFIL, "r");
	mfp = fopen(MONFIL, "a");
	fseek(ctfp, 0l, 0);
	cpt = hpt = 0;
        while (empty(stdin->_file) == 0)
            fgets(hbuf, 64, stdin);
	printf("%sHit return for results ... ", cursor(32, LIST_HEIGHT + 1));
	fgets(hbuf, 64, stdin);
	printf("%s  C   H  WORD\n", cursor(CLEAR, 0));
	fgets(cbuf, 64, ctfp);
	fgets(hbuf, 64, htfp);
	for (;;) {
	    i = strcmp(cbuf, hbuf);
	    if (i < 0) {
		i = strlen(cbuf) - w_len;
		cpt += i;
		printf("%3d      %s", i, uinsert(cbuf));
		fgets(cbuf, 64, ctfp);
	    } else if (i > 0) {
		i = strlen(hbuf) - w_len;
		if (i <= 0)
		    printf("      0  (too short) %s", uinsert(hbuf));
		else if (!exists(hbuf, up))
		    printf("      0  (illegal) %s", uinsert(hbuf));
		else {
		    hpt += i;
		    printf("    %3d  %s", i, uinsert(hbuf));
		    if (mfp != NULL)
			fputs(hbuf, mfp);
		}
		fgets(hbuf, 64, htfp);
	    } else {
		if (cbuf[0] == '~')
		    break;
		fgets(cbuf, 64, ctfp);
		fgets(hbuf, 64, htfp);
	    }
	}
	fclose(ctfp);
	unlink(CTMPFIL);
	fclose(htfp);
	unlink(HTMPFIL);
	if (mfp != NULL)
	    fclose(mfp);
	printf("--- ---\n%3d%4d  This round\n", cpt, hpt);
	cpnts += cpt;
	hpnts += hpt;
	printf("--- ---\n%3d%4d  Game total\n", cpnts, hpnts);
}

lfreq(string, lf)           /* calc letter frequency */
char    *string, lf[26];
{
	register char *cp;

	for (cp = &lf[26]; --cp >= lf; *cp = 0);
	for (cp = string; *cp >= 'a' && *cp <= 'z'; cp++)
	    lf[*cp - 'a']++;
}

pfreq(string, pf)           /* calc pair frequencies */
char    *string, pf[26][26];
{
	register char *cp;
	register int i, j;


	for (i = 26; --i >= 0; )
	    for(j = 26; --j >= 0; )
		pf[i][j] = 0;
	for (cp = string; *cp > ' ' && (j = *cp++ - 'a') < 0 && j > 25; );
	while ((i = *cp++ - 'a') >= 0 && i <= 25) {
	    pf[i][j]++;
	    j = i;
	}
}

dpfreq(string, pf)          /* calc pair freqs with diagonals, etc */
char    *string, pf[26][26];
{
	register int n, i, j;
	char s[N_CUBES];

	for (i = n_cubes; --i >= 0; )
	    s[i] = string[i] - 'a';
	for (i = 26; --i >= 0; )
	    for (j = 26; --j >= 0; )
		pf[i][j] = 0;
	for (i = n_cubes; --i >= 0; )
	    for (n = 0; (j = nbr[i][n]) != -1; n++)
		pf[s[i]][s[j]]++;
}

exists(string, up)
char    *string, *up;
{
	register int i;

	for (i = n_cubes; --i >= 0; )
	    if (up[i] == *string && track(&string[1], up, i))
		return(1);
	return(0);
}

track(string, bd, i)
char    *string, *bd;
{
	register int n, j;
	char q[N_CUBES], *sp;

	if (*string <= ' ')
	    return(1);
	for (j = n_cubes; --j >= 0; )
	    q[j] = bd[j];
	q[i] = 0;
	for (n = 0; (j = nbr[i][n]) != -1; n++)
	    if (q[j] == *string && track(&string[1], q, j))
		return(1);
	return(0);
}

setup()                                         /* do misc one-time set up */
{
	register int i, j;

	for (i = n_cubes; --i >= 0; ) {
	    j = 0;
	    if (i >= b_size)
		nbr[i][j++] = i - b_size;
	    if (i >= b_size && (i + 1) % b_size != 0)
		nbr[i][j++] = i - b_size + 1;
	    if ((i + 1) % b_size != 0)
		nbr[i][j++] = i + 1;
	    if (i < n_cubes - b_size && (i + 1) % b_size != 0)
		nbr[i][j++] = i + b_size + 1;
	    if (i < n_cubes - b_size)
		nbr[i][j++] = i + b_size;
	    if (i < n_cubes - b_size && i % b_size != 0)
		nbr[i][j++] = i + b_size - 1;
	    if (i % b_size != 0)
		nbr[i][j++] = i - 1;
	    if (i >= b_size && i % b_size != 0)
		nbr[i][j++] = i - b_size - 1;
	    nbr[i][j] = -1;
	}
}

equal(a, b)
char    *a, *b;
{
	register char *ap, *bp;

	ap = a;
	bp = b;
	while (*ap++ == *bp++)
	    if (*ap < ' ' && *bp < ' ')
		return(1);
	return(0);
}

char    *
uinsert(string)
char    *string;
{
	register char *cp, *bp;
	static char buf[64];

	for (cp = string, bp = buf ; *bp++ = *cp; )
	    if (*cp++ == 'q')
		*bp++ = 'u';
	return (buf);
}

char    *
udelete(string)
char    *string;
{
	register char *cp, *bp;
	static char buf[64];

	for (cp = string, bp = buf ; *bp++ = *cp++; )
	    if (*cp == 'u' && cp[-1] == 'q')
		cp++;
	return (buf);
}

