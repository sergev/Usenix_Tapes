/*
 * f=hdiff.c  (In honor of Mr Heckel, the guy who thought up this
 * algorithm).
 *
 * author - dennis bednar  8 22 84
 * Source file comparison program similar to UNIX diff, except this
 * version outputs moved blocks whereas UNIX diff reports it as
 * delete/add blocks.
 *
 * Algorithm from "A Technique for Isolating Differences Between Files"
 * CACM, April 1978, by Paul Heckel.
 * Some ideas for pass 6 were borrowed from "What's The Diff? -- A File
 * Comparator for CP/M", Dr. Dobb's Journal, August 1984, by D.E. Cortesi.
 * The borrowed idea was that when you are at the beginning of two
 * blocks of lines which link with lines in the other file, but don't
 * match, then the smaller ascending block is the "moved block".
 *
 * The pass5a () is coded directly from his his pass5 Pascal procedure.
 * It doesn't work if the old line being looked up in the symbol table
 * isn't unique!!!  You would have to change the format of the linerec
 * record to make it work.  That's why it's commented out.
 *
 * Output

DELETES
-------
old d new 		// Single line delete - Old line number 'old' is
			// deleted after new line numbered 'new'
startold,endold d new	// Block line delete - Old block of lines 'startold'
			// to 'endold' are deleted after new line number 'new'

INSERTS
-------
old a new		// After old line number 'old' is new line number 'new'
old a startnew,endnew	// After old line number 'old', new lines numbered
			// 'startnew' to 'endnew'

CHANGES
-------
old c new		// Change old line numbered 'old' to new line numbered
			// 'new'
old c startnew,endnew	// Change one line to a block of lines.  Old line
			// numbered 'old' becomes the new set of lines.
startold,endold c new	//  Change a block of old lines to one new line.
startold,endold c startnew,endnew	// Change a block of lines to
			// a different block of lines.

MOVES
-----
old m new		// Old line number 'old' is moved to new line
			// number 'new'
startold,endold m startnew,endnew  // The old block of lines have been moved
			// and the old line numbers have changed.


For DELETES, INSERTS, and CHANGES (but not MOVES) the old line and new lines
are displayed as follows:
< old line
> new line

 */

#include <stdio.h>
#include "stripnl.h"

/* choose SKIPCOUNT and BIGPRIME as follows:
 * BIGPRIME > 2*MAXLINES in case both files have all unique lines
 * such that SKIPCOUNT*tablesize probes will hit every slot in hash tbl
 * (hint: use the hashtbl a.out to help you)
 */
#define MAXLINES 5000		/* max lines per file that can handle	*/
#define BIGPRIME 10001		/* ideally a prime number > 2*MAXLINES for hash into symtbl */
#define _MAXLINE1 MAXLINES+2	/* two extra for first, last sentinel	*/
#define LINESIZE 1024		/* max chars per line			*/
#define _LINESIZE LINESIZE+2	/* two extra for '\n', '\0' fgets rtns	*/
#define SKIPCOUNT	4	/* reprobe into hash symbol table	*/
#define CHANGESEP	"---\n"	/* line separator for change block	*/
#define ZAPFLAG	1		/* 1 = yes, remove leading white space	*/




/* structure for old, new files.  One structure per line.
 * An array oa for the old file, and an array na for the new file.
 * If the line 'oldi' in the old file has NOT been matched to a line in
 * the new file then oa[oldi].flag == L_SYMINDX.
 * Similarly if the line 'newi' in the new file has NOT been matched to
 * a line in the old file, then na[newi].flag == L_SYMINDX.
 * When line 'oldi' in the old file has been matched to line 'newi'
 * in the new file, then oa[oldi].flag == L_LINENUM,
 * na[newi].flag == L_LINENUM, and each line points to the other, e.g.,
 * oa[oldi].lineloc == newi, and na[newi].lineloc == oldi.
 */
struct linerec {
	int lineloc;		/* line number in other file or symtbl index */
	int flag;		/* tells which as defined below		*/
	};
#define L_LINENUM 0
#define L_SYMINDX 1



/* symbol table structure.  One per unique line in both files.		*/
/* the line is keyed into the array via a hash code			*/
struct symrec {
	char	*stashline;	/* saved line in malloc'ed memory	*/
	int	ocount;		/* count of lines in old file, 0,1,many	*/
	int	ncount;		/* count of lines in new file, 0,1,many	*/
	int	olinenum;	/* line number in the old file		*/
	};




/*globals */
char	*cmd,			/* name of this command			*/
	errbuf [_LINESIZE],	/* build err msg for perror		*/
	linebuf [_LINESIZE],	/* read line from either file into here	*/
	runflag [26],		/* array of options flags set		*/
	*oldfile,		/* name of old file			*/
	*newfile;		/* name of new file			*/

FILE	*oldfp,			/* stream for old file			*/
	*newfp;			/* stream for new file			*/

struct linerec
	oa [_MAXLINE1],		/* for old file				*/
	na [_MAXLINE1];		/* for new file				*/

