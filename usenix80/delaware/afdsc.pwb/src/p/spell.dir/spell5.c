#
/* nc -O -s -n -i spell5.c ... -lj */
#include "bbuf.h"
#include <error.h>
#include <site.h>
#define	nextarg		{++argv; --argc;}		/* function to move to next argument */

char *prog_id "~|^' spell  Release 3 Version 0\n";


struct bufr dictbuf;
struct bufr docbuf;
struct tree {
        struct tree *t_rp, *t_lp;
        struct tree *t_group; /* permutations of endings
                                 are grouped together */
        char t_docnum;  /* we allow 255 docs, (that's all that argv allows
                         anyway */
        char t_wierd;
        char t_word[];
};
struct tree *new, *old;

long htbl[26*26];       /* hash table */
long sizeofht sizeof htbl;
char *hname DICTHASH;  /* a var so that it can be moved */
int hfd;

char **argx;    /* copy of argv */
int docnum;     /* current document number */
#define cvc 1
#ifdef debug
#define db(a,b) printf(a,b)
#endif
#ifndef debug
#define db(a,b)
#endif
#define wcntlim 1500 /* depends on how much data space you're processor has */
                        /* should be about 2500-3000 max */
int expandfl 0;   /* =1 for expanded printout */
int thresh 5000;        /* complain if no version less
                           wierd than this defined */

char *dname DICT;     /* dname must be a variable */
struct tree *tp;
char * hinterland;
extern int flagg 2;     /* should be static, but can't init it then */
main (argc, argv)
int argc;
char **argv;
{
        register int a;
        char line[128];

        static int wcnt;

/*        struct tree *tp;*/
        extern int find(), print();
/*        char * hinterland;*/

        nice(20);       /* Yes! */
        tp = 0;


	/* COMMAND LINE ARGUMENT GATHERING SECTION */
	nextarg;			/* skip over the arg with command name (arg 0)*/
	while (argc > 0 && **argv == '-')		/* while there are arguments, and */
							/* they start with a dash */
	{
		for(;;) {				/* until end of string is detected */
			switch (*++*argv)		/* interpret each flag letter */
			{
			case 'x':			/* expanded printout option */
				expandfl = 1;
				break;
			case 't':			/* reset threshold */
				nextarg;
				thresh = num(*argv);
				goto ugh_a_goto;
			case 'd':			/* use an alternate dictionary file */
				nextarg;
				dname = *argv;
				goto ugh_a_goto;
			case 'h':			/* use an altername hash table */
				nextarg;
				hname = *argv;
				goto ugh_a_goto;
			case '\0':			/* end of string, go on to next argument */
				goto ugh_a_goto;
			default:			/* No flags allowed */
				exit(EFLAG);
			}
		}
		ugh_a_goto:
		nextarg;
	}

	/* MAIN BODY OF PROGRAM */

        argx = argv;
        db("dictionary is %s\n", dname);
        db("hash table is %s\n", hname);
        if (openb (dname, &dictbuf, 0) < 0) {
                printb ("No dictionary\n");
                exit(ENOENT);
        }
        hfd = open (hname, 0);
        if (hfd < 0) {
                printb ("No hash table\n");
                exit(ENOENT);
        }
        for(a=3; signal(a,1)>=0; a++);
        setuid(getuid());                       /* no comment */
        hinterland = sbrk(0);
/*%%*/  db("Argc is %d\n", argc);
        a = 0;
        if (argc<1)
        {
                initb(0,&docbuf);
                goto cont;
        }
        for (; a<argc; a++) {
/*%%*/          db("Opening %s\n",argv[a]);
                if (openb (argv[a], &docbuf, 0) < 0) {
                        printb ("No %s\n", argv[a]);
                        exit(ENOENT);
                }
        cont:
                docnum = a;     /* for diagnostics? */
                while (word (line) == 0) {
                        if(wcnt>=wcntlim)
                        {
                                wcnt = 0;
                                doit();
                        }
                        new = insert(line, &tp);
                        if(new)
                        {
/*%%*/                          db("ends called!\n",0);
                                ends (&tp, line);
                        }
                        wcnt++;
                }
/*%%*/          db("Closing %s\n", argv[a]);
                closeb (&docbuf);
        }
        trav (tp, &find);
        close (hfd);
        trav (tp, &print);
}

struct tree *
insert (wrd, tpp)
char *wrd;
struct tree **tpp;
{
        register int i;
        register char *cp;


/*        char * hinterland;*/
        struct tree * tp;

