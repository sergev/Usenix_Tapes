static char *sccsid = "@(#)sq.c         1.8u (UCF) 83/09/02 (09/20/84)";
/*
 *   sq.c - CP/M compatible file squeezer utility
 *
 *   compile as follows:
 *   cc [-DVAX] -O sq.c -o sq
 *	 (define VAX only if running on VAX)
 */

#include <stdio.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0
#define ERROR (-1)
#define PATHLEN 312	/* Number of characters allowed in pathname */
#define ALTNAME "sq.out"

/* Definitions and external declarations */

#define RECOGNIZE 0xFF76	/* unlikely pattern */

/* *** Stuff for first translation module *** */

#define DLE 0x90


/* *** Stuff for second translation module *** */

#define SPEOF 256	/* special endfile token */
#define NUMVALS 257	/* 256 data values plus SPEOF*/


#ifdef VAX   /*  we only want 16 bit integers  */

typedef short INT;
typedef unsigned short UNSIGNED;

#else	/*  PDP-11 and other 16-bit machines  */

typedef int INT;
typedef unsigned UNSIGNED;

#endif


/* Definitions and external declarations */

INT Usestd;	/* Use stdout for squeezed output */
UNSIGNED crc;	/* error check code */

/* *** Stuff for first translation module *** */

INT likect;	/*count of consecutive identical chars */
INT lastchar, newchar;
unsigned char state;

/* states */

#define NOHIST	0	/*don't consider previous input*/
#define SENTCHAR 1	/*lastchar set, no lookahead yet */
#define SENDNEWC 2	/*newchar set, previous sequence done */
#define SENDCNT 3	/*newchar set, DLE sent, send count next */

/* *** Stuff for second translation module *** */

#define NOCHILD -1	/* indicates end of path through tree */
#define NUMNODES (NUMVALS + NUMVALS - 1)	/* nbr of nodes */

#define MAXCOUNT (UNSIGNED) 65535	/* biggest UNSIGNED integer */

/* The following array of structures are the nodes of the
 * binary trees. The first NUMVALS nodes becomethe leaves of the
 * final tree and represent the values of the data bytes being
 * encoded and the special endfile, SPEOF.
 * The remaining nodes become the internal nodes of the final tree.
 */

struct	nd {
	UNSIGNED weight;	/* number of appearances */
	INT tdepth;		/* length on longest path in tre*/
	INT lchild, rchild;	/* indexes to next level */
} node[NUMNODES];

INT dctreehd;	/*index to head node of final tree */

/* This is the encoding table:
 * The bit strings have first bit in  low bit.
 * Note that counts were scaled so code fits UNSIGNED integer
 */

INT codelen[NUMVALS];		/* number of bits in code */
UNSIGNED code[NUMVALS]; 	/* code itself, right adjusted */
UNSIGNED tcode; 		/* temporary code value */


/* Variables used by encoding process */

INT curin;	/* Value currently being encoded */
INT cbitsrem;	/* Number of code string bits remaining */
UNSIGNED ccode; /* Current code shifted so next code bit is at right */

/* This program compresses a file without losing information.
 * The usq.com program is required to unsqueeze the file
 * before it can be used.
 *
 * Typical compression rates are:
 *	.COM	6%	(Don't bother)
 *	.ASM	33%	(using full ASCII set)
 *	.DIC	46%	(using only uppercase and a few others)
 * Squeezing a really big file takes a few minutes.
 *
 * Usage:
 *	sq file ...
 *
 *
 * The squeezed file name is formed by changing the second from last
 * letter of the file type to Q. If there is no file type,
 * the squeezed file type is QQQ. If the name exists it is
 * overwritten!
 *
 * The transformations compress strings of identical bytes and
 * then encode each resulting byte value and EOF as bit strings
 * having lengths in inverse proportion to their frequency of
 * occurrence in the intermediate input stream. The latter uses
 * the Huffman algorithm. Decoding information is included in
 * the squeezed file, so squeezing short files or files with
 * uniformly distributed byte values will actually increase size.
 */