struct symrec
	symtbl [BIGPRIME];	/* symbol table of all lines seen	*/

int	lastnew,		/* total lines read from new file	*/
	lastold,		/* total lines in old file		*/
	numsymbols,		/* number of entries in symbol table	*/
	debug;			/* debug flag set by -d flag		*/




/* external non-integer functions */
FILE	*fopen();		/* fopen (3)			*/
char	*malloc ();		/* malloc (3)			*/
char	*remwhite ();		/* remove white space from string */

/* internal non-integer functions */
unsigned int hashline ();


main (argc, argv)
	int argc;
	char **argv;

{
	int	usage;		/* TRUE iff there is a usage error */

	/* name of command */
	cmd = argv [0];

	/* check arguments and assign global file names */
	usage = 0;		/* assume no usage error */
	debug = 0;		/* assume no debugging wanted */
	if (argc == 3)
		{
		oldfile = argv [1];
		newfile = argv [2];
		}
	else if (argc == 4 && *argv[1] == '-')
		{
		register char *cp;
		for (cp = argv[1]+1; *cp; ++cp)
			if ('a' <= *cp && *cp <= 'z')
				switch (*cp) {
				case 'v':	/* verbose - debug */
				case 'd':	/* drop */
				case 'm':	/* monotonical by 1 */
				case 'c':	/* count */
				case 'w':	/* white space */
					if (*cp == 'v')
						debug = 1;
					runflag [*cp - 'a'] = 1;
					break;
				default:
					usage = 1;
					break;
				}
			else
				usage = 1;
		oldfile = argv[2];
		newfile = argv[3];
		}
	else
		usage = 1;

	if (usage)
		{
		fprintf (stderr, "usage: %s [-cdmvw] oldfile newfile\n", cmd);
		exit (1);
		}


	/* open both files */
	openfiles ();

	initvars ();

	src_compare ();

	/* close both files */
	closefiles ();

}

initvars ()
{
	numsymbols = 0;
}



/*
 * compare the 2 files in 6 easy passes
 */

src_compare ()
{
	pass1 ();	/* read, store new file				*/
	pass2 ();	/* read, store old file				*/
	pass3 ();	/* match up lines which occur only once		*/
	pass4 ();	/* apply rule 2 working toward end		*/
	pass5 ();	/* apply rule 2 working toward beginning	*/
/*	pass5a ();	/* convert block-moves to insert/deletes	*/
/*	dumparray ();	/* see if internal tables are built correctly	*/
	pass6 ();	/* print out differences			*/
	if (debug)
		printf ("debug: symbol table: occupied = %d, total = %d\n",
		numsymbols, BIGPRIME);
}

/*
 * pass 1
 * read in new file.
 */

pass1 ()
{
	int
		linenum,	/* which line we are on			*/
		stat,		/* result from stripnl			*/
		sindex;		/* index into symbol table		*/
	char	*cp;		/* char pointer into line		*/



	/* read each line of new file in a loop.		*/
	/* stop if eof or can't handle that many lines		*/
	for (linenum = 0;
		fgets (linebuf, sizeof(linebuf), newfp) != (char *)NULL &&
		++linenum <= MAXLINES; )
		{

		/* strip newline at end and make sure line wasn't too long */
		stat = stripnl (linebuf, sizeof(linebuf) );

		if (stat == L_SUCCESS)
			;
		else if (stat == L_BADFORM)
			fprintf (stderr, "%s: Warning, line %d in file %s not terminated with newline\n",
			cmd, linenum, newfile);
		else
			{
			fprintf (stderr, "%s: line %d longer than %d chars in file %s\n",
			cmd, linenum, LINESIZE, newfile);
			exit (1);
			}

		/* if compressing white space, then compress into line buf */
		if (runflag ['w' - 'a'] )
			{
			char *cp;
			cp = remwhite (linebuf, ZAPFLAG);
			strcpy (linebuf, cp);
			}

		sindex = addsymbol (linebuf);	/* put line into symbol tbl */
		if (symtbl [sindex].ncount < 2)
			++symtbl [sindex].ncount;
		na [linenum].lineloc = sindex;
		na [linenum].flag = L_SYMINDX;
		}

	/* see if new file was empty */
	if (linenum == 0)
		{
		fprintf (stderr, "%s:  New file %s is empty.  Diff is delete entire old file.\n", newfile, cmd);
		exit (1);
		}

	if (linenum > MAXLINES)
		{
		fprintf (stderr, "%s:  New file %s is too big.  Last line read was %d.\n", cmd, newfile, MAXLINES);
		exit (1);
		}

	/* assign global number of new lines */
	lastnew = linenum;
}


/*
 * pass 2
 * read in old file.
 */

