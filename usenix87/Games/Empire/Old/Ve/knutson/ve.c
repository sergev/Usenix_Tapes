/* 
 *      ve - visual empire (with due respect to Peter Langston).
 *
 *      Written by Matthew Diaz and Michael Baldwin
 *      Modified by Bill Jones, Jim Knutson, Ken Montgomery and Dan Reynolds
 *
 *      Usage: ve [-a|c] census commodity map ship spy radar coastwatch....
 *
 *      Compile using: cc -o ve ve.c -O -lcurses -ltermcap
 */

#include <ctype.h>
#include <stdio.h>
#include <curses.h>
#include <signal.h>

/* Things you may want to change */
#define VIPATH      "/usr/ucb/vi"
#define EXPATH      "/usr/ucb/ex"
#define SHPATH      "/bin/sh"
#define TEMPFILE    "/tmp/vetemp"

#define MAPSIZE     128                 /* Empire world size */
#define RADARSIZE   40                  /* Radar scan size */
#define MAXSHIPS    1024                /* Maximum number of ships */
#define VALUESIZE   30                  /* Number of values in sector */
#define DELSIZE     12                  /* Number of deliveries per sector */
#define CNTSIZE     12                  /* Number of contracts per sector */
#define NODISK      -1                  /* No disk address flag */
#define NOSHIPS     -1                  /* No ships present */
#define UNKNOWN     -1                  /* Unknown ship index flag */
#define MCOLS       79                  /* Width of the map window */
#define MLINES      17                  /* Height of the map window */
#define NOX         0                   /* Expansion flags for getline */
#define EX          1
#define NOSU        0                   /* Survey or no survey for mapdr */
#define SURV        1
#define ESC         '\033'

#define ODD(x)      ((x)&01)
#define EVEN(x)     (!((x)&01))
#define MAX(A,B)    ((A) > (B) ? (A) : (B))
#define MIN(A,B)    ((A) < (B) ? (A) : (B))
#define VALID(x,y)  (EVEN(x)&&EVEN(y)||ODD(x)&&ODD(y))
#define CTRL(c)     (('c')-0100)

/* Offsets into sector arrays for various items */
#define CIV     0
#define MIL     1
#define FOOD    2
#define SH      3
#define GUN     4
#define PL      5
#define IRON    6
#define DUST    7
#define BAR     8
#define OIL     9
#define LCM     10
#define HCM     11
#define EFF     12
#define MOB     13
#define MIN     14
#define GMIN    15
#define FERT    16
#define PET     17
#define DES     18
#define COU     19
#define CONT    20
#define DEL     21
#define CHKPT   22

char    usage[] = "Usage: ve [-c|a out] files...";

/*
 * If you want accounting, change the filename below (make sure
 * it is 622 at least).  Everytime someone plays that version
 * a line will be entered showing login name, elapsed time and
 * date.
 */
#ifdef ACCTNG
char    acct[] = "/u2/empire/veacct";
#endif

/*
 * Temporary file related variables
 */
char tfile[30];                     /* temporary file name */
int temp = -1;                      /* temporary file descriptor */

/*
 * Map sector data structures
 */
struct sector {
        char        surv;           /* Survey value */
        char        mark;           /* Mark character */
        char        own;            /* Do you own or know that sector? */
        char        des;            /* Sector designation */
        int         shp;            /* Index of current ship at x,y */
        long int    diskaddr;       /* Disk address containing values */
} map[MAPSIZE][MAPSIZE];

struct shipentry {
        char        des;            /* Ship designation (type) */
        int         number;         /* Ship number */
        int         x,y;            /* Map coordinates of ship */
        char        fleet;          /* Fleet designation */
        long int    diskaddr;       /* Disk address containing values */
} ships[MAXSHIPS];

struct  value {
        int         val[VALUESIZE]; /* Values as described by items */
        char        del[DELSIZE];   /* Delivery routes */
        char        cnt[CNTSIZE];   /* Contracts */
} values;
                        
struct item {
        char        *nm;            /* Item name - prefix */
        int         len;            /* Length of prefix (for strncmp) */
} items[] = {
        "civ",  3,
        "mil",  3,
        "foo",  3,
        "sh",   2,
        "gun",  3,
        "pl",   2,
        "iro",  3,
        "dus",  3,
        "bar",  3,
        "oil",  3,
        "lcm",  3,
        "hcm",  3,
        "eff",  3,
        "mob",  3,
        "min",  3,
        "gmi",  3,
        "fer",  3,
        "pet",  3,
        "des",  3,
        "cou",  3,
        "con",  3,
        "del",  3,
        "",     0
};


/* 
 * Vector of ship names
 */
char *shipnames[] = {
    "yacht",                  
    "tender",               
    "minesweep",            
    "destroyer",            
    "submarine",            
    "cargo ship",           
    "battleship",           
    "oil derrick",          
    "patrol boat",          
    "fishing boat",         
    "heavy cruiser",        
    "aircraft carr",     
    "aircraft carrier",     
    0                       
}; /* shipnames */


/*
 * ve input file function processors 
 */
int     census();                   /* census file processor */
int     readmap();                  /* map file processor */
int     commodities();              /* commodities file processor */
int     spy();                      /* spy report processor */
int     radarscan();                /* naval radar scan processor */
int     ship();                     /* ship report processor */
int	coastwatch();		    /* coastwatch report processor */

struct funsw {                      /* list of prefixes and functions */
        char    *type;
        int     (*func)();
} fsw[] = {
        "cen",  census,
        "map",  readmap,
        "com",  commodities,
        "spy",  spy,
        "rad",  radarscan,
        "shi",  ship,
	"coa",  coastwatch,
        0,      0,
};

char    macros[127][MCOLS];     /* Macro definitions */
char    peekc;                  /* Lets you poke a character */
int     startx = -50000;        /* Leftmost x-coordinate of display window */
int     starty = -50000;        /* Uppermost y-coordinate of display window */
int     minx;                   /* Min and max x (used for map reading) */
int     maxx;
int     miny;
int     curmark = '>';          /* Current marking character */
int     range = 10;             /* Range for survey */
int     curx, cury;             /* Current x and y coordinates in map array */
int     shipcount = 0;          /* Total number of ships in ships array */
int     shipmode = FALSE;       /* True if in ship display mode */
int     surmap = 0;             /* Nonzero if survey map should be displayed */
int     noise = TRUE;           /* True if production messages to be printed */ 

FILE    *inpf;                  /* Input file pointer */
char    iname[BUFSIZ];          /* Input file name */
FILE    *outf;                  /* Output file pointer */
char    oname[BUFSIZ];          /* Output file name */

char    buf[BUFSIZ];            /* Line buffer */