        for(;;)
        {
        if (*tpp == 0) {
                cp = wrd;
                while (*cp++);
        back:
                {
                        i = sbrk(((sizeof **tpp) + cp-wrd + 1 ) & ~1);
                                /* ^ make sure that it is even */
                }
                if (i == -1) {
                        doit();
                        printb ("No more storage\n");
                        tpp = &tp;
                        goto back;
                }
                else
                        *tpp = i;
                (*tpp)->t_docnum = docnum;
                (*tpp)->t_wierd = 0;
                (*tpp)->t_rp = 0;
                (*tpp)->t_lp = 0;
                (*tpp)->t_group = 0;
                move (wrd, (*tpp)->t_word);
                return *tpp;
        }
        i = comp (wrd, (*tpp)->t_word);
        if (i <= 0) {
                if (i == 0 && docnum == (*tpp)->t_docnum) {
                        old = *tpp;
                        return 0;
                }
                tpp = &(*tpp)->t_lp;
                continue;
        }
        tpp = &(*tpp)->t_rp;
        }
}

comp (t1, t2)
char *t1, *t2;
{
        register char *s1, *s2;
        register int onetime;

        s1 = t1; s2 = t2;       /* s1 is in dict and s2 is in doc */
        onetime = 0;
/*%%*/  db("Comparing '%s' with ",s1);
/*%%*/  db("'%s'\n",s2);
    for(;;)
    {
        while (*s1 == *s2 
#ifdef cvc
                || ((*s2|' ')==(*s1|' ')&&(*s1|' ')>='a'&&(*s1|' ')<='z')
#endif
                ) {
                if (*s1++ == 0)
                {
                        return onetime;
                }
/*%%*/          db("    char is '%c'\n", *s2);
                s2++;
        }
        if (*s2 == '\''&&onetime == 0)
        {
                onetime = 1;
                s2++;
        }
        else
        if (*s1 == '\''&&onetime == 0)
        {
                onetime = -1;
                s1++;
        }
        else
                break;
    }
        if (
#ifndef cvc
                *s1 < *s2
#endif
#ifdef cvc
                (*s1|' ') < (*s2|' ')/* maps 07 into 047. C'est la guerre */
#endif
        ) return -1;
        return 1;
}

trav (tp, sub)
struct tree *tp;
int (*sub)();
{
        register struct tree *t;

        for(t = tp; t!=0; t = t->t_rp)
        {
                trav(t->t_lp,sub);
                (*sub)(t);
        }
}

find (tp)
struct tree *tp;
{
/*
if (tp->t_group) printb ("* ");
if (tp->t_wierd) db ("w=%d ",tp->t_wierd);
                db ("%s\n",tp->t_word);
return;
*/
        if (srchdict(tp->t_word)
/*      || (srchaddn(tp->t_word) */
        )
                tp->t_docnum = -1;
        db("%s in docnum ",tp->t_word); db("%d\n",tp->t_docnum);
}

int
word (pp)
char *pp;
{
        static int c;
        extern int flagg;
        register char *p;
        register int dotseen;

        dotseen = 0;
        do
        {
                p = pp;
                while (alph (c) <= 0 || c == '\'')
                {
                        if(c<0)
                        {
                                c = 0; flagg = 2;
                                return -1;
                        }
                        else if(c=='\n') flagg = 1;
                        else if(c=='.'&&flagg) dotseen = 1;
                        else if(c=='\'')
                        {
                                if(!flagg)      break;
                                dotseen = 1;
                        }
                        else if(c==0&&flagg==2);
                                /* ^ f*cking initial condition */
                        else if(c=='\\')                /* nroff escape */
                        {
                                getbc(&docbuf);
                        }
                        else flagg = dotseen = 0;
/*%%*/                  db(" Flushing '%c'\n",c);
                        c = getbc(&docbuf);
                }
                while(*p++ = alph(c))
                {
                        c = getbc(&docbuf);
                        if(c=='\n')
                        {
                                flagg = 1;
                        }
                        else
                                flagg = 0;
                }
/*%%*/          db("Word is %s\n", pp);
        }
        while((dotseen && p-pp == 3)|| p-pp == 1);      /* i.e. a two char nroff command */
                                                /* or a null word */

        if(c<0)
        {
                c = 0;
                flagg = 2;
                return -1;
        }
        else
                return 0;
}