pass2 ()
{
	int
		linenum,	/* which linenum we are on		*/
		stat,		/* result from stripnl			*/
		sindex;		/* index into symbol table		*/
	char	*cp;		/* char pointer into line		*/



	/* read each line of old file in a loop.		*/
	/* stop if eof or can't handle that many lines		*/
	for (linenum = 0;
		fgets (linebuf, sizeof(linebuf), oldfp) != (char *)NULL &&
		++linenum <= MAXLINES; )
		{

#if 0		/* old way */
		/* strip newline at end and make sure line wasn't too long */
		cp = &linebuf[strlen(linebuf)-1];
		if (*cp == '\n')
			*cp = '\0';
		else if (strlen(linebuf) < sizeof(linebuf)-1)
			fprintf (stderr, "%s: Warning, line %d in file %s not terminated with newline\n", cmd, linenum, oldfile);
		else
			{
			fprintf (stderr, "%s: line %d longer than %d chars in file %s\n",
				cmd, linenum, LINESIZE, oldfile);
			exit (1);
			}
#endif

		/* strip newline at end and make sure line wasn't too long */
		stat = stripnl (linebuf, sizeof(linebuf) );

		if (stat == L_SUCCESS)
			;
		else if (stat == L_BADFORM)
			fprintf (stderr, "%s: Warning, line %d in file %s not terminated with newline\n",
			cmd, linenum, oldfile);
		else
			{
			fprintf (stderr, "%s: line %d longer than %d chars in file %s\n",
			cmd, linenum, LINESIZE, oldfile);
			exit (1);
			}


		/* if compressing white space, then compress into line buf */
		if (runflag ['w' - 'a'] )
			{
			char *cp;
			cp = remwhite (linebuf, ZAPFLAG);
			strcpy (linebuf, cp);
			}

		sindex = addsymbol (linebuf);	/* put line into symbol tbl */
		if (symtbl [sindex].ocount < 2)
			++symtbl [sindex].ocount;
		symtbl[sindex].olinenum = linenum;
		oa [linenum].lineloc = sindex;
		oa [linenum].flag = L_SYMINDX;
		}

	/* see if old file was empty */
	if (linenum == 0)
		{
		fprintf (stderr, "%s:  Old file %s is empty.  Diff is add entire new file.\n", oldfile, cmd);
		exit (1);
		}

	if (linenum > MAXLINES)
		{
		fprintf (stderr, "%s:  Old file %s is too big.  Last line read was %d.\n", cmd, oldfile, MAXLINES);
		exit (1);
		}

	/* assign number of lines in old file */
	lastold = linenum;
}


/*
 * Add a line to the symbol table.
 * The hash function determines the initial probe into the symbol table.
 * If the line is new, then core is malloc'ed for storage, and
 * the 'value' is the pointer to the line (empty slot can be found).
 * If the line is old, the old index is returned.
 *
 * returns
 *	index into symbol table
 *	updated global 'numsymbols' for debugging.
 *
 */
addsymbol (line)
	char *line;
{
	int	sindex,		/* symbol table index			*/
		firstprobe;	/* index of first probe into symbol tbl	*/
	char	*cp;		/* malloc char pointer			*/

	sindex = firstprobe = hashline (line, BIGPRIME);

	/* keep looping until empty slot found or table is full.	*/
	/* table cannot be full.					*/
	while (1)
		{
		/* if find an empty slot, add the line there.		*/
		if (symtbl [sindex].stashline == (char *)NULL)
			{
			cp = malloc (strlen (linebuf) + 1);
			if (cp == (char *)NULL)
				{
				fprintf (stderr, "%s: out of space in function addsymbol\n", cmd);
				exit (1);
				}

			/* copy the line to storage area		*/
			strcpy (cp, line);

			/* save the line in the symbol table entry	*/
			symtbl [sindex].stashline = cp;

			++numsymbols;

			break;
			}

		/* if the line is already in the table, then done */
		else if (strcmp (symtbl [sindex].stashline, line) == 0)
			break;

		/* else it's a collision with a different line, so
		 * let's do linear open addressing.  That is, reprobe
		 * SKIPCOUNT slots below, modulo the table size.
		 * SKIPCOUNT == 1 means primaray clustering.
		 * SKIPCOUNT > 1 means secondary clustering.
		 * Check for overflow.
		 */
		else
			{
			sindex += SKIPCOUNT;
			if (sindex >= BIGPRIME)
				sindex = 0;		/* wraparound */
			if (sindex == firstprobe)
				{
				fprintf (stderr, "%s: symtbl overflow!\n", cmd);
				exit (1);
				}
			}
		}


	return sindex;
}


/*
 * pass 3.
 * Process NA array and link lines up which appear only once
 * in both files.
 * Link virtual line pointer at front and end of both files.
 */

