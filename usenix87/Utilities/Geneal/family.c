/* family.c - produce a family information page */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 08:47:46 by jimmc (Jim McBeath) */
/* last edit 11-Sept-85 22:29:00 by tlr (Terry L. Ridder) */

#include "geneal.h"
#include <ctype.h>
#include "pagemap.h"

#define PGWD 80
#define LTMAR 4
#define RTMAR 4
#define CTRMAR 2
#define LTCOL ((PGWD-LTMAR-RTMAR-CTRMAR)/2)
#define RTCOL (PGWD-LTMAR-RTMAR-CTRMAR-LTCOL)
#define SO 10

#define TBLOCKSIZE 1000
#define NOTEMAX 500

struct tblock {				/* a text block */
    int width;				/* length of longest line */
    int lines;				/* number of lines */
    char *text[TBLOCKSIZE];		/* the text pointers */
    };


int notenum;
int notecount;				/* for storing the footnotes */
char *notes[NOTEMAX];

/*..........*/

int					/* 0 if OK */
family(famnum)
int famnum;				/* the family to give info about */
{
char *rtype;
char *famname, *hisname, *hername;
char *hislname, *herlname;
int hisnum, hernum;
char *headerline;
int i, cnum, *clist;
char *ss;

    notecount = notenum = 0;
    rtype = fgstr(famnum, "T");		/* get record type */
    if (rtype == 0 || *rtype == 0)
    {
	warning("no such record number %d", famnum);
	return 1;
    }
    if (rtype[0] != 'F')
    {
	warning("record %d is not a family", famnum);
	return 1;
    }
    famname  = fgstr(famnum, "N");	/* get the family name */
    hisnum   = fgnum(famnum, "H");	/* get husband and wife index nums */
    hernum   = fgnum(famnum, "W");
    hisname  = bname(hisnum);
    hername  = bname(hernum);
    hislname = fgstr(hisnum, "LN");
    herlname = fgstr(hernum, "LNM");
    if (herlname == 0 || herlname[0] == 0) 
    {
	herlname = fgstr(hernum, "LN");
    }

    if (hislname && hislname[0] == 0) 
    {
	hislname = 0;
    }
    if (herlname && herlname[0] == 0) 
    {
	herlname = 0;
    }
    if (famname && famname[0] == 0) 
    {
	famname = 0;
    }
    if (hisname == 0 || hisname[0] == 0) 
    {
	hisname = "???";
    }
    if (hername == 0 || hername[0] == 0) 
    {
	hername = "???";
    }
    if (famname && (hislname == 0 || strcmp(famname, hislname) != 0))
    {
	headerline = tprintf("%s - %s & %s", famname, hisname, hername);
    }
    else
    {
	headerline = tprintf("%s & %s", hisname, hername);
    }
    for (ss = headerline; *ss; ss++)
    {
	if (islower(*ss)) 
	{
	    *ss = toupper(*ss);		/* convert to upper case */
	
	}
    }
    printf("%*s%s\n\n", LTMAR, " ", headerline);
    printpair(0, famnum, hisnum, hernum);	/* print data about parents */
    cnum = fgclist(famnum, &clist);
    if (cnum == 0)
    {
/* be silent about no children...
	printf("%*s%s\n", LTMAR, " ", "NO CHILDREN");
 */
    }
    else
    {
	printf("%*s%-*s%*s%s\n\n", LTMAR, " ", LTCOL, "CHILDREN",
	    CTRMAR, " ", "SPOUSES OF CHILDREN");
	for (i = 0; i < cnum; i++)
	{
	 int childnum, marnum, sonum, chisnum, chernum;
	 int mnum;
	 char mnumstr[5];
    
	    childnum = clist[i];
	    for (mnum = 0, marnum = 1; marnum > 0; mnum++)
			    /* until we run out of marriages */
	    {
		sprintf(mnumstr,"S%d", mnum);
		marnum = fgnum(childnum, mnumstr);	/* get marriage */
		if (marnum > 0)
		{
		    chisnum = fgnum(marnum, "H");
		    chernum = fgnum(marnum, "W");
		    if (childnum == chisnum) 
		    {
			sonum = chernum;
		    }
		    else if (childnum == chernum) 
		    {
			sonum = chisnum;
		    }
		    else
		    {
			warning(
			    "person %d claims marraige %d, but not vice-versa!",
			    childnum, marnum);
			sonum= -1;
		    }
		}
		else sonum = -1;
		if (mnum == 0 || marnum > 0) 
		{
		    printpair(mnum, marnum, childnum, sonum);
		}
	    }
	}
    }
    if (notecount > 0)		/* if we accumulated any notes */
    {
	printf("%*s%s\n\n", LTMAR, " ", "-----------------------");
	for (i = 0; i < notecount; i++)
	{
	    printf("%*s%s\n", LTMAR, " ",  notes[i]);
	}
    }
    return 0;
}

/*..........*/