main(argc, argv)
int argc;
char *argv[];
{
        struct funsw *fp;       /* Pointer to input file function processor */
#ifdef ACCTNG
        FILE *actf;             /* Accounting file */
        long sclock;
        long eclock;
#endif

        if (argc == 1) puts(usage), exit(1);

#ifdef ACCTNG
        time(&sclock);
#endif
        opentf();
        presetmap();
        argv++;
        while (*argv)  {
            /* Open/create command file */
            if (**argv == '-' && (argv[0][1] == 'c'||argv[0][1] == 'a')) {
                if ((outf = fopen(*++argv,
                    (argv[0][1] == 'c') ? "w" : "a")) == NULL)
                         perror(*argv), exit(1);
                    strcpy(oname, *argv++);
                    continue;
            } /* if */

            /*
             * Process empire report files.  This is done by looking at
             * the second line of the file which contains a string of
             * the form:
             *
             *            cen ... (e.g. cen # >cen.out)
             *
             * The first word indicates the type of file it is (census,
             * map, etc).
             */
            if ((inpf = fopen(*argv, "r")) == NULL) perror(*argv), exit(1);
            if (fgets(buf, sizeof buf, inpf) == NULL ||
                fgets(buf, sizeof buf, inpf) == NULL) {
                    printf("%s: wrong format\n", *argv);
                    exit(1);
            }

            for (fp = fsw; *fp->type; fp++)
                if (!strncmp(fp->type, buf, strlen(fp->type)))
                    (fp->func)(inpf);
            fclose(inpf);
            argv++;
        } /* while */
        
        if (map[offset(0)][offset(0)].own)
            center (0,0);
        else
            center (minx + MCOLS/2, miny + MLINES/2);
        startx = 0 - MCOLS/2  ; /* Set at Capitol */
        if (ODD(startx)) startx--;
        starty = 0 - MLINES/2 ;
        if (ODD(starty)) starty--;

        signal(SIGINT, SIG_IGN);        /* Ignore common signals */
        signal(SIGQUIT, SIG_IGN);

        initscr();                      /* Start up curses etc. */
        presetty();
        commands();                     /* Process all commands */
        move(LINES-1, 0);               /* Done - move to bottom of screen */
        clrtoeol();
        refresh();

#ifdef ACCTNG
        /* Save accounting information */
        if ((actf = fopen(acct, "a")) != NULL) {
            time(&eclock);
            if (!getpw(getuid(), buf) && (bp = index(buf, ':'))) *bp=0;
                else sprintf(buf, "%d", getuid());
            eclock -= sclock;
            fprintf(actf, "%s\t%2d:%02d\t%s", buf,
                    eclock/60, eclock % 60, ctime(&sclock));
            fclose(actf);
        }
#endif
        endtty();                       /* Reset terminal characteristics */
} /* main */

/*
 * Input file processing functions
 *
 * These routines parse the information from the input files,
 * many times in a very ugly way.
 *
 * Each routine checks to see if there is a comma in the 4th position.
 * This is to make certain that the line is of the form x,y and not
 * another message (e.g. Bad weather).
 */

/*
 * census - Read and process census file.
 */
census(inpf)
FILE *inpf;
{
        register int x,y;
        register char *p;
        struct sector *mp;

        while (fgets(buf, sizeof buf, inpf) != NULL) {
            if (buf[3] != ',') {
            /*
             * Look for census lines of the form:
             * 'xxxxx yyy zzz in x,y' (reports of products produced)
             */
                if (p = (char *)rindex(buf, ',')) {
                    if (*++p == '-') p++;
                    if (noise && *p >= '0' && *p <= '9') {
                        fputs(buf, stderr);
                        fflush(stderr);
                    } /* if */
                } /* if */
                continue;
            } /* if */
            x = atoi(buf);
            y = atoi(&buf[4]);
            mp = &map[offset(x)][offset(y)];
            readvalues (offset(x),offset(y));
            mp->own = 1;
            values.del[CIV] = buf[19];
            values.del[MIL] = buf[20];
            values.del[FOOD] = buf[21];
            values.cnt[CIV] = buf[23];
            values.cnt[MIL] = buf[24];
            values.cnt[FOOD] = buf[25];
            values.val[CHKPT] = buf[27];
            values.val[COU] = -1;
            mp->des = (buf[8]=='-') ? '~' : ((buf[8]=='^') ? '&' : buf[8]);
            values.val[EFF] = atoi(&buf[10]);
            values.val[MOB] = atoi(&buf[15]);
            sscanf(&buf[29], "%d%d%d%d%d%d%d",
                   &values.val[CIV], &values.val[MIL], &values.val[FOOD],
                   &values.val[MIN], &values.val[GMIN], &values.val[FERT], 
                   &values.val[PET]);
            writevalues(offset(x),offset(y));
        } /* while */
} /* census */


/*
 * commodities - Read and process commodities file.
 */
commodities(inpf)
FILE *inpf;
{
        register int x,y;
        register char *p;
        struct sector *mp;

        while (fgets(buf, sizeof buf, inpf) != NULL) {
            if (buf[3] != ',') {
            /*
             * Look for census lines of the form:
             * 'xxxxx yyy zzz in x,y' (reports of products produced)
             */
                if (p = (char *)rindex(buf, ',')) {
                    if (*++p == '-') p++;
                    if (noise && *p >= '0' && *p <= '9') {
                        fputs(buf, stderr);
                        fflush(stderr);
                    } /* if */
                } /* if */
                continue;
            } /* if */
            x = atoi(buf);
            y = atoi(&buf[4]);
            readvalues (x,y);
            mp = &map[offset(x)][offset(y)];
            mp->own = 1;
            sscanf(&buf[14], "%c%c%c%c%c%c%c%c%c",
                   &values.del[SH], &values.del[GUN], &values.del[PL],
                   &values.del[IRON], &values.del[DUST], &values.del[BAR],
                   &values.del[OIL], &values.del[LCM], &values.del[HCM]);
            sscanf(&buf[24], "%c%c%c%c%c%c%c%c%c",
                   &values.cnt[SH], &values.cnt[GUN], &values.cnt[PL],
                   &values.cnt[IRON], &values.cnt[DUST], &values.cnt[BAR],
                   &values.cnt[OIL], &values.cnt[LCM], &values.cnt[HCM]);
            sscanf(&buf[34], "%d%d%d%d%d%d%d%d%d",
                   &values.val[SH], &values.val[GUN], &values.val[PL],
                   &values.val[IRON], &values.val[DUST], &values.val[BAR],
                   &values.val[OIL], &values.val[LCM], &values.val[HCM]);
            writevalues(x,y);   
        } /* while */
} /* commodities */


editradarscan(rm, xr, yr, xp, yp, rp)
char rm[RADARSIZE][RADARSIZE];
int *yr, *xr;
int *xp, *yp;
float *rp;
{
        register int x,y;
        register int xs,ys;
        register int mx,my;

        y = 10.0 * (*rp);
        if (((y%10) == 5) && *yp < 0) ys = (*yr)/2;
        else                          ys = (*yr+1)/2;
        xs = *xr/2;
        if (rm[xs][ys] == ' ') xs++;       
        for (x = 0; x <= *xr; x++)
            for (y = 0; y <= *yr; y++) {
                switch(rm[x][y]) {

                    case 'A':
                    case 'B':
                    case 'C':
                    case 'D':
                    case 'F':
                    case 'H':
                    case 'M':
                    case 'O':
                    case 'P':
                    case 'S':
                    case 'T':
                    case 'Y': rm[x][y] = '$';
                              break;
      
                    case '/': rm[x][y] = ' ';
                } /* switch */
                merge(rm[x][y], (*xp-xs+x), (*yp-ys+y));
            } /* for */
} /* editradarscan */


findshipname(inpf)
FILE *inpf;
{
        register char c, *sp;
        register int i;

nomatch:    c = getc(inpf);
            if (c == EOF) return(0);
            for (i = 0; sp = shipnames[i]; i++) {
                while (*sp) {
                    if (c != *sp) {
                        if (sp == shipnames[i]) goto next;
                        if (skipword(inpf)) goto nomatch;
                        return(0);
                    } /* if */
                    c = getc(inpf);
                    sp++;
                } /* while */
                fscanf(inpf," %*d at ");
                return(1);
next:           continue;
            } /* for */
            if (!(c == '-' || (c >= '0' && c <= '9'))) goto nomatch;
            ungetc (c, inpf);
            return(2);
} /* findshipname */

 
/*
 * getshiptype - Get ship type.
 */