pass3 ()
{
	int	newi,		/* loop counter thru na array		*/
		oldi,		/* index into oa array			*/
		symi;		/* index into symbol table array	*/

	/* loop thru all new lines.  Those that occur once are linked to
	 * each other instead of to the symbol table as they used to be.
	 */
	for (newi = 1; newi <= lastnew; ++newi)
		{
		symi = na[newi].lineloc;	/* get symbol tbl index	*/
		if ((symtbl[symi].ocount == 1) && (symtbl[symi].ncount == 1))
			{
			oldi = symtbl[symi].olinenum;
			linkup (oldi, newi);
			}
		}

	/* link the virtual line 'begin' of each file to each other */
	linkup (0, 0);

	/* line the virtual line 'end' of each file to each other */
	linkup (lastold+1, lastnew+1);
}


/*
 * pass 4
 * loop ascending thru new lines in na array.
 * If line newi in new file is linked to line oldi in old file,
 * but the next lines of each contain the same symbol table
 * pointer, then link na[newi+1] to oa[oldi+1].
 */
pass4 ()
{
	int	newi,
		oldi;

	/* process 'begin', all lines in new file, but NOT virtual 'end' */
	for (newi = 0; newi <= lastnew; ++newi)
		{
		if (na[newi].flag == L_LINENUM)
			{
			oldi = na[newi].lineloc;
			if ((na[newi+1].flag == L_SYMINDX) &&
			    (oa[oldi+1].flag == L_SYMINDX) &&
			    (na[newi+1].lineloc == oa[oldi+1].lineloc))
				linkup (oldi+1, newi+1);
			}
		}
}


/*
 * pass 5
 * Process new file in descending order.
 * Similar to pass 4, but in reverse order.
 */
pass5 ()
{
	int	newi,
		oldi;

	/* process 'end', all lines in new file, but NOT virtual 'begin' */
	for (newi = lastnew+1; newi > 0; --newi)
		{
		if (na[newi].flag == L_LINENUM)
			{
			oldi = na[newi].lineloc;
			if ((na[newi-1].flag == L_SYMINDX) &&
			    (oa[oldi-1].flag == L_SYMINDX) &&
			    (na[newi-1].lineloc == oa[oldi-1].lineloc))
				linkup (oldi-1, newi-1);
			}
		}


}


/*
 * pass 6
 * output the differences.
 */
pass6 ()
{
	int	oldi,		/* current line number in old file	*/
		newi,		/* current line number in new file	*/
		oldmatch,	/* true iff old line matches SOME line in new file */
		newmatch,	/* true iff new line matches SOME line in old file */
		omatcount,	/* match count in old file		*/
		nmatcount,	/* match count in new file		*/
		odrop,		/* old file "relative drop" of new line */
		ndrop,		/* new file "relative drop" of old line */
		ocomp,		/* comparison result for old file	*/
		ncomp,		/* comparison result for new file	*/
		ospan,		/* size of old monotonical block	*/
		nspan;		/* size of new monotonical block	*/

	for (oldi = 0, newi = 0; oldi <= lastold+1 && newi <= lastnew+1;)
		{

		/* set flags to indicate if each line is linked
		 * to another line in the other file.
		 */

		/* is old line linked to another line in the new file? */
		oldmatch = (oa[oldi].flag == L_LINENUM);

		/* is new line linked to another line in the old file? */
		newmatch = (na[newi].flag == L_LINENUM);


		/* if old line moved from old file up (toward begin) in
		 * new file, then we have already processed the block
		 * move in the new file.  Just skip the line in the old file.
		 */
		if (oa[oldi].lineloc < newi && oldmatch)
			{
			++oldi;
			continue;
			}

		/* if old line moved from old file down (toward end) in
		 * old file, then we we have already processed the block
		 * move in the old file.  Just skip the line in the new file.
		 */
		else if (na[newi].lineloc < oldi && newmatch)
			{
			++newi;
			continue;
			}


		/* there are four combinations of booleans oldmatch and newmatch */

		/* if both lines are linked to SOME line in the other file */
		if (oldmatch && newmatch)
			{
			/* if both lines match each other, then go
			 * to next line in both files
			 */
			if (oa[oldi].lineloc == newi)
				{
				if (debug)
					printf ("debug: Same old line %d and new line %d\n", oldi, newi);
				++oldi;
				++newi;
				continue;
				}

			/* blocks not linked to each other.
			 * If there are more lines in oldfile that
			 * are monotonically increasing by one in
			 * the new file, then this is normal, and the
			 * new block is moved down in the old file.
			 * If there are more lines in newfile that
			 * are monotonically increasing by one in
			 * the old file, then this is normal, and
			 * the old block is moved down in the new file.
			 */

			/* get size of old block which is monitonically
			 * increasing by one in the new file.
			 */
			ospan = oldmon (oldi);

			/* get size of new block which is monitonically
			 * increasing by one in the old file.
			 */
			nspan = newmon (newi);

			/* count the number of lines in the old file between
			 * the current line and the line which corresponds
			 * to the first new moved line.
			 */
			omatcount = gomatch (oldi, na[newi].lineloc);

			/* count the number of lines in the new file between
			 * the current line and the old moved line which
			 * match.
			 */
			nmatcount = gnmatch (newi, oa[oldi].lineloc);

			/* get number of old lines the new block drops down
			 * into the old file.  If old drop is < new drop
			 * then the old block has moved down further into
			 * the new file than the new block has moved down
			 * into the old file.  The relative steepness of the
			 * slope of the line from oldi to its matched line
			 * is more than the steepnes of the other slope,
			 * and since line don't usually move far, assume
			 * that the new to old move is a match, and the
			 * old to new is an old block moved down.
			 */
			odrop = na [newi].lineloc - oldi;

			/* same for new file */
			ndrop = oa [oldi].lineloc - newi;

			/* if 'd' flag is set, use 'drop' for match */
			if (runflag ['d' - 'a'])
				{
				ocomp = odrop;
				ncomp = ndrop;
				}
			/* if 'm' flag set, use bigger monotonically increasing
			 * by 1 span
			 */
			else if (runflag ['m' - 'a'])
				{
				ocomp = ospan;
				ncomp = nspan;
				}
			/* if 'c' flag then use match count */
			else if (runflag ['c' - 'a'])
				{
				ocomp = omatcount;
				ncomp = nmatcount;
				}
			/* default - best one */
			else
				{
				ocomp = omatcount;
				ncomp = nmatcount;
				}



			if (ocomp < ncomp)		/* old block moved down */
				eatold (&oldi, ospan);	/* eat old mv'ed blk */
			else				/* new block moved down */
				eatnew (&newi, nspan);	/* eat new mv'ed blk */
			}

		/* new lines added (inserted) into new file */
		else if (oldmatch && !newmatch)
			newinsert (oldi, &newi); /* skip new insert blk */

		/* old lines deleted from old file */
		else if (!oldmatch && newmatch)
			olddelete (&oldi, newi);	/* skip old delete block */

		/* old lines changed into new file */
		else	/* !oldmatch && !newmatch */
			oldchange (&oldi, &newi); /* skip old delete block and new insert block */
		}
}

