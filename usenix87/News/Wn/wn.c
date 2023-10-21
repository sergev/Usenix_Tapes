/*
 * compare the news active file and the caller's newsrc to see if
 * there is news and in what groups the news exists
 * see routine uhoh() for usage instructions
 *
 * David Elins 6 January 1986
 *
 * This program works as follows:
 *
 * Options are parsed (se uhoh() for the meaning of the options
 * and the name of the user's .newsrc file is determined
 * By default, this name is $HOME/.newsrc. It may be changed by
 * an option or by specifying a non-option argument (which is
 * assumed to be a username, the .newsrc from that user's $HOME
 * is used).
 *
 * The news active file is read and stored in a linked list.
 * As of 6 January 1986, this list is an array of structures stored
 * on the stack. Perhaps in the future it will be changed to a malloc/realloc
 * scheme at a later date. The newsgroup names in the active file are
 * also stored on the stack and may be malloc'd at a later date.
 *
 * The user's .newsrc file is opened. It is assumed to be
 * in the format used by Larry Wall's 'rn' program (though this
 * program sort-of works with vnews,readnews format also). Each
 * line of .newsrc is parsed and, the unread article numbers are
 * determined based on the information from the active file.
 *
 * Implementation details:
 *
 * Like 'rn', this program will deal with gaps in the ranges of
 * unread article numbers. A 'range' in a .newsrc is delimited
 * by commas, spaces, end-of-line. A range consists either of
 * a single number or two numbers separated by a hyphen.
 *
 * The linked list is not sorted, there is no need. Newsgroups are
 * scanned in the order they appear in .newsrc. After a newsgroup
 * from .newsrc is processed, its entry in the linked list is
 * removed. This makes the time needed to search the linked list
 * faster as the program progresses.
 *
 * exit status:
 * if there is news, this program exits with a no error
 * status (0). if there is no news, this program exits with error
 * status (1). This is so it can be included in shell scripts that
 * perform some other action dependent on whether or not there is news.
 * However, the program will exit with a different non-zero status
 * if it encounters a missing file or other irregularity.
 */
#define TRUE 1
#define FALSE 0
#define ACTIVE      "/usr/lib/news/active"
#define NEWSRC      ".newsrc"
#define STRSPACE    16384   /* space for strings */
#define MAXGRPS     400     /* maximum number of newsgroups allowed */
#define EOS '\0'
#define EXIT_OK     0
#define EXIT_NONEWS 1
#define EXIT_ERR    2
#define EQS(str1,str2)  (strcmp((str1),(str2))==0)
#define EQN(str1,str2)  (strncmp((str1),(str2),strlen(str2))==0)
#define PLURAL(many)    ((many) ? "s" : "")
#define VERB(many)      ((many) ? "are" : "is")

/* debugging macros */
#define PR1(ff,v0)      (void)fprintf(stderr,"v0=%ff ",(v0))
#define DB1(ff,v0)      (void)fprintf(stderr,"v0=%ff\n",(v0))
#define PR2(ff,v1,v2)   PR1(ff,v1),PR1(ff,v2)
#define DB2(ff,v1,v2)   PR1(ff,v1),DB1(ff,v2)

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
extern struct passwd *getpwnam();
extern void perror(),exit(),insgrp(),showgrps(),whatsleft(),dbgrps(),uhoh();
extern char *asctime(),*strcpy(),*strcat(),*strchr();
extern char *strncpy(),*strtok(),*strrchr(),*getenv();
extern char *basename(),*mytok();
extern long atol();
extern struct tm *localtime();

/* for a linked list of newsgroups in the active file */
struct grp {
    char *name;         /* group name */
    struct grp *next;   /* link to next group */
    long lowest;        /* lowest numbered article */
    long highest;       /* highest numbered article */
};

static char active[]=ACTIVE;
static struct grp *actvlist;    /* will be array of group nodes */
static char *progname;          /* who am I */
/* head of list with  group name lexically preceeding all names */
/* 'next' NULL means end of list */
static struct grp head={
    "",NULL,0,0};