/* CHANGE HISTORY:
 * 1.5u **nix version - means output to stdout.
 *  (stdin not allowed becuase sq needs to rewind input, which
 *  won't work with pipes.)
 * Filename generation changed to suit **nix and stdio.
 * 1.6u machine independent output for file compatibility with
 *	original CP/M version SQ, when running on machine with
 *	IBM byte ordering such as Z8000 and 68000.
 * 1.7u machine independence was still lacking for 32-bit machines
 *	like the VAX-11/780, so a typedef was added.  No action
 *	need be taken if running on a 16-bit machine, but if
 *	running on a VAX, define VAX either on the cc line or
 *	in the program preamble.   Ben Goldfarb 12/13/82
 * 1.8u Modified to run under CI-86 compiler for the IBM PC
 *	Robert J. Beilstein 09/02/83
 *	(Note that the program must be compiled as SMALL model.)
 */

#define VERSION "1.8u   09-02-83"
/* #define VERSION "1.7u   12-13-82" */

INT inbackground = 0;  /* change to 1 to suppress informative messages */

INT buildenc(), gethuff(), getc_crc();

main(argc, argv)
INT argc;
char *argv[];
{
    register INT i,c;

#ifdef VAX

    if (signal(SIGINT, SIG_IGN)==SIG_IGN)
	inbackground++;
    else
	signal(SIGINT, SIG_DFL);
    signal(SIGHUP, SIG_IGN);

#endif

    /* Process the parameters in order */
    for(i = 1; i < argc; ++i) {
	if(strcmp(argv[i], "-")==0)
	    Usestd = TRUE;
    }
    for(i = 1; i < argc; ++i) {
	if(strcmp(argv[i], "-")!=0)
	    obey(argv[i]);
    }

    if(argc < 2) {
	fprintf(stderr,"File squeezer version %s by\n\tRichard Greenlaw\n\t251 Colony Ct.\n\tGahanna, Ohio 43230\n", VERSION);
	fprintf(stderr, "Usage: sq [-] pathname ...\n");
	fprintf(stderr, "\t- squeezed output to stdout\n");
	exit(1);
    }
    exit(0);
}
obey(p)
unsigned char *p;
{
    unsigned char *w,*q;
    unsigned char outfile[PATHLEN+2];	     /* output file spec. */

    /* First build output file name */

    strcpy(outfile, p);
    /* Find and change output file type */
    for(w=q = outfile; *q != '\0'; ++q)     /* skip leading /'s */
	if( *q == '/')
	    w = q+1;
    for(q = w; *q != '\0'; ++q)
	if(*q == '.')
	    if(*(q + 1) == '\0')
		*q = '\0';      /* kill trailing dot */
	    else
		switch(*(q+2)) {
		    case 'q':
		    case 'Q':
			    fprintf(stderr, "sq: %s ignored ( already squeezed?)", p);
			    return;
		    case '\0':
			    *(q+3) = '\0';
			    /* fall thru */
		    default:
			    *(q + 2) = 'Q';
			    goto named;
		}
    /* No file type */
    strcat(outfile, ".QQQ");
named:
    if(strlen(w)>14)
	strcpy(outfile, ALTNAME);	/* check for too long name */
    squeeze(p, outfile);
}

squeeze(infile, outfile)
unsigned char *infile, *outfile;
{
    register INT i, c;
    FILE *inbuff, *outbuff; /* file buffers */

    if (!inbackground)
	fprintf(stderr, "\n%s -> %s: ", infile, outfile);

    if((inbuff=fopen(infile, "rb")) == NULL) {
	fprintf(stderr, "sq: can't open %s\n", infile);
	return;
    }
    if(Usestd)
	outbuff=stdout;
    else if((outbuff=fopen(outfile, "wb")) == NULL) {
	fprintf(stderr, "sq: can't create %s\n", outfile);
	fclose(inbuff);
	return;
    }

    /* First pass - get properties of file */
    crc = 0;	    /* initialize checksum */
    if (!inbackground)
	fprintf(stderr, "analyzing, ");
    init_ncr();
    init_huff(inbuff);
    /* rewind(inbuff); */ fseek(inbuff,0L,0);

    /* Write output file header with decoding info */
    wrt_head(outbuff, infile);

    /* Second pass - encode the file */
    if (!inbackground)
	fprintf(stderr, "squeezing, ");

    init_ncr();     /* For second pass */

    /* Translate the input file into the output file */
    while((c = gethuff(inbuff)) != EOF)
	if(putc(c, outbuff) == ERROR && ferror(outbuff)) {
	    fprintf(stderr, "sq: write error\n");
	    goto closeall;
	    }
    if (!inbackground)
	fprintf(stderr, " done.\n");
closeall:
    fclose(inbuff);
closeout:
    fflush(outbuff);
    fclose(outbuff);
}