/*
 * block of old lines beginning at linenum 'oldi' are matched with
 * a block of new lines.  Count the number of lines in the old block
 * which are monotonically increasing by one in the new file.
 * returns -
 *	size of old block
 */
oldmon (oldi)
	int	oldi;
{
	int	osize,		/* size of old block			*/
		expnew,		/* line number expected in new file	*/
		curoldnum;	/* current line number in old file	*/


	curoldnum = oldi;	/* save old line number so can tell
				 * how big the old block is at the end
				 */

	do
		{
		expnew = oa[curoldnum].lineloc + 1;
		++curoldnum;
		}
	while ((curoldnum <= lastold+1)			/* in bounds */
		&& (expnew == oa[curoldnum].lineloc)	/* monotonical by 1 */
		&& (oa[curoldnum].flag == L_LINENUM));	/* match */

	osize = curoldnum - oldi;

	if (debug)
		printf ("debug: lines %d-%d of old file are monotonical\n", oldi, curoldnum-1);

	return osize;
}


newmon (newi)
	int	newi;
{
	int	nsize,		/* size of new block			*/
		expold,		/* line number expected in old file	*/
		curnewnum;	/* current line number in new file	*/


	curnewnum = newi;	/* save new line number so can tell
				 * how big the new block is at the end
				 */

	do
		{
		expold = na[curnewnum].lineloc + 1;
		++curnewnum;
		}
	while ((curnewnum <= lastnew+1)			/* in bounds */
		&& (expold == na[curnewnum].lineloc)	/* monotonical by 1 */
		&& (na[curnewnum].flag == L_LINENUM));	/* match */

	nsize = curnewnum - newi;

	if (debug)
		printf ("debug: lines %d-%d of new file are monotonical\n", newi, curnewnum-1);

	return nsize;
}


/*
 * eat old block beginning with old line number *'oldi' and 'ospan' lines
 * These old lines are moved down into the new file.
 * The output format is
5,6m7,8		// old lines 5-6 are moved to new lines 7-8
5m7		// old line 5 is moved to new line 7
 * returns
 *	*oldi = updated old line number
 */
eatold (oldi, ospan)
	int	*oldi,
		ospan;
{
	int	newi;

	/* get starting line number of new block */
	newi = oa[*oldi].lineloc;

	/* how many lines in the old block - one or more? */
	if (ospan == 1)
		printf ("%dm%d\n", *oldi, newi);
	else if (ospan > 1)
		printf ("%d,%dm%d,%d\n", *oldi, *oldi+ospan-1, newi, newi+ospan-1);
	else
		{
		fprintf (stderr, "%s: unexpected negative ospan = %d in function eatold\n", ospan);
		exit (1);
		}

	/* return old line number after the old moved block */
	*oldi += ospan;

}


/*
 * eat the new block
 * Like eatold, but works on the new block.
 */

eatnew (newi, nspan)
	int	*newi,
		nspan;
{
	int	oldi;

	/* get starting line number in the old file */
	oldi = na[*newi].lineloc;

	if (nspan == 1)
		printf ("%dm%d\n", oldi, *newi);
	else if (nspan > 1)
		printf ("%d,%dm%d,%d\n", oldi, oldi+nspan-1, *newi, *newi+nspan-1);
	else
		{
		fprintf (stderr, "%s: unexpected negative nspan = %d in function eatnew\n", nspan);
		exit (1);
		}

	*newi += nspan;
}