static int newind = 0;          /* index of new group in linked list */

static char *work;              /* pointer to space for new group names */
static char *wstart;            /* keep track of start of string work space */

main(argc, argv)
int argc;                       /* how many command line arguments */
char *argv[];                   /* command line arguments */
{
    char buf[256];
    extern long time();
    long tm=time((long *) 0);
    register char *cp;          /* general character pointer */
    register long lowest;       /* lowest article number in a news group */
    register long highest;      /* highest article number in a news group */
    register long articles=0;   /* total article count */
    int ingrp;                  /* articles in one group */
    int newsgrps=0;             /* total newsgroup count */
    int chkgrps=0;              /* check for groups in active, not in .newsrc */
    int summary=0;              /* summary report only */
    int showall=0;              /* show full information */
    int debug=0;                /* debugging ? */
    char newsrc[64];            /* full name of user's .newsrc file */
    struct grp grpspace[MAXGRPS];   /* allocate stack space for groups */
    char strspace[STRSPACE];    /* allocate stack space for group names */
    struct stat statbuf;        /* file status of active file */
    char *rcnum;                /* articles read as shown in .newsrc */
    int inrc;                   /* news group is enabled in .newsrc ? */
    int exitstat;               /* exit status */

    progname = basename(argv[0]);   /* save program name */
    (void)printf("%s - %s", progname, asctime(localtime(&tm))); /* heading */

    /* parse any options (not quite getopt, but portable) */
    newsrc[0] = '\0';        /* used as an indicator */
    for (; argc > 1 && (argv[1][0] == '-'); argc--,argv++) {
        for (cp = &argv[1][1]; *cp; cp++) {
            switch (*cp) {
            case 'a':           /* per group unread message count */
                showall = 1;
                break;
            case 'c':           /* check for newsgroups missing from .newsrc */
                chkgrps = 1;
                break;
            case 's':           /* just a quick summary message */
                summary = 1;
                break;
            case 'x':           /* turn debugging on */
                debug = 1;
                break;
            case 'f':           /* file specified instead of .newsrc */
                if (cp[1]) {    /* filename included in option string */
                    (void)strcpy(newsrc,&cp[1]);
                    cp += strlen(cp) - 1;   /* so inner loop will terminate */
                    break;
                }
                if ((argc > 2) && (argv[2][0] != '-')) { /* filename is next */
                    (void)strcpy(newsrc,argv[2]);        /* argument */
                    argc--;
                    argv++;
                    break;
                }
                /* incorrect specification, fall through to error */
            default:
                uhoh();
                exit(EXIT_ERR);
            }
        }
    }

    /* determine full name of .newsrc file */
    if (newsrc[0] == '\0') {    /* non-zero if alternate .newsrc specified */
        struct passwd *who;
        if (argc > 1)           /* only argument may be another user name */
            cp = argv[1];
        else
            cp  = getenv("LOGNAME");    /* default - user's login directory */
        who=getpwnam(cp);
        if (who == NULL) {
            (void)fprintf(stderr,"Invalid user name %s\n", cp);
            uhoh();
            exit(EXIT_ERR);
        }
        (void)sprintf(newsrc, "%s/%s", who->pw_dir, NEWSRC);
        if (argc == 2) {
            (void)printf("Peeking at %s\n",newsrc);
            argc--;
        }
    }

    if (argc > 1) {             /* should be no arguments left */
        uhoh();
        exit(EXIT_ERR);
    }

    work = wstart = strspace;   /* point to string space */
    actvlist = grpspace;        /* point to group linklist space */

    /* read and save active file information */
    if (freopen(active,"r",stdin) == NULL) {
        (void)sprintf(strspace, "unable to open %s", active);
        perror(strspace);
        exit(EXIT_ERR);
    }

    (void)fstat(fileno(stdin),&statbuf);    /* when was active file modified */

    (void)printf("active news file last updated on %s",
        asctime(localtime(&statbuf.st_mtime)));

    /* scan active file and save info */
    (void)gets(buf);
    while (!feof(stdin)) {
        (void)strtok(buf," ");                      /* EOS after group name */
        highest = atol(strtok((char *)NULL," "));   /* last article # */
        lowest = atol(strtok((char *)NULL," "));    /* first article # */
        if ((highest > 0)  && (!EQS(buf,"junk")))
            insgrp(buf,lowest,highest);
        (void)gets(buf);
    }

    if (debug)
        dbgrps("Newsgroups at start of scan");

    /* read  users .newsrc info and compare to active */
    if (freopen(newsrc,"r",stdin) == NULL) {
        (void)sprintf(strspace, "unable to open %s", newsrc);
        perror(strspace);
        exit(EXIT_ERR);
    }
    for ((void)gets(buf);
        !feof(stdin);
        (void)gets(buf)) {
        rcnum = mytok(buf,' ');     /* find list of articles read */
        if (rcnum == NULL)          /* and terminate group name */
            rcnum = "0";
        cp = buf + strlen(buf) - 1;
        if (*cp == '!')
            inrc = FALSE;   /* group names must end with 1 and only 1 [:!] */
        else
            inrc = TRUE;
        *cp = EOS;          /* EOS after group name (replacing ':') */
        if (EQS(buf,"junk"))
            continue;
        ingrp = countgrp(buf,rcnum,inrc,showall);   /* count this group */
        if (ingrp) {
            articles += ingrp;
            newsgrps++;
            if (summary)
                break;
        }
    }

    if (articles) {
        exitstat = EXIT_OK;     /* exit status can indicate if there is news */
        if (summary)
            (void)puts("There is news");
        else {
            (void)printf("%ld article%s in %d news group%s %s unread\n",
                articles, PLURAL(articles>1),
                newsgrps,
                PLURAL(newsgrps>1), VERB(articles>1));
        }
    }
    else {
        exitstat = EXIT_NONEWS; /* exit status can indicate if there is news */
        (void)puts("No News is Bad News");
    }

    if (chkgrps)
        whatsleft(newsrc);

    if (debug)
        dbgrps("Newsgroups at end of scan");

    exit(exitstat);
}