printpair(n, mn, cn, sn)	/* print info about a couple */
int n;			/* which marriage in the list this is; -1=only one */
int mn;			/* marriage number */
int cn;			/* primary person number */
int sn;			/* spouse number */
{
struct tblock cntb, sntb;	/* where to store data */
int i, max;

    fampdat(n, mn, cn, &cntb);		/* get the data */
    fampdat(-1, mn, sn, &sntb);
/* decide if they should both be on the same lines or not */
    if (cntb.width > LTCOL || sntb.width > RTCOL) /* separate */
    {
	printtb(&cntb, LTMAR);	/* output the first one */
	printtb(&sntb, LTMAR + SO);	/* output spouse */
    }
    else		/* both on the same line */
    {
	if (cntb.lines > sntb.lines) 
	{
	    max = cntb.lines;
	}
	else 
	{
	    max = sntb.lines;
	}
	for (i = 0; i < max; i++)
	{
	    if (i >= cntb.lines)
	    {
		printf("%-*s%s\n",
		    (LTMAR + CTRMAR + LTCOL), " ", sntb.text[i]);
	    }
	    else if (i >= sntb.lines)
	    {
		printf("%*s%s\n",
		    LTMAR, " ", cntb.text[i]);
	    }
	    else
	    {
		printf("%*s%-*s%*s%s\n", LTMAR, " ",
		    LTCOL, cntb.text[i], CTRMAR, " ", sntb.text[i]);
	    }
	}
	printf("\n");
    }
}

/*..........*/

printtb(b, offset)		/* print a text block */
struct tblock *b;
int offset;			/* left margin offset */
{
int i;

    for (i = 0; i < b->lines; i++)
    {
	printf("%*s%s\n", offset, " ", b->text[i]);
    }
    if (b->lines != 0) 
    {
	printf("\n");
    }
}

/*..........*/

fampdat(i, m, n, b)		/* get a tblock about a person */
int i;			/* iteration number to determine how to format */
int m;			/* marriage number */
int n;			/* the person to get info about */
struct tblock *b;	/* where to put the data */
{
char *name, *birth, *death, *mar;

    b->lines = b->width = 0;	/* clear it out first */
    if (n <= 0) 
    {
	return;
    }
    name = bname(n);
    birth = fgbirth(n);
    death = fgdeath(n);
    mar = fgmar(m);
    if (i <= 0)
    {
	addtline(name, b);
	if (birth && *birth) 
	{
	    addtline(birth, b);
	}
	if (death && *death) 
	{
	    addtline(death, b);
	}
	if (i == 0)
	{
	    if (mar && *mar) 
	    {
		addtline(mar, b);
	    }
	    adnotes(n, m, b);		/* add general comment notes */
	}
	else	/* i== -1 */
	{
	    adnotes(n, -1, b);		/* don't add marriage notes */
	}
    }
    else
    {
	if (mar && *mar) 
	{
	    mar = tprintf("re-%s", mar);
	}
	else 
	{
	    mar = "remarried:";
	}
	addtline(mar, b);
	adnotes(-1, m, b);
    }
}

/*..........*/

adnotes(n, m, b)	/* add general comment notes to a block */
int n;			/* index of person */
int m;			/* index of marriage */
struct tblock *b;	/* text block to add to */
{
    adnote(n, b, " ");
    adnote(m, b, " (m)");
}

/*..........*/

adnote(n, b, ss)	/* add general comment notes to a block */
int n;			/* index of person */
struct tblock *b;	/* text block to add to */
char *ss;		/* note indicator string */
{
int i;
char comnum[10];
char gencom[1000];
char *lnotes[1000];
int lcount;
int hlen;
int t;

    gencom[0] = 0;		/* clear to start */
    lcount = 0;
    hlen = strlen(ss) + sizeof("Note") + 2;
    for (i = 0; i == 0 || gencom[0]; i++)	/* read each comment */
    {
	sprintf(comnum,"GEN%d", i);
	bstr(n, comnum, gencom);	/* read comment line */
	if (gencom[0] == 0) 
	{
	    break;
	}
	lnotes[lcount++] = strsav(gencom);
    }
    if (lcount == 1 && ((t = strlen(lnotes[0]) + hlen) <= b->width || t < LTCOL))
    {		/* if we have one relatively short string, do it in-line */
	addtline(tprintf("[Note%s: %s]", ss, lnotes[0]), b);
    }
    else if (lcount > 0)
    {
	addtline(tprintf("[Note %d%s]", 1 + notenum, ss), b);
	notes[notecount++] = tprintf("Note %d%s:", 1 + notenum++, ss);
	for (i = 0; i < lcount; i++)
	{
	    notes[notecount++] = lnotes[i];	/* add string to notes */
	    if (notecount >= NOTEMAX -1)
	    {
		warning("notelist overflow!");
		notecount--;
	    }
	}
	notes[notecount++] = "";
    }
}

/*..........*/

addtline(ss, b)			/* add a line to a tblock */
char *ss;			/* the string to add */
struct tblock *b;		/* the block to add to */
{
int l;

    if (b->lines >= TBLOCKSIZE)
    {
	warning("tblock overflow!");
	return;			/* ignore the addition */
    }
    b->text[b->lines++] = ss;	/* add in the string */
    l = strlen(ss);
    if (l > b->width) 
    {
	b->width = l;	/* keep track of width */
    }
}

/* end */