/*
 * Print new lines inserted in old file to transform into new file
 * input
 *	oldi - one after the current old line number !!!!!!!
 *	*newi - first line number in new file of inserted line.
 * output
 *	*newi - next line number in new file after inserted block.
 *	printed lines of the form
 *

5a7		// after old line 5 add one newline as newline 7
> new line 8

 * or

5a7,8		// after old line 5 add two newlines as newline 7 and 8
> new line 7
> new line 8

 */
newinsert (oldi, newi)
	int	oldi,
		*newi;
{
	int	nsize,		/* number of lines inserted in new file	*/
		csize,		/* current number of lines in a loop	*/
		curnewline;	/* current line number in new file	*/

	if (debug)
		printf ("debug: Found New Insert.  Old line %d.  New line %d.\n", oldi, *newi);

	/* first compute size so we know which format to use (single line
	 * insert or multiline insert)
	 * Get the number of lines in the insert block.
	 */
	nsize = inssize (*newi);

	if (nsize == 1)
		printf ("%da%d\n", oldi-1, *newi);
	else if (nsize > 1)
		printf ("%da%d,%d\n", oldi-1, *newi, *newi+nsize-1);
	else
		{
		fprintf (stderr, "%s: unexpected new block size = %d in function newinsert\n", nsize);
		exit (1);
		}

	/* print the new inserted lines */
	for (curnewline = *newi, csize = nsize; csize > 0; ++curnewline, --csize)
		printf ("> %s\n", symtbl [ na[curnewline].lineloc ].stashline);

	/* return the new line */
	*newi += nsize;
}




/*
 * Print that a group of lines in the old file was deleted
 * input -
 *	*oldi - line number in old file where the group of lines begins
 *	newi - line number + 1 in new file after which the delete occurs.
 * output
 *	*oldi - updated with the next old line after the delete block
5d7		// old line 5 deleted after new line 7
< old line 5
5,6d7		// old lines 5 and 6 deleted after new line 7
< old line 5
< old line 6
 *
 */

olddelete (oldi, newi)
	int	*oldi,
		newi;
{
	int	osize,		/* number of lines deleted in old file	*/
		csize,		/* current size in in the loop		*/
		curoldline;	/* current line number in old file	*/

	if (debug)
		printf ("debug: Found Old Delete.  Old line %d.  New line %d.\n", *oldi, newi);

	/* first compute size so we know which format to use (single line
	 * insert or multiline delete)
	 * Get the number of lines in the old delete block.
	 */
	osize = delsize (*oldi);


	/* print the header for the line deletes */
	if (osize == 1)
		printf ("%dd%d\n", *oldi, newi-1);
	else if (osize > 1)
		printf ("%d,%dd%d\n", *oldi, *oldi+osize-1, newi-1);
	else
		{
		fprintf (stderr, "%s: unexpected old block size = %d in function olddelete\n", osize);
		exit (1);
		}

	/* print the old deleted lines */
	for (curoldline = *oldi, csize = osize; csize > 0; ++curoldline, --csize)
		printf ("< %s\n", symtbl [ oa[curoldline].lineloc ].stashline);

	/* return the old line after the delete block */
	*oldi += osize;
}




/*
 * Print that a group of old lines has been updated to a group of new lines.
 * input
 *	*oldi = line number of start of old lines
 *	*newi = line number of start of new lines
 * output
 *	*oldi = line number after block deleted in the old file
 *	*newi = line number after block inserted in the new file
 *	print statements according to one of 4 forms:
startold c startnew
startold c startnew,endnew
startold,endold c startnew
startold,endold c startnew,endnew
 * Examples
5c7		// old line 5 has been replace by new line 7
< old line 5
---
> new line 7

5,6c7,8		// old lines 5 and 6 have been replaced by new lines 7 and 8
< old line 5
< old line 6
---
> new line 7
> new line 8
 */