/* insert newsgroup in linked list - order is not important */
void
insgrp(name,lowest,highest)
char *name;     /* group name */
long lowest;    /* lowest article number */
long highest;   /* highest article number */
{
    register struct grp *curr,*new; /* used to scan and modify linked list */

    /* insert new group at end of list */
    if (newind == 0)
        curr = &head;
    else
        curr = &actvlist[newind - 1];
    new = &actvlist[newind++];      /* space for new group node, bump index */
    new->name = strcpy(work,name);  /* store name */
    new->lowest = lowest;           /* lowest article number */
    new->highest = highest;         /* and highest article number */
    work += strlen(name) + 1;       /* update pointer space for new names */
    new->next = curr->next;         /* insert node after current one */
    curr->next = new;
    return;
}

findgrp(name,lowest,highest)
char *name;     /* group name */
long *lowest, *highest;
{
    register struct grp *curr,*last; /* used to scan and modify linked list */
    int retval;

    *lowest = *highest = 0;
    if (newind <= 0)
        return(-1);     /* no nodes in list - should never happen */
    /* scan list trying to match newsgroup name */
    for (curr = &head; ; curr = curr->next) {
        if (EQS(name, curr->name)) {
            *lowest = curr->lowest;
            *highest = curr->highest;
            retval = 1;
            break;
        }
        if (curr->next == NULL) {   /* end of list, shouldn't happen */
            retval = 0;
            break;
        }
        last = curr;
    }
    if (retval)     /* name match - remove node - should always happen */
        last->next = curr->next;

    return (retval);
}

void
whatsleft(newsrc)
char *newsrc;
{
    if (head.next == NULL) {    /* no groups remain in list */
        (void)printf("\nAll groups in %s are in %s\n", active, newsrc);
        return;
    }

    (void)printf("\nGroups in %s but not in %s:\n", active, newsrc);
    showgrps();
}