getshiptype(bp)
register char *bp;
{
        register int i;
        register char *sp;

        for (i = 0; sp = shipnames[i]; i++) {
            if (!strncmp (sp, bp, strlen(sp)))
                return (shipnames[i][0] - 'a' + 'A');
        } /* for */
        return (FALSE);

} /* getshiptype */


/*
 * merge - Update sector designation.
 */
merge(uc, x, y)
register char uc;
register int x,y;
{
        struct sector *mp;

        mp = &map[offset(x)][offset(y)];
        switch(mp->des) {

            case '^':   if (uc == '&') mp->des = uc;
                        /* fall through */

            case '\\':  break;

            case '~':   if (uc == '-') break;
                        /* fall through */

            case '&':   if (uc == '^') break;
                        /* fall through */

            default:
            case '-':
            case '?':   if (uc == ' ' || uc == '$' || uc == '?') break;
                        /* fall through */

            case 0:
            case ' ':   if (uc == '$') mp->des = '.';
                        else mp->des = uc;
                        break;
        } /* switch */
} /* merge */


presetradarmap(rm)
char rm[RADARSIZE][RADARSIZE];
{
        register int x,y;

        for (x = 0; x < RADARSIZE; x++)
            for (y = 0; y < RADARSIZE; y++)
              rm[x][y] = ' ';
} /* presetradarmap */


radarscan(inpf)
FILE *inpf;
{
        int xp, yp;
        float rp;
        int xr, yr;
        char radarmap[RADARSIZE][RADARSIZE];

        while (findshipname(inpf)) {
            if (!readradarlines(inpf, &xp, &yp, &rp)) continue;
            if (rp < 0.6) continue;
            presetradarmap(radarmap);
            if (!readradarscan(inpf, radarmap, &xr, &yr)) continue;
            editradarscan(radarmap, &xr, &yr, &xp, &yp, &rp);
        } /* while */
} /* radarscan */


/*
 * readmap - Read and process the map file.  This is a real kludge where you 
 *           try to figure out where you are on the map by looking at the
 *           coordinates along the edge of the map.
 */
readmap(inpf)
FILE *inpf;
{
        register int x,y;       /* x,y map coordinates */
        int sign;               /* Sign of number being converted */
        char minb[5];           /* Number buffers for min and max x */
        char maxb[5];
        char *sp = minb;
        char *lp = maxb;
        char *bp;

        fgets(buf, sizeof buf, inpf);
        /* Now determine the minimum x value */
        *sp++ = buf[4];
        *lp++ = buf[strlen(buf) - 2];
        sign = (index(buf, '-')) ? -1 : 1;
        fgets(buf, sizeof buf, inpf);
        *sp++ = buf[4];
        *lp++ = buf[strlen(buf) - 2];
        *sp = 0;
        *lp = 0;

        minx = atoi(minb);
        maxx = atoi(maxb);

        if (sign == -1)  {
            if (minx > 0) minx *= -1;
        } else {
            if (minx > maxx) {
                maxx *= -1;
                minx *= -1;
            } /* if */
        } /* if */

        fgets(buf, sizeof buf, inpf);
        miny = atoi(buf);

        for (y = miny; buf[2] != ' '; y++) {
            for (bp = &buf[4], x = minx; x <= maxx; x++, bp++) {
                merge (*bp, x, y);
            } /* for */
            fgets(buf, sizeof buf, inpf);
        } /* for */
} /* readmap */


readradarlines(inpf, xp, yp, rp)
FILE *inpf;
int *xp, *yp;
float *rp;
{
        static char format[] = 
           "%d,%d efficiency %*d%%, barometer at %*d, max range %f";

        if (fscanf(inpf, format, xp, yp, rp) != 3) return(FALSE);
        pitchline(inpf);
        return(TRUE);
} /* readradarlines */


readradarscan(inpf, rm, xr, yr)
FILE *inpf;
char rm[RADARSIZE][RADARSIZE];
int *xr, *yr;
{
        register char c;
        register int x,y;
        register int blankline;
        register int leadingblanks;

        *xr = x = 0;
        *yr = y = 0;
        leadingblanks = TRUE;
        blankline = FALSE;

        while ((c = getc(inpf)) != EOF) {
            switch (c) {
                case '\n': if (!leadingblanks && blankline)
                               return (*xr > 0 && --(*yr) > 0);
                           if (!leadingblanks)
                               if (++y >= RADARSIZE) return(FALSE);
                           x--;
                           *xr = MAX(*xr, x);
                           *yr = MAX(*yr, y);
                           blankline = TRUE;
                           if (x > 0) {
                               x = 0;
                               continue;
                           } /* if */ 
                           /* fall through */

                default:   if (x <= 0) {
                               ungetc (c, inpf);
                               (*yr)--;
                               return (*xr > 0 && *yr > 0);
                           } /* if */
                           blankline = FALSE;
                           leadingblanks = FALSE;
                           /* fall through */

                case ' ':  rm[x][y] = c;
                           if(++x >= RADARSIZE) return(FALSE);
                       
            } /* switch */
        } /* while */
        (*yr)--;
        return (*xr > 0 && *yr > 0);
} /* readradarscan */


/*
 * ship - Read and process ship reports.
 */
ship(inpf)
FILE *inpf;
{
        char des;
        register int number;
        register int i;
        struct sector *mp;
        struct shipentry *sp;

        while (fgets (buf, sizeof buf, inpf) != NULL) {
            if (buf[22] != ',') continue;
            if (!(des = getshiptype (&buf[5]))) continue;
            number = atoi (buf);
            if ((i = readship (UNKNOWN, number)) == NOSHIPS) {
                i = shipcount;
                if (++shipcount >= MAXSHIPS) {
                    fputs ("Ship vector overflow!\n", stderr);
                    fflush (stderr);
                    exit(1);
                } /* if */
                sp = &ships[i];
                sp->des = des;
                sp->number = number;
                sp->x = atoi (&buf[19]);
                sp->y = atoi (&buf[23]);
                sp->fleet = buf[27];
                values.val[COU] = -1;
                values.val[EFF] = atoi (&buf[29]);
                sscanf (&buf[34], "%d%d%d%d%d%d%d%d%d%d%d",
                        &values.val[CIV], &values.val[MIL], &values.val[SH],
                        &values.val[GUN], &values.val[PL], &values.val[IRON],
                        &values.val[DUST], &values.val[BAR], &values.val[FOOD],
                        &values.val[OIL], &values.val[MOB]);
                writeship (i, number);

                mp = &map[offset(sp->x)][offset(sp->y)];
                if (mp->shp == NOSHIPS) mp->shp = i;
                mp->own = 1;
            } /* if */
        } /* while */
} /* ship */