oldchange (oldi, newi)
	int	*oldi,
		*newi;
{
	int	curold,		/* current line number in old file	*/
		curnew,		/* current line number in new file	*/
		osize,		/* size of block changed in old file	*/
		nsize,		/* size of block changed in new file	*/
		csize,		/* current block size for the loop	*/
		curoldline,	/* current line number in old file	*/
		curnewline;	/* current line number in new file	*/

	if (debug)
		printf ("debug: Found Changed Lines.  Old line %d.  New line %d\n", *oldi, *newi);

	/* get the size of the old deleted block and the new inserted block */
	osize = delsize (*oldi);
	nsize = inssize (*newi);

	/* print the header for the old deleted block */
	if (osize == 1)
		printf ("%dc", *oldi);
	else if (osize > 1)
		printf ("%d,%dc", *oldi, (*oldi)+osize-1);
	else
		{
		fprintf (stderr, "%s: unexpected old block size = %d in function oldchange\n", osize);
		exit (1);
		}

	/* print the header for the new inserted block */
	if (nsize == 1)
		printf ("%d\n", *newi);
	else if (nsize > 1)
		printf ("%d,%d\n", *newi, (*newi)+nsize-1);
	else
		{
		fprintf (stderr, "%s: unexpected new block size = %d in function oldchange\n", nsize);
		exit (1);
		}

	/* Now print the old changed (delete) lines */
	for (curoldline = *oldi, csize = osize; csize > 0; ++curoldline, --csize)
		printf ("< %s\n", symtbl [ oa[curoldline].lineloc ].stashline);

	/* print line change separator */
	printf (CHANGESEP);

	/* Now print the new changed (inserted) lines */
	for (curnewline = *newi, csize = nsize; csize > 0; ++curnewline, --csize)
		printf ("> %s\n", symtbl [ na[curnewline].lineloc ].stashline);

	/* return the new old and new lines */
	*oldi += osize;
	*newi += nsize;
}



/*
 * these next two routines are called upon by functions
 *	olddelete, newinsert, and oldchange
 */

/*
 * starting with line number 'newi' in the new file, count the number of
 * lines in the insert block.
 *
 * Loop thru the insert block.  The block is a series of new lines
 * that aren't linked to lines in the old file.  That is, each
 * inserted line points to a symbol table entry.
 */
inssize (newi)
	int	newi;
{
	register
	int	curnewline,		/* current line number in new file	*/
		nsize;			/* number of new lines in insert block	*/


	curnewline = newi;
	while (curnewline <= lastnew+1 && na[curnewline].flag == L_SYMINDX)
		++curnewline;

	/* curnewline is now the new line number AFTER the insert block */

	/* compute the number of lines in the insert block */
	nsize = curnewline - newi;

	if (debug)
		printf ("debug: at new line %d, the insert block has %d lines\n", newi, nsize);

	return nsize;
}



/*
 * compute the number of lines in the old delete block beginning
 * with old line number 'oldi' and return it.
 *
 * Loop thru the delete block.  The block is a series of old lines
 * that aren't linked to lines in the new file.  That is, each
 * deleted line points to a symbol table entry.
 */

delsize (oldi)
	int oldi;
{
	register
	int	curoldline,	/* current line number in old file	*/
		osize;		/* number of old lines in delete block	*/

	curoldline = oldi;
	while (curoldline <= lastold+1 && oa[curoldline].flag == L_SYMINDX)
		++curoldline;

	/* curoldline is now the old line number AFTER the delete block */

	/* compute the number of lines in the delete block */
	osize = curoldline - oldi;

	if (debug)
		printf ("debug: at old line %d, the delete block has %d lines\n", oldi, osize);

	/* return the delete block size */
	return osize;
}






/*
 * dump the oa and na internal arrays which is the crudest way
 * to print the file comparisons.
 * This routine may be called in lieu of pass 6 as a debugging tool.
 */
dumparray ()
{
	int	oldi,
		newi;

	for (oldi = 0; oldi <= lastold+1; ++oldi)
		if (oa[oldi].flag == L_LINENUM)
			printf ("oa[%d] = %d\n", oldi, oa[oldi].lineloc);
		else
			printf ("oa[%d] INSERTED.\n", oldi);


	for (newi = 0; newi <= lastnew+1; ++newi)
		if (na[newi].flag == L_LINENUM)
			printf ("na[%d] = %d\n", newi, na[newi].lineloc);
		else
			printf ("na[%d] DELETED.\n", newi);
}



/* borrowed from hashname.c
 * eventually will be removed.
 */

unsigned int hashline (name,modval)

	register char *name;
	register unsigned int modval;

{
	register unsigned int i;

	i=0;
	while (*name != '\0'){
		i=((i<<2)+(*name&~040))%modval;

		name++;
		}

	return(i);
}



/*
 * linkup line oldi in old file to line newi' in new file
 */
linkup (oldi, newi)
	int	oldi,
		newi;
{
	oa[oldi].lineloc = newi;
	oa[oldi].flag = L_LINENUM;
	na[newi].lineloc = oldi;
	na[newi].flag = L_LINENUM;
}


/*
 * pass 5a converts block moves to delete/insert pairs
 */
pass5a ()
{
	int	oldi,
		newi;

	for (oldi = 1, newi = 1; oldi <= lastold+1; )
		{
		while (oa[oldi].flag == L_SYMINDX && oldi <= lastold+1)
			++oldi;		/* skip deletes in old file	*/
		while (na[newi].flag == L_SYMINDX && newi <= lastnew+1)
			++newi;		/* skip inserts in new file	*/
		if (oldi > lastold+1)
			break;
		if (newi > lastnew+1)
			break;

		if (oa[oldi].lineloc == newi)	/* begin matching lines */
			{
			oldi++;
			newi++;
			}
		else				/* discontinuity ?*/
			{
			if (oa[oldi].lineloc != lastnew+1)
				resolve (oldi, newi);	/* yes */
			else
				;			/* no, sentinel */
			}
		}
}