/* First translation - encoding of repeated characters
 * The code is byte for byte pass through except that
 * DLE is encoded as DLE, zero and repeated byte values
 * are encoded as value, DLE, count for count >= 3.
 */

init_ncr()	/*initialize getcnr() */
{
    state = NOHIST;
}

INT
getcnr(iob)
FILE *iob;
{
    switch(state) {
	case NOHIST:
		/* No relevant history */
		state = SENTCHAR;
		return lastchar = getc_crc(iob);
	case SENTCHAR:
		/* Lastchar is set, need lookahead */
		switch(lastchar) {
		case DLE:
			state = NOHIST;
			return 0;	/* indicates DLE was the data */
		case EOF:
			return EOF;
		default:
			for(likect = 1; (newchar = getc_crc(iob)) == lastchar && likect < 255; ++likect)
				;
			switch(likect) {
			case 1:
				return lastchar = newchar;
			case 2:
				/* just pass through */
				state = SENDNEWC;
				return lastchar;
			default:
				state = SENDCNT;
				return DLE;
			}
		}
	case SENDNEWC:
		/* Previous sequence complete, newchar set */
		state = SENTCHAR;
		return lastchar = newchar;
	case SENDCNT:
		/* Sent DLE for repeat sequence, send count */
		state = SENDNEWC;
		return likect;
	default:
		fprintf(stderr,"sq: Bug - bad state\n");
		exit(1);
		/* NOTREACHED */
	}
}


/******** Second translation - bytes to variable length bit strings *********/


/* This translation uses the Huffman algorithm to develop a
 * binary tree representing the decoding information for
 * a variable length bit string code for each input value.
 * Each string's length is in inverse proportion to its
 * frequency of appearance in the incoming data stream.
 * The encoding table is derived from the decoding table.
 *
 * The range of valid values into the Huffman algorithm are
 * the values of a byte stored in an integer plus the special
 * endfile value chosen to be an adjacent value. Overall, 0-SPEOF.
 *
 * The "node" array of structures contains the nodes of the
 * binary tree. The first NUMVALS nodes are the leaves of the
 * tree and represent the values of the data bytes being
 * encoded and the special endfile, SPEOF.
 * The remaining nodes become the internal nodes of the tree.
 *
 * In the original design it was believed that
 * a Huffman code would fit in the same number of
 * bits that will hold the sum of all the counts.
 * That was disproven by a user's file and was a rare but
 * infamous bug. This version attempts to choose among equally
 * weighted subtrees according to their maximum depths to avoid
 * unnecessarily long codes. In case that is not sufficient
 * to guarantee codes <= 16 bits long, we initially scale
 * the counts so the total fits in an unsigned integer, but
 * if codes longer than 16 bits are generated the counts are
 * rescaled to a lower ceiling and code generation is retried.
 */

/* Initialize the Huffman translation. This requires reading
 * the input file through any preceding translation functions
 * to get the frequency distribution of the various values.
 */