coastwatch(inpf)
FILE *inpf;
{
        char des;
        register int number;
        register int i;
	int j,cnum;
        struct sector *mp;
        struct shipentry *sp;

        while (fgets (buf, sizeof buf, inpf) != NULL) {
	    for (i=0; buf[i] != ' '; i++) ;		/* find country */
	    if (buf[++i] != '(' || buf[++i] != '#') continue;
            cnum = atoi(&buf[++i]);
	    for ( ; buf[i] != ' '; i++) ;		/* find ship type */
            if (!(des = getshiptype (&buf[++i]))) continue;
	    for ( ; buf[i] != '#'; i++) ;		/* find ship number */
            number = atoi (&buf[++i]);
            if ((j = readship (UNKNOWN, number)) == NOSHIPS) {
                j = shipcount;
                if (++shipcount >= MAXSHIPS) {
                    fputs ("Ship vector overflow!\n", stderr);
                    fflush (stderr);
                    exit(1);
                } /* if */
                sp = &ships[j];
                sp->des = des;
                sp->number = number;
		for ( ; buf[i] != '@'; i++) ;		/* find coordinates */
                sp->x = atoi (&buf[++i]);
                sp->y = atoi (&buf[i+4]);
                sp->fleet = ' ';
                values.val[COU] = cnum;
                writeship (j, number);

                mp = &map[offset(sp->x)][offset(sp->y)];
                if (mp->shp == NOSHIPS) mp->shp = j;
                mp->own = 1;
            } /* if */
        } /* while */
} /* coastwatch */


/*
 * spy - Read and process spy reports.
 */
spy(inpf)
FILE *inpf;
{
        register int x,y;
        struct sector *mp;
        register int i;

        while (fgets(buf, sizeof buf, inpf) != NULL) {
            if (buf[3] != ',') continue;
            x = atoi(buf);
            y = atoi(&buf[4]);
            mp = &map[offset(x)][offset(y)];
            readvalues (x,y);
            if (mp->own && values.val[COU] == -1) continue;
            mp->own = 1;
            values.val[COU] = atoi(&buf[8]);
            if (buf[14] == 'N') goto noreport;  /* Ignore "No report... */
            merge (buf[11], x, y);
            values.val[DES] = buf[11];
            values.val[EFF] = atoi(&buf[13]);
            sscanf(&buf[18], "%d%d%d%d%d%d%d",
                   &values.val[CIV], &values.val[MIL], &values.val[SH],
                   &values.val[GUN], &values.val[IRON], &values.val[PL],
                   &values.val[FOOD]);

noreport:   values.val[CHKPT] = ' ';
            for (i = 0; i < DELSIZE; i++) values.del[i] = '.';
            for (i = 0; i < CNTSIZE; i++) values.cnt[i] = '.';
            writevalues(x,y);
        } /* while */
} /* spy */


/*
 * censusinfo - Display census and commodities info for sector.
 */
censusinfo(x,y)
register int x,y;
{
        register struct sector *mp = &map[offset(x)][offset(y)];
        register struct shipentry *sp;

        censusheader(shipmode && mp->shp != NOSHIPS);
        move(LINES-2, 0);
        clrtoeol();
        move(LINES-4, 0);
        clrtoeol();
        readvalues(x,y);
        printw("%3d,%-3d  %c", x, y, mp->des ? mp->des : ' ');
        if (mp->own) {
            printw("%6d%%%4d %c%c%c %c%c%c   %c %4d%4d%5d%5d%5d%5d%4d",
                   values.val[EFF], values.val[MOB], values.del[CIV],
                   values.del[MIL], values.del[FOOD], values.cnt[CIV],
                   values.cnt[MIL], values.cnt[FOOD], values.val[CHKPT],
                   values.val[CIV], values.val[MIL], values.val[FOOD],
                   values.val[MIN], values.val[GMIN], values.val[FERT],
                   values.val[PET]);
            move(LINES-2, 0);
            if (!shipmode || mp->shp == NOSHIPS) {
                if (values.val[COU] == -1) addstr ("   ");
                    else printw("%3d", values.val[COU]);
                if (values.del[SH])
                    printw(" %c%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c%c",
                       values.del[SH], values.del[GUN], values.del[PL],
                       values.del[IRON], values.del[DUST], values.del[BAR],
                       values.del[OIL], values.del[LCM], values.del[HCM],
                       values.cnt[SH], values.cnt[GUN], values.cnt[PL],
                       values.cnt[IRON], values.cnt[DUST], values.cnt[BAR],
                       values.cnt[OIL], values.cnt[LCM], values.cnt[HCM]);
                else
                   addstr("                    ");
                        
                printw("%4d%5d%4d%5d%5d%5d%5d%5d%5d",
                   values.val[SH], values.val[GUN], values.val[PL],
                   values.val[IRON], values.val[DUST], values.val[BAR],
                   values.val[OIL], values.val[LCM], values.val[HCM]);
            } else {
                sp = &ships[mp->shp];
                readship(mp->shp, sp->number);
                if (values.val[COU] == -1) addstr ("   ");
                    else printw("%3d", values.val[COU]);
                printw(" %4d  %c  %3d,%-3d %c %3d%%%4d%4d",
                       sp->number, sp->des, x, y, sp->fleet,
                       values.val[EFF], values.val[CIV], values.val[MIL]);
                printw("%4d%4d%4d%4d%4d%4d%4d%4d%4d",
                       values.val[SH], values.val[GUN], values.val[PL],
                       values.val[IRON], values.val[DUST], values.val[BAR],
                       values.val[FOOD], values.val[OIL], values.val[MOB]);
            } /* if */
        } /* if */
} /* censusinfo */


/*
 * censusheader - Display census header.
 */
censusheader(mode)
register int mode;
{
        move(LINES-5, 0);
        addstr("  sect  des   eff mob cmf cmf % * ");
        addstr(" civ mil food  min gmin fert oil");
        move(LINES-3, 0);
        clrtoeol();
        if (!mode) {
            addstr("cou sgpidbolh sgpidbolh  ");
            addstr("sh  gun  pl iron dust  bar  oil  lcm  hcm");
        } else {
            addstr("cou    # des   x,y   f  ");
            addstr("eff civ mil  sh gun pln irn dst gld food oil mu");
        } /* if */
} /* censusheader */


/*
 * center - Center display window about (x,y).
 */
center(x,y)
register int x,y;
{
        if (x - 10 < startx || x + 10 > startx + MCOLS ||
            y -  4 < starty || y +  4 > starty + MLINES) {
            startx = x - MCOLS/2;
            starty = y - MLINES/2;
        } /* if */
        return; 
} /* center */


/*
 * clearmks - Clear sector marks.  If 'all' flag is set, then ALL marks
 *            cleared, otherwise just curmark marks are cleared.
 */
clearmks(all)
int all;
{
        register int x,y;

        for (y = 0; y < MAPSIZE; y++)
            for (x = 0; x < MAPSIZE; x++)
                if (all || map[x][y].mark == curmark) map[x][y].mark = 0;

} /* clearmks */


/*
 *      closetf - close temporary file.
 */
closetf()
{
        close(temp);
} /* closetf */


/*
 * commands - Process input commands.
 */