void
dbgrps(title)
char *title;
{
    if (head.next == NULL) {
        (void)fprintf(stderr,"Nothing scanned in %s\n",active);
        exit(EXIT_ERR);
    }

    (void)printf("\%s\n", title);
    showgrps();

    PR1(d,newind);  /* show space used */
    PR2(x,wstart,work);
    DB1(d,work-wstart);
}

void
showgrps()
{
    register struct grp *curr;  /* general group link pointer */

    curr = &head;

    do {
        curr = curr->next;      /* point to next legit group */
        (void)printf("%-35s lowest article : %d highest: %d\n",
            curr->name, curr->lowest, curr->highest);
    } while (curr->next != NULL);
}

/* argument: an optional user name (default is $LOGNAME)
 * options: -s quick summary message: There is or is not news
 *          -a show per group unread message count
 *          -c check for newsgroups in active but not in .newsrc
 *          -x enable debugging output
 *          -f use a file other that $HOME/.newsrc as the .newsrc file,
 *             the filename must be the follow -f, optionally separated
 *             by whitespace
 */
void
uhoh()
{
    (void)fprintf(stderr,
        "usage: %s [-acsx] [-f other_.newsrc_file] [username]\n",
        progname);
}

char *
basename(name)  /* return pointer to filename portion of name */
char name[];
{
    register char *cp=strrchr(name,'/');

    return(cp==NULL ? name : (cp + 1));
}

countgrp(grp,nums,inrc,showall)     /* count this group's articles */
char *grp;                          /* news group */
char *nums;                         /* article numbers already read */
int inrc;                           /* 0 means just remove group from list */
int showall;                        /* display group's statistics */
{
    register long rchigh,rclow;     /* articles read as shown in .newsrc */
    register char *numptr,*endptr,*comma="";
    register int cnt, grpcnt;
    long highest,lowest;            /* articles in news files */

    if (findgrp(grp,&lowest,&highest) <= 0) /* find group in linked list */
        return(0);          /* not found */

    if (inrc == FALSE)      /* not in .newsrc */
        return(0);

    /* process .newsrc line, assuming 'rn' format */
    grpcnt = 0;
    for (numptr=nums; numptr != NULL;) {
        endptr = mytok(numptr,',');     /* number ranges are separated by ',' */
        rchigh = rclow = atol(numptr);  /* may be a single number */
        numptr = mytok(numptr,'-');     /* or it may be a range */
        if (numptr != NULL)
            rchigh = atol(numptr);
        if (rclow > lowest) {   /* true if articles not recorded in .newsrc */
            cnt = rclow - lowest;   /* how many */
            if (showall) {
                if (grpcnt == 0)
                    (void)printf("%-28s unread: ",grp);
                (void)printf("%s%ld",comma,lowest);
                if (cnt > 1)        /* is a range unread */
                    (void)printf("-%ld",rclow-1);
                comma = ",";        /* delimit multiple unread ranges */
            }
            grpcnt += cnt;          /* total count for this newsgroup */
        }
        lowest = rchigh + 1;        /* new 'lowest' unread article number */
        numptr = endptr;            /* check next range */
    }

    if (highest > rchigh) {         /* new unread articles ? */
        cnt = highest - rchigh;
        if (showall) {
            if (grpcnt == 0)
                (void)printf("%-28s unread: ",grp);
            (void)printf("%s%ld",comma,rchigh+1);
            if (cnt > 1)
                (void)printf("-%ld",highest);
        }
        grpcnt += cnt;
    }

    if (grpcnt && showall)
        (void)printf(" (%d)\n", grpcnt);

    return(grpcnt);
}

/* this function is sort of like strtok. It returns a pointer to the
 * next token delimited by 'chr', but it doesn't 'remember' its position
 */
char *
mytok(str,chr)
register char *str;
register char chr;
{
    str = strchr(str,chr);
    if (str == NULL)
        return (str);
    *str++ = EOS;
    while (*str == chr)
        str++;
    return(str);
}