resolve (oldi, newi)
	int	oldi,
		newi;

{
	int	xo, xn;
	int	t, ospan, nspan;
	int	symi;

	/* measure block starting at oa[oldi] */
	xo = oldi;
	do
		{
		t = 1 + oa[xo].lineloc;
		xo++;
		}
	while (t != oa[xo].lineloc);
	ospan = xo - oldi;

	/* measure block starting at na[newi] */
	xn = newi;
	do
		{
		t = 1 + na[xn].lineloc;
		xn++;
		}
	while (t != na[xn].lineloc);
	nspan = xn - newi;



	if (ospan < nspan)
		{
		xo = oldi;
		xn = oa[oldi].lineloc;
		t = ospan;
		oldi = oldi + ospan;
		}
	else
		{
		xn = newi;
		xo = na[newi].lineloc;
		t = nspan;
		newi = newi + nspan;
		}

	while (t > 0)
		{
		symi = 0;
		while (symi < BIGPRIME && symtbl[symi].olinenum != xo)
			symi++;

		if (symi >= BIGPRIME)
			{
			fprintf (stderr, "%s: can't find old line %d in symtbl\n", cmd, xo);
			dumpsym ();
			exit (1);
			}


		/* link the lines to the same symbol tbl entry */
		oa[xo].flag = L_SYMINDX;
		oa[xo].lineloc = symi;
		na[xn].flag = L_SYMINDX;
		na[xn].lineloc = symi;

		++xo;
		++xn;
		--t;
		}
}

/*
 * open global files 'oldfile' and 'newfile'
 */
openfiles ()
{
	/* open old source file for reading */
	oldfp = fopen (oldfile, "r");
	if (oldfp == (FILE *)NULL)
		{
		sprintf (errbuf, "%s: can't oldfile open %s", cmd, oldfile);
		perror (errbuf);
		exit (1);
		}




	/* open new source file for reading */
	newfp = fopen (newfile, "r");
	if (newfp == (FILE *)NULL)
		{
		sprintf (errbuf, "%s: can't open newfile %s", cmd, newfile);
		perror (errbuf);
		exit (2);
		}
}


/*
 * close both files
 */
closefiles ()
{
	fclose (oldfp);
	fclose (newfp);
}


/*
 * dump contents of symbol table
 * For now, only the old line number and the line of text.
 */
dumpsym ()
{
	register int i;

	printf ("Symbol Table Contents:\n");
	printf ("------ ----- --------\n");
	printf ("old_line_num: <line of text>\n");
	for (i = 0; i < BIGPRIME; ++i)
		if (symtbl[i].olinenum != 0)
			printf ("%d: %s\n", symtbl[i].olinenum, symtbl[i].stashline);
}


/*
 * get number of old lines which match new lines.
 *
 * return the number of lines number n in the old file such that
 * startline <= n < endline which are linked to lines in the new
 * file, and are strictly monotonically increasing in the new file (doesn't
 * have to be monotonically increasing by one, but it can't be the same).
 * Once the monotonical increase stops, we stop counting.
 * 'startline' is definitely matched to some line in new file.
 */

gomatch (startline, endline)
	int	startline,
		endline;

{
	register int	count,	/* lines counted in old file		*/
			newi,	/* current new line number last matched	*/
			oldi;	/* current old line number		*/


	/* stop counting lines if reach endline or find non-monitonical
	 * increase in the new file.  newi set to zero is virtual line begin.
	 */
	for (oldi = startline, count = 0, newi = 0;
		oldi < endline && oldi <= lastold+1; ++oldi)
		if (oa[oldi].flag != L_LINENUM)		/* old line doesn't match any new line */
			continue;			/* so skip old line */
		else if (oa[oldi].lineloc > newi)	/* monotonical? */
			{
			newi = oa[oldi].lineloc;	/* yes, save new line number */
			++count;
			}
		else
			break;				/* no, stop counting */

	return count;
}


/*
 * same idea as gomatch, except count lines in new file which match lines in
 * old file and old file lines are strictly monotonically increasing
 */
gnmatch (startline, endline)
	int	startline,
		endline;

{
	register int	count,
			oldi,
			newi;


	/* stop counting lines if reach endline or find non-monitonical
	 * increase in the old file.  oldi set to zero is virtual line begin.
	 */
	for (newi = startline, count = 0, oldi = 0;
		newi < endline && newi <= lastnew+1; ++newi)
		if (na[newi].flag != L_LINENUM)		/* new line doesn't match any new line */
			continue;			/* so skip new line */
		else if (na[newi].lineloc > oldi)	/* monotonical? */
			{
			oldi = na[newi].lineloc;	/* yes, save old line number */
			++count;			/* bump count */
			}
		else
			break;				/* no, stop counting */

	return count;
}