commands()
{
        register char c;                            
        int x,y;                    /* Indexes into map array */
        register int i;
        register struct item *ip;   /* Pointer to current item */
        int tx,ty;                  /* Temporary x,y */
        int crou = -1;              /* Current route */
        int status;                 /* Fork status return */
        int pflg = 1;               /* Print census flag */
        int update = 0;             /* Set if screen update needed */
        struct funsw *fp;           /* Pointer to input file function process */
        char *bp;
        char prbuf[BUFSIZ];
        char *getenv();

        if (map[offset(0)][offset(0)].own)
            center (x = 0, y = 0);
        else {
            x = minx + MCOLS/2 - ODD(minx + MCOLS/2);
            y = miny + MLINES/2 - ODD(miny + MLINES/2);
            center (x,y);
        } /* if */

        mapdr(NOSU);            /* Draw map, census header and census */
        censusinfo(x,y);

        move(y-starty, x-startx);

        while (refresh(), ((c = getac()) != 'q')) {
            update = 0;         /* Initialize variables for next command */
            curx = x;
            cury = y;
            switch(c) {
                /* Movement commands */
                case 'y':       x--;
                                y--;
                                break;

                case 'u':       x++;
                                y--;
                                break;

                case 'j':       x += 2;
                                break;

                case 'n':       x++;
                                y++;
                                break;

                case 'b':       x--;
                                y++;
                                break;

                case 'g':       x -= 2;
                                break;

                case CTRL(B):
                case CTRL(N):   y += 6;
                                break;

                case CTRL(Y):
                case CTRL(U):   y -= 6;
                                break;

                case CTRL(G):   x -= 6;
                                break;

                case CTRL(J):   x += 6;
                                break;

                case '\f':      clear();        /* Redraw the screen */
                                mapdr(surmap);
                                censusinfo(x,y);
                                break;

                case '?':       query();        /* Do query at bottom */
                                mapdr(surmap);
                                censusinfo(x,y);
                                break;

                case '!':                       /* Fork a shell */
                                if ((bp = getenv("SHELL")) == NULL)
                                    strcpy(prbuf, SHPATH);
                                else
                                    strcpy(prbuf, bp);
                                if (outf) fclose(outf);
                                if (fork() == 0) {
                                    move(LINES-1, 0);
                                    clrtoeol();
                                    refresh();
                                    endtty();
                                    execl(prbuf, prbuf, 0);
                                } else
                                    wait(&status);
                                presetty();
                                clear();
                                mapdr(surmap);
                                censusinfo(x,y);
                                if (outf)
                                    if ((outf = fopen(oname, "a")) == NULL)
                                        putline("Cannot reopen %s", oname);
                                break;

                case '+':       nextship(x,y);      /* Advance to next ship */
                                update++;
                                break;

                case '-':       previousship(x,y);  /* Back up to prev ship */
                                update++;
                                break;

                case '/':       firstship(x,y);     /* Go to first ship */
                                update++;
                                break;

                case 'C':
                case 'c':       clearmks(c=='C'); /* Clear marks */
                                mapdr(surmap);
                                break;

                case 'M':       curmark = '>';  /* Reset mark */
                                break;

                case 'm':       getline(buf,"mark: ",NOX); /* Change mark */
                                if (*buf) curmark = *buf;
                                break;

                case 'G':       locateship (&x,&y,surmap); /* Leap to ship */
                                update++;
                                break;

                case 'P':       pflg = (pflg == 0);  /* Toggle census refresh */
                                move(LINES-1, 0);
                                clrtoeol();
                                printw("Printing %s", pflg ? "on" : "off");
                                break;

                case 'N':                           /* Toggle ship display */
                                shipmode = (shipmode == FALSE);
                                mapdr(surmap);
                                censusinfo(x,y);
                                break;

                case 'a':       if (outf) {     /* Append to the file */
                                    getline(buf, "", EX);
                                    x = curx;
                                    y = cury;
                                    if (*buf) {
                                        strcat(buf, "\n");
                                        fputs(buf, outf);
                                    } /* if */
                                } else
                                putline("No output file specified - use O");
                                break;

                case 'i':                   /* Read in input data (map,...) */
                case 'I':
                                getline(buf, "New input file: ", NOX);
                                if ((inpf = fopen(buf, "r")) == NULL) {
                                        putline("%s: cannot open", buf);
                                        break;
                                }
                                strcpy(iname, buf);
                                if (fgets(buf, sizeof buf, inpf) == NULL ||
                                    fgets(buf, sizeof buf, inpf) == NULL) {
                                        putline("%s: wrong format\n", iname);
                                        break;
                                }

                                noise = FALSE;  /* don't report production */
                                for (fp = fsw; *fp->type; fp++)
                                    if (!strncmp(fp->type, buf,
                                        strlen(fp->type))) (fp->func)(inpf);
                                noise = TRUE;
                                fclose(inpf);
                                mapdr(surmap);
                                censusinfo(x,y);
                                break;
                                
                case 'o':                   /* Change/create output file */
                case 'O':
                                getline(buf, "New output file: ", NOX);
                                if (outf) fclose(outf);
                                if ((outf = fopen(buf, "a")) == NULL)
                                    putline("%s: cannot create", buf);
                                else
                                    strcpy(oname, buf);
                                break;

                case 's':                   /* Set a macro */
                                getline(buf, "macro name: ", NOX);
                                getline(prbuf, "define: ", NOX);
                                strcopy(prbuf,macros[*buf]);
                                break;

                case 'd':               /* Delete a macro */
                                getline(buf, "delete macro: ", NOX);
                                *macros[*buf] = 0; 
                                break;

                case 'V':       if ((bp = getenv("VISUAL")) == NULL)
                                    strcpy(prbuf, VIPATH);
                                else
                                    strcpy(prbuf, bp);
                                goto forkeditor;

                case 'E':       if ((bp = getenv("EDITOR")) == NULL)
                                    strcpy(prbuf, EXPATH);
                                else
                                    strcpy(prbuf, bp);

forkeditor:                     if (outf) {
                                    fclose(outf);
                                    move(LINES-1, 0);
                                    clrtoeol();
                                    refresh();
                                    if (fork() == 0) {
                                        endtty();
                                        execl(prbuf, prbuf, oname, 0);
                                    } else
                                        wait(&status);
                                    presetty();
                                    clear();
                                    mapdr(surmap);
                                    censusinfo(x,y);
                                    if ((outf = fopen(oname, "a")) == NULL)
                                        putline("Cannot reopen %s", oname);
                                } else putline("No output file");
                                break;

                case 'S':                       /* Survey */
                                getline(buf, "Survey: ", NOX);
                                survey(buf);
                                mapdr(surmap = SURV);
                                break;

                case 'R':                       /* Range for Survey */
                                sprintf(prbuf, "Range (%d): ", range*10);
                                getline(buf, prbuf, NOX);
                                if (*buf) {
                                    range = atoi(buf)/10;
                                    if (range < 1) {
                                        putline("range should be >= 10");
                                        range = 10;
                                    } /* if */
                                } /* if */
                                break;

                case CTRL(F):                   /* Flip maps */
                                mapdr((surmap = (surmap == NOSU)));
                                break;

                case 'r':                       /* Trace route */
                                getline(buf, "Route: ");
                                for (crou = -1, ip = items; ip->len; ip++)
                                    if (!strncmp(buf, ip->nm, ip->len))
                                        crou = ip - items;
                                if (crou < 0)
                                    putline("I don't know about %s", buf);
                                break;

                case 'w':                       /* Walk along route */
                                readvalues (x,y);
                                if (crou > -1 && values.del[crou] != '.')
                                    peekc = values.del[crou];
                                break;
                                
                case 'p':       break;          /* Print census */

                case 'l':       getline(buf, "Leap to: ");
                                if (*buf && (bp = (char *)index(buf, ','))) {
                                    tx = atoi(buf) ;
                                    ty = atoi(++bp);
                                    if (!VALID(tx, ty)) {
                                        tx -= ODD(tx);
                                        ty -= ODD(ty);
                                    } /* if */
                                    x = tx;
                                    y = ty;
                                    center(x,y);
                                    update++;
                                } /* if */
                                break;
            } /* switch */

            if (x < startx || x > startx+MCOLS) {
                startx = (x < startx) ? x : x-MCOLS;
                update++;
            } /* if */
            if (y < starty || y > starty+MLINES) {
                starty = (y < starty) ? y : y-MLINES;
                update++;
            } /* if */

            if (update) {
                mapdr(surmap);
                censusinfo(x,y);
                touchwin(stdscr);
            } else {
                if (pflg || c == 'p') censusinfo(x,y);
            } /* if */
            move(y-starty, x-startx);
        } /* while */
} /* commands */