int
dword (p)
char *p;
{
        register int c;

        while ((c = getbc(&dictbuf)) >= 0 && c != '\n') {
/*%%*/          db("Char before alph() is '%c'\n",c);
                if (c=='\b')
                        getbc(&dictbuf);                /* swallow next char */
                else if ((c=alph(c)) > 0)
                {
                        *p++ = c;
/*%%*/                  db("Gathering '%c'\n", c);
                }
        }
        *p++ = '\0';
        if (c < 0) return c;
        return 0;
}


int
alph(c) int c;
{
        register int k;

        k = c;
        if(k>'z')       return 0;
        else if(k>='a') return k;
        else if(k>'Z')  return 0;
        else if(k>='A') return k
#ifndef cvc
                                -'A'+'a'
#endif
                                ;
        else if(k=='\'')return k;       /* was -1 */
                        return 0;
}
/*
int srchaddn (x) {
        return 0;
}
*/

char dwrd[128] "";
int dicteof 0;
int curidx -1;
int chr -1;

int srchdict (w)
char *w;
{
        register int cc;
        register int idx;
        long int schr;
/*%%*/  int ii;
        char *wp;

        if (dicteof) return 0;
        wp = w;
        db ("looking for %s\n",wp);
        schr = hchr(&wp);
        idx = hash(wp);
        if (schr != chr) {
                db ("seeking in hash table %s\n",w);
#               ifdef debug
                {
                        printf("seek is to absolute %D\n", sizeofht*schr);
                }
#               endif
                lseek (hfd, sizeofht * schr, 0);
                chr = schr;
                curidx = -1;
                read (hfd, htbl, sizeof htbl);
        }
        if (idx != curidx) {
                db("hash index is %d\n", idx);
                db("seeking to %D\n", htbl[idx]);
                if (htbl[idx] == -1) return 0;
                curidx = idx;
                seekb (&dictbuf, htbl[idx]);
                dwrd[0] = '\0';
        }
        while ((cc=comp(dwrd,w)) < 0) {
/*%%*/          if (ii=dword(dwrd)) {
                        db("dword(dwrd) is %d\n",ii);
                        dicteof = 1;
                        return 0;
                }
        db ("%s -- dict: ",dwrd);
        }
/*%%*/  db("Cc is %d\n", cc);
#       ifdef debug
        {
                dword(dwrd);
        }
#       endif
/*%%*/  db("Next word in dict is %s\n",dwrd);
        if (cc == 0) return 1;
        return 0;
}

wadd (tpp, w, l)
struct tree **tpp;
char *w;
int l;
{
        register struct tree *t, *t2;

        if (l > thresh) return;
        t = new;
        docnum++;       /* guarantee new entry */
        t2 = insert (w, tpp);
        docnum--;
        new = t;
        t2->t_docnum = docnum;
        t2->t_wierd = l;
        ring (t2, t);
}

ring (t1, tt2)
struct tree *t1, *tt2;
{
        register struct tree *t2;

        t2 = tt2;
        if (t2->t_group == 0) t2->t_group = t2;
        t1->t_group = t2->t_group;
        t2->t_group = t1;
}

print (tp)
struct tree *tp;
{
        register struct tree *t;

        db ("%s tested ",tp->t_word);
        db ("docnum=%d ",tp->t_docnum);
        db ("grp=%d ",tp->t_group); db ("wierd=%d\n",tp->t_wierd);
        if (tp->t_docnum < 0) return;
        if (tp->t_group == 0) goto pr;
        if (tp->t_wierd != 0) return;
        if (expandfl && tp->t_wierd == 0) goto pr;
        t = tp;
        while ((t=t->t_group) != tp) {
                db ("in chain: %s\n",t->t_word);
                if (t->t_docnum < 0) return;
        }

pr:
        printb (docnum>1? "%s from %s\n": "%s\n",
                tp->t_word,
                argx[tp->t_docnum]);
}

num (s)
char *s;
{
        register int i;
        register char *p;

        i = 0;
        p = s;
        while (*p) {
                if (*p <= '9' && *p >= '0') i = i*10 + (*p++-'0');
                else {
                        printb ("illegal number %s\n", s);
                        exit (EFLAG);
                }
        }
        return i;
}

doit()
{
/*        char * hinterland;*/
/*        struct tree *tp;*/
        extern int curidx;

        trav(tp,&find);
        trav(tp,&print);
        brk(hinterland);
        tp = 0;
        curidx = -1;
}