init_huff(ib)
FILE *ib;
{
    register INT c, i;
    INT btlist[NUMVALS];    /* list of intermediate binary trees */
    INT listlen;	    /* length of btlist */
    UNSIGNED *wp;	    /* simplifies weight counting */
    UNSIGNED ceiling;	    /* limit for scaling */

    /* Initialize tree nodes to no weight, no children */
    init_tree();

    /* Build frequency info in tree */
    do {
	c = getcnr(ib);
	if(c == EOF)
	    c = SPEOF;
	if(*(wp = &node[c].weight) !=  MAXCOUNT)
	    ++(*wp);
    } while(c != SPEOF);

    ceiling = MAXCOUNT;

    do {    /* Keep trying to scale and encode */
	if(ceiling != MAXCOUNT)
	    fprintf(stderr, "sq: *** rescaling ***, ");
	scale(ceiling);
	ceiling /= 2;	/* in case we rescale */

	/* Build list of single node binary trees having
			 * leaves for the input values with non-zero counts
			 */
	for(i = listlen = 0; i < NUMVALS; ++i)
	    if(node[i].weight != 0) {
		node[i].tdepth = 0;
		btlist[listlen++] = i;
	    }

	/* Arrange list of trees into a heap with the entry
			 * indexing the node with the least weight at the top.
			 */
	heap(btlist, listlen);

	/* Convert the list of trees to a single decoding tree */
	bld_tree(btlist, listlen);

	/* Initialize the encoding table */
	init_enc();

	/* Try to build encoding table.
			 * Fail if any code is > 16 bits long.
			 */
    } while(buildenc(0, dctreehd) == ERROR);

    /* Initialize encoding variables */
    cbitsrem = 0;   /*force initial read */
    curin = 0;	    /*anything but endfile*/
}

/* The count of number of occurrances of each input value
 * have already been prevented from exceeding MAXCOUNT.
 * Now we must scale them so that their sum doesn't exceed
 * ceiling and yet no non-zero count can become zero.
 * This scaling prevents errors in the weights of the
 * interior nodes of the Huffman tree and also ensures that
 * the codes will fit in an unsigned integer. Rescaling is
 * used if necessary to limit the code length.
 */

scale(ceil)
UNSIGNED ceil;	/* upper limit on total weight */
{
    register INT i,c;
    INT ovflw, divisor;
    UNSIGNED w, sum;
    unsigned char increased;	     /* flag */

    do {
	for(i = sum = ovflw = 0; i < NUMVALS; ++i) {
	    if(node[i].weight > (ceil - sum))
		++ovflw;
	    sum += node[i].weight;
	}

	divisor = ovflw + 1;

	/* Ensure no non-zero values are lost */
	increased = FALSE;
	for(i = 0; i < NUMVALS; ++i) {
	    w = node[i].weight;
	    if (w < divisor && w != 0) {
		/* Don't fail to provide a code if it's used at all */
		node[i].weight = divisor;
		increased = TRUE;
	    }
	}
    } while(increased);

    /* Scaling factor choosen, now scale */
    if(divisor > 1)
	for(i = 0; i < NUMVALS; ++i)
	    node[i].weight /= divisor;
}

/* heap() and adjust() maintain a list of binary trees as a
 * heap with the top indexing the binary tree on the list
 * which has the least weight or, in case of equal weights,
 * least depth in its longest path. The depth part is not
 * strictly necessary, but tends to avoid long codes which
 * might provoke rescaling.
 */

heap(list, length)
INT list[], length;
{
    register INT i;

    for(i = (length - 2) / 2; i >= 0; --i)
	adjust(list, i, length - 1);
}

/* Make a heap from a heap with a new top */

adjust(list, top, bottom)
INT list[], top, bottom;
{
    register INT k, temp;

    k = 2 * top + 1;	    /* left child of top */
    temp = list[top];	    /* remember root node of top tree */
    if( k <= bottom) {
	if( k < bottom && cmptrees(list[k], list[k + 1]))
	    ++k;

	/* k indexes "smaller" child (in heap of trees) of top */
	/* now make top index "smaller" of old top and smallest child */
	if(cmptrees(temp, list[k])) {
	    list[top] = list[k];
	    list[k] = temp;
	    /* Make the changed list a heap */
	    adjust(list, k, bottom); /*recursive*/
	}
    }
}

/* Compare two trees, if a > b return true, else return false
 * note comparison rules in previous comments.
 */

cmptrees(a, b)
INT a, b;	/* root nodes of trees */
{
    if(node[a].weight > node[b].weight)
	return TRUE;
    if(node[a].weight == node[b].weight)
	if(node[a].tdepth > node[b].tdepth)
	    return TRUE;
    return FALSE;
}