/*
 * endtty - Restore terminal to normal mode.
 */
endtty()
{
        nl();
        echo();
        crmode();
        endwin();
        closetf();
} /* endtty */


/*
 * findblank - Find next blank character.
 */
char *findblank(addr)
register char *addr;
{
        while ((*addr != 0) && (*addr != ' ')) addr++; 
        return (addr);
} /* findblank */


/*
 * findchar - Find next non-blank character.
 */
char *findchar(addr)
char *addr;
{
        while ((*addr != 0) && (*addr == ' ')) addr++; 
        return (addr);
} /* findchar */


/*
 * findship - Find ship or first ship of fleet.
 */
findship(ship,fleet) 
register int ship;
register char fleet;
{
        register int i;

        if (ship == UNKNOWN) {
            for (i = 0; i < shipcount; i++)
                if (ships[i].fleet == fleet) {
                    ship = ships[i].number;
                    break;
                } /* if */
        } /* if */
        if (ship == UNKNOWN) return (NOSHIPS);
        return (readship (UNKNOWN, ship));
} /* findship */        


/*
 * firstship - Locate first ship at x,y.
 */
firstship(x,y)
int x,y;
{
        register int mx,my;
        register int i;
        struct sector *mp;
        struct shipentry *sp;

        if (!shipmode) return;
        mx = offset(x);
        my = offset(y);
        mp = &map[mx][my];
        if (mp->shp == NOSHIPS) return;
        for (i = 0; i < shipcount; i++) {
            sp = &ships[i];
            if (offset (sp->x) == mx && offset(sp->y) == my) {
                mp->shp = i;
                return;
            } /* if */
        } /* for */
} /* firstship */


/*
 * getac - Get a character. Return peekc if non-zero,
 *         otherwise read a character from the keyboard.
 */
getac()
{
        register char tc;

        if (peekc) {
            tc = peekc;
            peekc = 0;
            return(tc);
        } else
            return(getch());

} /* getac */


/*
 * getline - Get input line from the bottom of the screen,
 *           using pr as a prompt if non-zero. If ex is set,
 *           then expand macros.
 */
getline(bp, pr, ex)
char *bp;
char *pr;
int  ex;
{
        register int x,y;
        register char c;
        char *mp;
        char *np;
        char nbuf[10];          /* Number buffer */
        char processmove();
        char *ip = bp;

        move(LINES-1, 0);
        clrtoeol();
        if (*pr) addstr(pr);
        while (refresh(), (c = getch()) != '\r') {
            if (ex && *macros[c]) {         /* check for macros */
                mp = macros[c];
                while (*mp) {
                    if (*mp == '.') {       /* expand . */
                        mp++;
                        sprintf(np = nbuf, "%d,%d ", curx, cury);
                        while (*ip++ = *np++);
                        ip--;
                        addstr(nbuf);
                    } else {
                        addch( *ip++ = *mp++);
                    } /* if */
                } /* while */
                continue;
            } /* if */

            switch(c) {
                case '\b':      if (ip > bp) {  /* backspace */
                                    ip--;
                                    *ip = 0;
                                    addstr("\b \b");
                                } /* if */
                                continue;

                case '\\':      addstr("\\\b"); /* backslash */
                                refresh();
                                c = getch();
                                break;

                case '.':       if (ex) {       /* expand . */
                                    sprintf(np = nbuf, "%d,%d ", curx, cury);
                                    while (*ip++ = *np++);
                                    ip--;
                                    *ip = 0;
                                    addstr(nbuf);
                                    continue;
                                }
                                else break;

                case ESC:       getyx(stdscr,y,x); /* jump to current x,y */
                                move(cury-starty, curx-startx);
                                refresh();
                                sleep(1);
                                move(y, x);
                                continue;

                case '\n':      continue;

                case CTRL(P):   if (ex) c = processmove (bp, &ip);
                                if (c > ' ') break;
                                continue;

                case '@':       move(LINES-1, 0);   /* erase the line */
                                if (*pr) addstr(pr);
                                clrtoeol();
                                *(ip = bp) = 0;
                                continue;
            } /* switch */
            addch(*ip++ = c);
            *ip = 0;
        } /* while */
        *ip = 0;
} /* getline */


/*
 * getnewxy - Get last postion of MOVE or NAV command.
 */
getnewxy(bp,x,y)
char *bp;
int  *x,*y;
{
        register char *ip = bp;
        char *findblank();
        char *findchar();
        char fleet;
        char xbuf[6];
        char ybuf[6];
        int oldy,oldx;
        int shipnumber;
        register int i;

        switch(*ip) {
            case 'm':   if (index(ip,'mov') == 0) return(0);
                        if (*(ip = findblank(ip)) == 0) return(0);
                        if (*(ip = findchar (ip)) == 0) return(0);
                        if (*(ip = findblank(ip)) == 0) return(0);
                        if (*(ip = findchar (ip)) == 0) return(0);
                        for (i = 0; (i < 5) && (*ip != ',') && (*ip != 0);
                            i++) {
                            xbuf[i] = *ip++;
                            xbuf[i+1] = 0;
                        } /* for */
                        if ((*ip != ',') || (*ip == 0)) return(0);
                        ip++;
                        for (i = 0; (i < 5) && (*ip != ' ') && (*ip != 0);
                            i++) {
                            ybuf[i] = *ip++;
                            ybuf[i+1] = 0;
                        } /* for */
                        if ((*ip != ' ') || (*ip == 0)) return(0);
                        *x = atoi(xbuf);
                        *y = atoi(ybuf);
                        if (*(ip = findchar(ip))  == 0) return(0);
                        if (*(ip = findblank(ip)) == 0) return(0);
                        if (*(ip = findchar(ip))  == 0) return(1);
                        break;

           case 'n':    if (index(ip,'nav') == 0) return(0);
                        if (*(ip = findblank(ip)) == 0 ) return (0);
                        if (*(ip = findchar (ip)) == 0 ) return (0);
                        shipnumber = UNKNOWN;
                        fleet = *ip;
                        if ((fleet >= '0') && (fleet <= '9')) {
                            for (i = 0; (i <5 ) && (*ip != '/') &&
                                        (*ip != ' ') && (*ip != 0);
                                i++) {
                                xbuf[i] = *ip++;
                                xbuf[i+1] = 0;
                            } /* for */
                            shipnumber = atoi(xbuf);
                        } /* if */
                        if ((i = findship(shipnumber,fleet)) == NOSHIPS)
                            return(0);
                        *x = ships[i].x;
                        *y = ships[i].y;
                        if (*ip != ' ') ip = findblank(ip);
                        if (*ip == 0) return(0);
                        shipmode = TRUE;
                        map[offset(*x)][offset(*y)].shp = i;
                        getyx(stdscr,oldy,oldx);
                        curx = *x;
                        cury = *y;
                        center(curx,cury);
                        mapdr(surmap);
                        touchwin(stdscr);
                        censusinfo(curx,cury);
                        move(oldy,oldx);
                        if (*(ip = findchar(ip)) == 0) return(1);
                        break;
        
           default:     return(0);
        } /* switch */

        while (*ip != 0) { 
            switch(*ip) {
                case 'y':       (*x)--;
                                (*y)--;
                                break;

                case 'u':       (*x)++;
                                (*y)--;
                                break;

                case 'j':       *x += 2;
                                break;
                        
                case 'n':       (*x)++;
                                (*y)++;
                                break;

                case 'b':       (*x)--;
                                (*y)++;
                                break;
                             
                case 'g':       *x -= 2;
                                break;
                        
                default:        return(0);
            } /* switch */
        ip++;
        } /* while */
        return(1);
} /* getnewxy */


/*
 * locateship - Locate arbitrary ship and move display window.
 */
locateship(sx,sy,sflg)
register int *sx,*sy;
int sflg;
{

        register int number;
        register int i;
        char fleet;

        getline (buf, "Ship number: ", NOX);
        if (*buf) {
            fleet = buf[0];
            if (fleet >= '0' && fleet <= '9')
                number = atoi (buf);
            else
                number = UNKNOWN;
            if ((i = findship (number, fleet)) == NOSHIPS) {
                putline("No info on ship");
                return;
            } /* if */
            *sx = ships[i].x;
            *sy = ships[i].y;
            center(*sx,*sy);
            map[offset(*sx)][offset(*sy)].shp = i;
            if (!shipmode) {
                shipmode = TRUE;
                mapdr(sflg);
            } /* if */
            censusinfo(*sx,*sy);
            return;
        } /* if */
} /* locateship */


/*
 * mapdr - Display map.
 */
mapdr(sflg)
register int sflg;
{
        register int x,y;
        register char des;
        register struct sector *mp;

        for (y = starty; y <= starty + MLINES; y++)
            for (x = startx; x <= startx + MCOLS; x++) {
                mvaddch(y-starty, x-startx, ' ');
                if (!VALID(x, y)) continue;
                mp = &map[offset(x)][offset(y)];
                if (shipmode && mp->shp != NOSHIPS)
                    des = ships[mp->shp].des;
                else
                    if (sflg && mp->surv)
                        des = mp->surv;
                    else
                        des = mp->des;
                mvaddch(y-starty, x-startx-1, (mp->mark)?mp->mark:' ');
                if (des) mvaddch(y-starty, x-startx, des);
            } /* for */
} /* mapdr */


/*
 * mark - Mark map according to command.
 */
mark(sp, pass)
char *sp;
int pass;
{
        register int itm1;
        register int itm2;
        register int x,y;
        register int num;
        register int val;
        register int markit;
        register struct item *ip;
        register struct item *tp;
        char cmd[20];
        char *cp = cmd;
        struct sector *mp;

        while (isalpha(*cp++ = *sp++));         /* get first word */
        *--cp = 0;
        if (!*--sp) return;
        for (ip = items; ip->len; ip++)         /* check it */
            if (!strncmp(cmd, ip->nm, ip->len)) break;

        if (!ip->len) {
            putline("I don't know about %s\n", cmd);
             return;
        } /* if */

        itm1 = ip - items;
        /*
         * At this point, cmd contains the left side, *sp
         * is the operator and sp+1 is the right side.
         */
        num = (itm1 == DES) ? *(sp+1) : atoi(sp+1);
        if (itm1 == CONT || itm1 == DEL) {
            for (tp = items; tp->len; tp++)
                if (!strncmp(sp+1, tp->nm, tp->len)) break;
            if (!tp->len) {
                putline("I don't know about %s", cmd);
                return;
            } /* if */
            itm2 = tp - items;
        } /* if */
        for (y = 0; y < MAPSIZE; y++)
            for (x = 0; x < MAPSIZE; x++) {
                mp = &map[x][y];
                if (!VALID(x, y) || !mp->own || mp->des == '.') continue;
                readvalues (x,y);
                if (values.val[COU] != -1) continue;
                markit = 0;
                val = values.val[itm1];
                if (itm1 == CONT) {
                    num = '$';
                    val = values.cnt[itm2];
                } else {
                    if (itm1 == DEL) {
                        num = '.';              /* KLUDGE */
                        *sp = '#';
                        val = values.del[itm2];
                    } /* if */
                } /* if */
                switch( *sp) {
                    case '=':   if (val == num) markit++;
                                break;

                    case '#':   if (val != num) markit++;
                                break;

                    case '>':   if (val > num)  markit++;
                                break;

                    case '<':   if (val < num)  markit++;
                                break;
                } /* switch */
                if (markit && (!pass || (pass && mp->mark)))
                    mp->mark = curmark;
                else
                    if (mp->mark == curmark) mp->mark = 0;
            } /* for */
} /* mark */


/*
 * nextship - Advance ship index to next ship at x,y.
 */
nextship(x,y)
int x,y;
{
        register int mx,my;
        register int i;
        struct sector *mp;
        struct shipentry *sp;

        if (!shipmode) return;
        mx = offset(x);
        my = offset(y);
        mp = &map[mx][my];
        if (mp->shp == NOSHIPS) return;

        for (i = mp->shp+1; i < shipcount; i++) {
            sp = &ships[i];
            if (offset(sp->x) == mx && offset(sp->y) == my) {
                mp->shp = i;
                return;
            } /* if */
        } /* for */

        for (i = 0; i < mp->shp; i++) {
            sp = &ships[i];
            if (offset(sp->x) == mx && offset(sp->y) == my) {
                mp->shp = i;
                readship (i, sp->number);
                return;
            } /* if */
        } /* for */
} /* nextship */


/*
 * offset - Return transformed coordinate. 
 */
offset(coordinate)
int coordinate;
{       
        register int modulo;
        
        modulo = coordinate%MAPSIZE;
        return  (modulo < 0 ? modulo+MAPSIZE : modulo);

} /* offset */


/*
 *      opentf - open temporary file.
 */
opentf()
{
        sprintf(tfile, "%s%d", TEMPFILE, getpid());
        temp = creat(tfile, 0600);
        if (temp < 0) {
            write(2, "can't create temp file\n", 23);
            exit(1);
        } /* if */
        /*
         * Re-open tfile in read/write mode. This shouldn't be
         * necessary. It wouldn't be if some blooming idiot hadn't
         * made creat() and open() be two different system calls.
         * (At very least creat() should be able to return a file
         * descriptor which can be used both to read and to write.)
         */
        close(temp);   
        temp = open(tfile, 2);
        if (temp < 0) {
            unlink(tfile);
            write(2, "can't open temp file\n", 22);
            exit(1);
        } /* if */
        unlink(tfile);
} /* opentf */


pitchline(inpf)
FILE *inpf;
{
        register int c;

        while ((c = getc(inpf)) != '\n')
            if (c == EOF) break;

} /* pitchline */


/*
 * presetmap - Preset map array.
 */
presetmap()
{
        register int x,y;
        register int i;
        struct shipentry *sp;
        
        for (y = 0; y < MAPSIZE ; y++)
            for (x = 0; x < MAPSIZE ; x++) {
                map[x][y].des = 0;
                map[x][y].own = 0;
                map[x][y].shp = NOSHIPS;
                map[x][y].diskaddr = NODISK;
            } /* for */

        for (i = 0; i < MAXSHIPS; i++) {
            sp = &ships[i];
            sp->des = 0;
            sp->number = UNKNOWN;
            sp->diskaddr = NODISK;
        } /* for */

} /* presetmap */