/* HUFFMAN ALGORITHM: develops the single element trees
 * into a single binary tree by forming subtrees rooted in
 * interior nodes having weights equal to the sum of weights of all
 * their descendents and having depth counts indicating the
 * depth of their longest paths.
 *
 * When all trees have been formed into a single tree satisfying
 * the heap property (on weight, with depth as a tie breaker)
 * then the binary code assigned to a leaf (value to be encoded)
 * is then the series of left (0) and right (1)
 * paths leading from the root to the leaf.
 * Note that trees are removed from the heaped list by
 * moving the last element over the top element and
 * reheaping the shorter list.
 */

bld_tree(list, len)
INT list[];
INT len;
{
    register INT freenode;	    /* next free node in tree */
    register struct nd *frnp;	    /* free node pointer */
    INT lch, rch;	    /* temporaries for left, right children */
    INT i;

    /* Initialize index to next available (non-leaf) node.
	     * Lower numbered nodes correspond to leaves (data values).
	     */
    freenode = NUMVALS;

    while(len > 1) {
	/* Take from list two btrees with least weight
			 * and build an interior node pointing to them.
			 * This forms a new tree.
			 */
	lch = list[0];	/* This one will be left child */

	/* delete top (least) tree from the list of trees */
	list[0] = list[--len];
	adjust(list, 0, len - 1);

	/* Take new top (least) tree. Reuse list slot later */
	rch = list[0];	/* This one will be right child */

	/* Form new tree from the two least trees using
			 * a free node as root. Put the new tree in the list.
			 */
	frnp = &node[freenode]; /* address of next free node */
	list[0] = freenode++;	/* put at top for now */
	frnp->lchild = lch;
	frnp->rchild = rch;
	frnp->weight = node[lch].weight + node[rch].weight;
	frnp->tdepth = 1 + maxchar(node[lch].tdepth, node[rch].tdepth);
	/* reheap list	to get least tree at top*/
	adjust(list, 0, len - 1);
    }
    dctreehd = list[0];     /*head of final tree */
}

/* ???????????? */
maxchar(a, b)
{
    return a > b ? a : b;
}
/* Initialize all nodes to single element binary trees
 * with zero weight and depth.
 */

init_tree()
{
    register INT i;

    for(i = 0; i < NUMNODES; ++i) {
	node[i].weight = 0;
	node[i].tdepth = 0;
	node[i].lchild = NOCHILD;
	node[i].rchild = NOCHILD;
    }
}

init_enc()
{
    register INT i;

    /* Initialize encoding table */
    for(i = 0; i < NUMVALS; ++i) {
	codelen[i] = 0;
    }
}

/* Recursive routine to walk the indicated subtree and level
 * and maintain the current path code in bstree. When a leaf
 * is found the entire code string and length are put into
 * the encoding table entry for the leaf's data value .
 *
 * Returns ERROR if codes are too long.
 */

INT		/* returns ERROR or NULL */
buildenc(level, root)
INT level;/* level of tree being examined, from zero */
INT root; /* root of subtree is also data value if leaf */
{
    register INT l, r;

    l = node[root].lchild;
    r = node[root].rchild;

    if( l == NOCHILD && r == NOCHILD) {
	/* Leaf. Previous path determines bit string
			 * code of length level (bits 0 to level - 1).
			 * Ensures unused code bits are zero.
			 */
	codelen[root] = level;
	code[root] = tcode & (((UNSIGNED)~0) >> (16 - level));
	return (level > 16) ? ERROR : NULL;
    }
    else {
	if( l != NOCHILD) {
	    /* Clear path bit and continue deeper */
	    tcode &= ~(1 << level);
	    /* NOTE RECURSION */
	    if(buildenc(level + 1, l) == ERROR)
		return ERROR;
	}
	if(r != NOCHILD) {
	    /* Set path bit and continue deeper */
	    tcode |= 1 << level;
	    /* NOTE RECURSION */
	    if(buildenc(level + 1, r) == ERROR)
		return ERROR;
	}
    }
    return NULL;    /* if we got here we're ok so far */
}

/* Write out the header of the compressed file */