/*
 * presetty - Set up terminal for display mode.
 */
presetty()
{
        crmode();
        noecho();
        nonl();
} /* presetty /*


/*
 * previousship - Back up to previous ship at x,y.
 */
previousship(x,y)
int x,y;
{
        register int mx,my;
        register int i;
        struct sector *mp;
        struct shipentry *sp;

        if (!shipmode) return;
        mx = offset(x);
        my = offset(y);
        mp = &map[mx][my];
        if (mp->shp == NOSHIPS) return;

        for (i = mp->shp-1; i >= 0; i--) {
            sp = &ships[i];
            if (offset(sp->x) == mx && offset(sp->y) == my) {
                mp->shp = i;
                return;
            } /* if */
        } /* for */

        for (i = shipcount-1; i > mp->shp; i--) {
            sp = &ships[i];
            if (offset(sp->x) == mx && offset(sp->y) == my) {
                mp->shp = i;
                readship (i, sp->number);
                return;
            } /* if */
        } /* for */
} /* previousship */


/*
 * processmove - process MOVE or NAV command.
 */
char processmove(bp,ip)
char *bp;
char **ip;
{
        register char c;
        int x,y;
        int oldx,oldy;  

        if (getnewxy(bp,&x,&y) == 0) return(0);
        getyx(stdscr,oldy,oldx);
        updatescreen(x,y);
        while (refresh(),(c = getch()) != -1 ) {
            switch (c) {
                case 'y':       x--;
                                y--;
                                break;
               
                case 'u':       x++;
                                y--;
                                break;

                case 'j':       x += 2;
                                break;

                case 'n':       x++;
                                y++;
                                break;

                case 'b':       x--;
                                y++;
                                break;

                case 'g':       x -= 2;
                                break;


                default:        updatescreen(curx,cury);
                                move(oldy,oldx); 
                                refresh();
                                return(c);
            } /* switch */
            move(oldy,oldx);
            **ip = c;
            (*ip)++;
            **ip = 0;
            addch(c);
            oldx++; 
            updatescreen(x,y);  
        } /* while */
} /* processmove */


/*
 * putline - Do a printw at the bottom of the screen.
 */
putline(fmt, a1, a2, a3, a4)
{
        move(LINES-1, 0);
        printw(fmt, a1, a2, a3, a4);

} /* putline */


/*
 * query - Parse ? command.
 */
query()
{
        register int pass = 0;
        char *bp = buf;
        char *tp;

        getline(bp, "?", NOX);
        if (!*bp) return;
        clearmks(0);
        for (;;) {
            if (tp = (char *)index(bp, '&')) {
                *tp++ = 0;
                 mark(bp, pass);
                 bp = tp;
            } else {
                 mark(bp, pass);
                 break;
            } /* if */
            pass++;
        } /* for */
} /* query */


/*
 * readship - Read ship values from temporary file.
 */
readship(ix, number)
register int ix;                    /* Index into ship vector if known */
register int number;                /* Ship number (used if index unknown) */
{
        register int i;

        for (i = 0; i < VALUESIZE; i++) values.val[i] = 0;
        for (i = 0; i < DELSIZE; i++)   values.del[i] = 0;
        for (i = 0; i < CNTSIZE; i++)   values.cnt[i] = 0;

        if ((i = ix) == UNKNOWN) {
            for (i = 0; i < shipcount; i++)
                if (ships[i].number == number) goto match;
            return (NOSHIPS);
        } /* if */

match:  if (ships[i].diskaddr == NODISK) return (i);
        lseek (temp, ships[i].diskaddr, 0);
        read  (temp, &values, sizeof (struct value));
        return (i);
} /* readship */


/*
 * readvalues - Read sector values from temporary file.
 */
readvalues (x,y)
int x,y;
{
        register int xt,yt;
        register int i;

        xt = offset(x);
        yt = offset(y);
        
        if (map[xt][yt].diskaddr == NODISK) {
            for (i = 0; i < VALUESIZE; i++) values.val[i] = 0;
            for (i = 0; i < DELSIZE; i++)   values.del[i] = 0;
            for (i = 0; i < CNTSIZE; i++)   values.cnt[i] = 0;
            return;
        } /* if */

        lseek(temp,map[xt][yt].diskaddr,0);
        read(temp,&values,sizeof (struct value));

} /* readvalues */


skipword(inpf)
FILE *inpf;
{
        register char c;

        for (;;) {
            c = getc(inpf);
            if (c == '\n' || c == ' ' || c == '\t') return(1);
            if (c == EOF) return(0);
        } /* for */
} /* skipword */


/*
 * strcopy - copy string.
 */
strcopy (str,dst)
register char *str,*dst;
{
        while(*dst++ = *str++ );
}


/*
 * survey - Display survey info for a given item.
 */
survey(sp)
char *sp;
{
        register struct item *ip;
        register int itm;
        register int x,y;
        register struct sector *mp;
        register int surval;

        for (ip = items; ip->len; ip++)
            if (!strncmp(sp, ip->nm, ip->len)) break;

        if (!ip->len) {
            putline("I don't know about %s", sp);
            return;
        } /* if */

        itm = ip - items;
        for (y = 0; y < MAPSIZE; y++)
            for (x = 0; x < MAPSIZE; x++) {
                mp = &map[x][y];
                if (mp->own && mp->des != '.') {
                    readvalues (x,y);
                    surval = values.val[itm]/range;
                    if (surval > 35) mp->surv = '$';
                    else {
                        mp->surv = surval;
                        mp->surv += (mp->surv > 9) ? ('A'-10) : '0';
                    } /* if */
                } /* if */
            } /* for */
} /* survey */


/*
 * updatescreen - Update screen if display window moved. 
 */
updatescreen(x,y)
register int x,y;
{
        register int update = 0;
        
        if (x < startx || x > startx + MCOLS) {
            startx = (x < startx) ? x : x - MCOLS;
            update++;
        } /* if */
        
        if (y < starty || y > starty + MLINES) {
            starty = (y < starty) ? y : y - MLINES;
            update++;
        } /* if */
        
        if (update) {
            mapdr(surmap);
            touchwin(stdscr);
        } /* if */
        censusinfo(x,y);
        move(y-starty,x-startx);
        refresh();
} /* updatescreen */
                        

/*
 * writeship - Write ship values to temporary file.
 */
writeship(ix, number)
register int ix;                    /* Index into ship vector if known */
register int number;                /* Ship number (used if index unknown) */
{
        register int i;
        long tell();

        if ((i = ix) == UNKNOWN) {
            for (i = 0; i < shipcount; i++)
                if (ships[i].number == number) goto match;
            return (NOSHIPS);
        } /* if */

match:  if (ships[i].diskaddr == NODISK) {
            lseek (temp, 0L, 2);
            ships[i].diskaddr = tell (temp);
        } else
            lseek (temp, ships[i].diskaddr, 0);
        write (temp, &values, sizeof (struct value));
        return (i);
} /* writeship */


/*
 * writevalues - write sector values to temporary file.
 */
writevalues(x,y)
int x,y;
{
        register int xt,yt;
        long tell();

        xt = offset(x);
        yt = offset(y);

        if(map[xt][yt].diskaddr == NODISK) {
            lseek(temp,0L,2);
            map[xt][yt].diskaddr = tell(temp);
        } else
            lseek(temp,map[xt][yt].diskaddr,0);
        write(temp,&values,sizeof (struct value));

} /* writevalues */