wrt_head(ob, infile)
FILE *ob;
unsigned char *infile;	 /* input file name (w/ or w/o drive) */
{
    register INT l,r;
    INT i, k;
    INT numnodes;	    /* nbr of nodes in simplified tree */

    putwe(RECOGNIZE, ob);   /* identifies as compressed */
    putwe(crc, ob);	    /* unsigned sum of original data */

    /* Record the original file name w/o drive */
    if(*(infile + 1) == ':')
	infile += 2;	/* skip drive */

    do {
	putce(*infile, ob);
    } while(*(infile++) != '\0');


    /* Write out a simplified decoding tree. Only the interior
	     * nodes are written. When a child is a leaf index
	     * (representing a data value) it is recoded as
	     * -(index + 1) to distinguish it from interior indexes
	     * which are recoded as positive indexes in the new tree.
	     * Note that this tree will be empty for an empty file.
	     */

    numnodes = dctreehd < NUMVALS ? 0 : dctreehd - (NUMVALS -1);
    putwe(numnodes, ob);

    for(k = 0, i = dctreehd; k < numnodes; ++k, --i) {
	l = node[i].lchild;
	r = node[i].rchild;
	l = l < NUMVALS ? -(l + 1) : dctreehd - l;
	r = r < NUMVALS ? -(r + 1) : dctreehd - r;
	putwe(l, ob);	/* left child */
	putwe(r, ob);	/* right child */
    }
}

/* Get an encoded byte or EOF. Reads from specified stream AS NEEDED.
 *
 * There are two unsynchronized bit-byte relationships here.
 * The input stream bytes are converted to bit strings of
 * various lengths via the static variables named c...
 * These bit strings are concatenated without padding to
 * become the stream of encoded result bytes, which this
 * function returns one at a time. The EOF (end of file) is
 * converted to SPEOF for convenience and encoded like any
 * other input value. True EOF is returned after that.
 *
 * The original gethuff() called a separate function,
 * getbit(), but that more readable version was too slow.
 */

INT		/*  Returns byte values except for EOF */
gethuff(ib)
FILE *ib;
{
    INT rbyte;	    /* Result byte value */
    INT need, take; /* numbers of bits */

    rbyte = 0;
    need = 8;	    /* build one byte per call */

    /* Loop to build a byte of encoded data
	     * Initialization forces read the first time
	     */

loop:
    if(cbitsrem >= need) {
	/* Current code fullfills our needs */
	if(need == 0)
	    return rbyte;
	/* Take what we need */
	rbyte |= ccode << (8 - need);
	/* And leave the rest */
	ccode >>= need;
	cbitsrem -= need;
	return rbyte & 0xff;
	}

	/* We need more than current code */
	if(cbitsrem > 0) {
	    /* Take what there is */
	    rbyte |= ccode << (8 - need);
	    need -= cbitsrem;
	}
	/* No more bits in current code string */
	if(curin == SPEOF) {
	    /* The end of file token has been encoded. If
		     * result byte has data return it and do EOF next time
		     */
	    cbitsrem = 0;

	    /*NOTE: +0 is to fight compiler bug? */
	    return (need == 8) ? EOF : rbyte + 0;
	}

	/* Get an input byte */
	if((curin = getcnr(ib)) == EOF)
	    curin = SPEOF;  /* convenient for encoding */

	/* Get the new byte's code */
	ccode = code[curin];
	cbitsrem = codelen[curin];

	goto loop;
}


/* Get next byte from file and update checksum */

INT
getc_crc(ib)
FILE *ib;
{
    register INT c;

    c = getc(ib);
    if(c != EOF)
	crc += c;	/* checksum */
    return c;
}

/* Output functions with error reporting */

putce(c, iob)
INT c;
FILE *iob;
{
    if(putc(c, iob) == ERROR && ferror(iob)) {
	fprintf(stderr, "sq: write error\n");
	exit(1);
    }
}

/*
 * machine independent put-word that writes low order byte first
 *  (compatible with CP/M original) regardless of host cpu.
 */
putwe(w, iob)
INT w;
FILE *iob;
{
    putc(w, iob);
    putc(w>>8, iob);
    if (ferror(iob)) {
	fprintf(stderr, "sq: write error\n");
	exit(1);
    }
}
