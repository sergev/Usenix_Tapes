/* des: duplicate the NBS Data Encryption Standard in software.
 * usage: des <file>
 * prompts for the password
 * If the filename ends in ".n" it will be decrypted with the key;
 * otherwise it will be encrypted.
 *
 * Permutation algorithm:
 *	The permutation is defined by its effect on each of the 16 nibbles
 *	of the 64-bit input.  For each nibble we give an 8-byte bit array
 *	that has the bits in the input nibble distributed correctly.  The
 *	complete permutation involves ORing the 16 sets of 8 bytes designated
 *	by the 16 input nibbles.  Uses 16*16*8 = 2K bytes of storage for
 *	each 64-bit permutation.  32-bit permutations (P) and expansion (E)
 *	are done similarly, but using bytes instead of nibbles.
 *	Should be able to use long ints, adding the masks, at a
 *	later pass.  Tradeoff: can speed 64-bit perms up at cost of slowing 
 *	down expansion or contraction operations by using 8K tables here and
 *	decreasing the size of the other tables.
 * The compressions are pre-computed in 12-bit chunks, combining 2 of the
 *	6->4 bit compressions.
 * The key schedule is also precomputed.
 * Compile with VALIDATE defined to run the NBS validation suite.
 *
 * Jim Gillogly, May 1977
 * Modified 8/84 by Jim Gillogly and Lauren Weinstein to compile with
 *   post-1977 C compilers and systems
 *
 * This program is now officially in the public domain, and is available for
 * any non-profit use as long as the authorship line is retained.
 */

/*#define VALIDATE	*/	/* define to check the NBS validation suite */
/*#define DEBUG 	*/
/*#define LATTICE	*/	/* define for Lattice C on IBM PC */

#include <stdio.h>

#ifndef LATTICE
#include <sgtty.h>
#include <signal.h>
#include <sys/types.h>  /* for local timer */
#include <sys/timeb.h>  /* ditto */

struct sgttyb ttybuf;			/* for gtty/stty 		*/
int bye();				/* for caught interrupts 	*/

#endif

char iperm[16][16][8],fperm[16][16][8]; /* inital and final permutations*/
char s[4][4096];			/* S1 thru S8 precomputed	*/
char p32[4][256][4];			/* for permuting 32-bit f output*/
char kn[16][6];				/* key selections		*/

endes(inblock,outblock)			/* encrypt 64-bit inblock	*/
char *inblock, *outblock;
{	char iters[17][8];		/* workspace for each iteration */
	char swap[8];			/* place to interchange L and R */
	register int i;
	register char *s, *t;

	permute(inblock,iperm,iters[0]);/* apply initial permutation	*/
	for (i=0; i<16; i++)		/* 16 churning operations	*/
		iter(i,iters[i],iters[i+1]);
					/* don't re-copy to save space  */
	s = swap; t = &iters[16][4];	/* interchange left		*/
	*s++ = *t++; *s++ = *t++; *s++ = *t++; *s++ = *t++;
	t = &iters[16][0];		/* and right			*/
	*s++ = *t++; *s++ = *t++; *s++ = *t++; *s++ = *t++;
	permute(swap,fperm,outblock);   /* apply final permutation	*/
}

dedes(inblock,outblock)			/* decrypt 64-bit inblock	*/
char *inblock,*outblock;
{	char iters[17][8];		/* workspace for each iteration */
	char swap[8];			/* place to interchange L and R */
	register int i;
	register char *s, *t;

	permute(inblock,iperm,iters[0]);/* apply initial permutation	*/
	for (i=0; i<16; i++)		/* 16 churning operations	*/
		iter(15-i,iters[i],iters[i+1]);
					/* reverse order from encrypting*/
	s = swap; t = &iters[16][4];	/* interchange left		*/
	*s++ = *t++; *s++ = *t++; *s++ = *t++; *s++ = *t++;
	t = &iters[16][0];		/* and right			*/
	*s++ = *t++; *s++ = *t++; *s++ = *t++; *s++ = *t++;
	permute(swap,fperm,outblock);   /* apply final permutation	*/
}

permute(inblock,perm,outblock)		/* permute inblock with perm	*/
char *inblock, *outblock;		/* result into outblock,64 bits */
char perm[16][16][8];			/* 2K bytes defining perm.	*/
{	register int i,j;
	register char *ib, *ob;		/* ptr to input or output block */
	register char *p, *q;

	for (i=0, ob = outblock; i<8; i++)
		*ob++ = 0;		/* clear output block		*/
	ib = inblock;
	for (j = 0; j < 16; j += 2, ib++) /* for each input nibble	*/
	{	ob = outblock;
		p = perm[j][(*ib >> 4) & 017];
		q = perm[j + 1][*ib & 017];
		for (i = 0; i < 8; i++)   /* and each output byte	*/
			*ob++ |= *p++ | *q++;   /* OR the masks together*/
	}
}

char ip[]				/* initial permutation P	*/
= {	58, 50, 42, 34, 26, 18, 10,  2,
	60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6,
	64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1,
	59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5,
	63, 55, 47, 39, 31, 23, 15,  7	};

char fp[]				/* final permutation F	  */
= {	40,  8, 48, 16, 56, 24, 64, 32,
	39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30,
	37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28,
	35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26,
	33,  1, 41,  9, 49, 17, 57, 25	};

/* expansion operation matrix   */	/* rwo: unused	*/
/* char ei[] = {	32,  1,  2,  3,  4,  5,
	 4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32,  1  };	*/

char pc1[]				/* permuted choice table (key)  */
= {	57, 49, 41, 33, 25, 17,  9,
	 1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27,
	19, 11,  3, 60, 52, 44, 36,

	63, 55, 47, 39, 31, 23, 15,
	 7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29,
	21, 13,  5, 28, 20, 12,  4	};

char totrot[]			   /* number left rotations of pc1 */
= {	1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28	};

char pc1m[56];			  /* place to modify pc1 into	*/
char pcr[56];			   /* place to rotate pc1 into	*/

char pc2[]				/* permuted choice key (table)  */
= {	14, 17, 11, 24,  1,  5,
	 3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8,
	16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32	};

char si[8][64]			  /* 48->32 bit compression tables*/
= {					/* S[1]			 */
	14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
					/* S[2]			 */
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
					/* S[3]			 */
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
					/* S[4]			 */
	 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
					/* S[5]			 */
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
					/* S[6]			 */
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
					/* S[7]			 */
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
					/* S[8]			 */
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11	};

char p32i[]				/* 32-bit permutation function  */
= {	16,  7, 20, 21,
	29, 12, 28, 17,
	 1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
	32, 27,  3,  9,
	19, 13, 30,  6,
	22, 11,  4, 25	};

desinit(key)				/* initialize all des arrays	*/
char *key;
{
#ifdef DEBUG
/*deb*/ printf("Initial perm init.\n");
#endif
	perminit(iperm,ip);		/* initial permutation		*/
#ifdef DEBUG
/*deb*/ printf("Final perm init.\n");
#endif
	perminit(fperm,fp);		/* final permutation		*/
#ifdef DEBUG
/*deb*/ printf("Key sched init.\n");
#endif
	kinit(key);			/* key schedule			*/
#ifdef DEBUG
/*deb*/ printf("Compression init.\n");
#endif
	sinit();			/* compression functions	*/

#ifdef DEBUG
/*deb*/ printf("32-bit perm init.\n");
#endif
	p32init();			/* 32-bit permutation in f	*/
#ifdef DEBUG
/*deb*/ printf("End init.\n");
#endif
}

int bytebit[]			   /* bit 0 is left-most in byte	*/
	= {	0200,0100,040,020,010,04,02,01 };

int nibblebit[] = { 010,04,02,01 };

sinit()				 /* initialize s1-s8 arrays		*/
{	register int i,j;

	for (i=0; i<4; i++)		/* each 12-bit position		*/
		for (j=0; j<4096; j++)  /* each possible 12-bit value   */
			s[i][j]=(getcomp(i*2,j>>6)<<4) |
				(017&getcomp(i*2+1,j&077));
					/* store 2 compressions per char*/
}

getcomp(k,v)				/* 1 compression value for sinit*/
int k,v;
{	register int i,j;		/* correspond to i and j in FIPS*/

	i=((v&040)>>4)|(v&1);		/* first and last bits make row */
	j=(v&037)>>1;			/* middle 4 bits are column	*/
	return (int) si[k][(i<<4)+j];   /* result is ith row, jth col   */
}

kinit(key)				/* initialize key schedule array*/
char *key;				/* 64 bits (will use only 56)   */
{	register int i,j,l;
	int m;

	for (j=0; j<56; j++)		/* convert pc1 to bits of key   */
	{	l=pc1[j]-1;		/* integer bit location		*/
		m = l & 07;		/* find bit			*/
		pc1m[j]=(key[l>>3] &	/* find which key byte l is in  */
			bytebit[m])	/* and which bit of that byte   */
			? 1 : 0;	/* and store 1-bit result	*/
	}
	for (i=0; i<16; i++)		/* for each key sched section   */
		for (j=0; j<6; j++)	/* and each byte of the kn	*/
			kn[i][j]=0;	/* clear it for accumulation	*/
	for (i=0; i<16; i++)		/* key chunk for each iteration */
	{	for (j=0; j<56; j++)	/* rotate pc1 the right amount  */
		pcr[j] = pc1m[(l=j+totrot[i])<(j<28? 28 : 56) ? l: l-28];
			/* rotate left and right halves independently   */
		for (j=0; j<48; j++)	/* select bits individually	*/
		if (pcr[pc2[j]-1])	/* check bit that goes to kn[j] */
			{	l= j & 07;
				kn[i][j>>3] |= bytebit[l];
			}		/* mask it in if it's there	*/
	}
}

p32init()				/* initialize 32-bit permutation*/
{	register int l, j, k;
	int i,m;

	for (i=0; i<4; i++)		/* each input byte position	*/
		for (j=0; j<256; j++)	/* all possible input bytes	*/
		for (k=0; k<4; k++)	/* each byte of the mask	*/
			p32[i][j][k]=0;	/* clear permutation array	*/
	for (i=0; i<4; i++)		/* each input byte position	*/
		for (j=0; j<256; j++)	/* each possible input byte	*/
		for (k=0; k<32; k++)	/* each output bit position	*/
		{   l=p32i[k]-1;	/* invert this bit (0-31)	*/
			if ((l>>3)!=i)	/* does it come from input posn?*/
			continue;	/* if not, bit k is 0		*/
			if (!(j&bytebit[l&07]))
			continue;	/* any such bit in input?	*/
			m = k & 07;	 /* which bit is it?		*/
			p32[i][j][k>>3] |= bytebit[m];
		}
}

perminit(perm,p)			/* initialize a perm array	*/
char perm[16][16][8];			/* 64-bit, either init or final */
char p[64];
{	register int l, j, k;
	int i,m;

	for (i=0; i<16; i++)		/* each input nibble position   */
		for (j=0; j<16; j++)	/* all possible input nibbles   */
		for (k=0; k<8; k++)	/* each byte of the mask	*/
			perm[i][j][k]=0;/* clear permutation array	*/
	for (i=0; i<16; i++)		/* each input nibble position   */
		for (j = 0; j < 16; j++)/* each possible input nibble   */
		for (k = 0; k < 64; k++)/* each output bit position	*/
		{   l = p[k] - 1;	/* where does this bit come from*/
			if ((l >> 2) != i)  /* does it come from input posn?*/
			continue;	/* if not, bit k is 0		*/
			if (!(j & nibblebit[l & 3]))
			continue;	/* any such bit in input?	*/
			m = k & 07;	/* which bit is this in the byte*/
			perm[i][j][k>>3] |= bytebit[m];
		}
}

iter(num,inblock,outblock)		/* 1 churning operation		*/
int num;				/* i.e. the num-th one		*/
char *inblock, *outblock;		/* 64 bits each			*/
{	char fret[4];			/* return from f(R[i-1],key)	*/
	register char *ib, *ob, *fb;
/*	register int i;	*/	/* rwo: unused	*/

	ob = outblock; ib = &inblock[4];
	f(ib, num, fret);		/* the primary transformation   */
	*ob++ = *ib++;			/* L[i] = R[i-1]		*/
	*ob++ = *ib++;
	*ob++ = *ib++;
	*ob++ = *ib++;
	ib = inblock; fb = fret;	/* R[i]=L[i] XOR f(R[i-1],key)  */
	*ob++ = *ib++ ^ *fb++;
	*ob++ = *ib++ ^ *fb++;
	*ob++ = *ib++ ^ *fb++;
	*ob++ = *ib++ ^ *fb++;
}

f(right,num,fret)			/* critical cryptographic trans */
char *right, *fret;			/* 32 bits each			*/
int num;				/* index number of this iter	*/
{	register char *kb, *rb, *bb;	/* ptr to key selection &c	*/
	char bigright[6];		/* right expanded to 48 bits	*/
	char result[6];			/* expand(R) XOR keyselect[num] */
	char preout[4];			/* result of 32-bit permutation */

	kb = kn[num];			/* fast version of iteration	*/
	bb = bigright;
	rb = result;
	expand(right,bb);		/* expand to 48 bits		*/
	*rb++ = *bb++ ^ *kb++;		/* expanded R XOR chunk of key  */
	*rb++ = *bb++ ^ *kb++;
	*rb++ = *bb++ ^ *kb++;
	*rb++ = *bb++ ^ *kb++;
	*rb++ = *bb++ ^ *kb++;
	*rb++ = *bb++ ^ *kb++;
	contract(result,preout);	/* use S fns to get 32 bits	*/
	perm32(preout,fret);		/* and do final 32-bit perm	*/
}

perm32(inblock,outblock)		/* 32-bit permutation at end	*/
char *inblock,*outblock;		/* of the f crypto function	*/
{	register int j;
/*	register int i;	*/	/* rwo: unused	*/
	register char *ib, *ob;
	register char *q;

	ob = outblock;			/* clear output block		*/
	*ob++ = 0; *ob++ = 0; *ob++ = 0; *ob++ = 0;
	ib=inblock;			/* ptr to 1st byte of input	*/
	for (j=0; j<4; j++, ib++)	/* for each input byte		*/
	{	q = p32[j][*ib & 0377];
		ob = outblock;		/* and each output byte		*/
		*ob++ |= *q++;		/* OR the 16 masks together	*/
		*ob++ |= *q++;
		*ob++ |= *q++;
		*ob++ |= *q++;
	}
}

expand(right,bigright)			/* 32 to 48 bits with E oper	*/
char *right,*bigright;			/* right is 32, bigright 48	*/
{
	register char *bb, *r, r0, r1, r2, r3;

	bb = bigright;
	r = right; r0 = *r++; r1 = *r++; r2 = *r++; r3 = *r++;
	*bb++ = ((r3 & 0001) << 7) |	/* 32				*/
		((r0 & 0370) >> 1) |	/* 1 2 3 4 5			*/
		((r0 & 0030) >> 3);	/* 4 5				*/
	*bb++ = ((r0 & 0007) << 5) |	/* 6 7 8			*/
		((r1 & 0200) >> 3) |	/* 9				*/
		((r0 & 0001) << 3) |	/* 8				*/
		((r1 & 0340) >> 5);	/* 9 10 11			*/
	*bb++ = ((r1 & 0030) << 3) |	/* 12 13			*/
		((r1 & 0037) << 1) |	/* 12 13 14 15 16		*/
		((r2 & 0200) >> 7);	/* 17				*/
	*bb++ = ((r1 & 0001) << 7) |	/* 16				*/
		((r2 & 0370) >> 1) |	/* 17 18 19 20 21		*/
		((r2 & 0030) >> 3);	/* 20 21			*/
	*bb++ = ((r2 & 0007) << 5) |	/* 22 23 24			*/
		((r3 & 0200) >> 3) |	/* 25				*/
		((r2 & 0001) << 3) |	/* 24				*/
		((r3 & 0340) >> 5);	/* 25 26 27			*/
	*bb++ = ((r3 & 0030) << 3) |	/* 28 29			*/
		((r3 & 0037) << 1) |	/* 28 29 30 31 32		*/
		((r0 & 0200) >> 7);	/* 1				*/
}

contract(in48,out32)			/* contract f from 48 to 32 bits*/
char *in48,*out32;			/* using 12-bit pieces into bytes */
{	register char *c;
	register char *i;
	register int i0, i1, i2, i3, i4, i5;

	i = in48;
	i0 = *i++; i1 = *i++; i2 = *i++; i3 = *i++; i4 = *i++; i5 = *i++;
	c = out32;			/* do output a byte at a time   */
	*c++ = s[0][07777 & ((i0 << 4) | ((i1 >> 4) & 017  ))];
	*c++ = s[1][07777 & ((i1 << 8) | ( i2	& 0377 ))];
	*c++ = s[2][07777 & ((i3 << 4) | ((i4 >> 4) & 017  ))];
	*c++ = s[3][07777 & ((i4 << 8) | ( i5	& 0377 ))];
}

/* End of DES algorithm (except for calling desinit below)	*/

#ifndef VALIDATE
char *inname, *outname;
FILE *infile, *outfile;

int encrypting;
char buf[512];
char keyx[9], keyy[9];

char *malloc(), *strcpy(), *strcat();

main(argc, argv)
int argc; char *argv[];
{	register char *u;
	char *filename;

	if (argc < 2)			/* filenames given? */
	{  fprintf(stderr, "Usage: des file ...\n");
	   exit(1); 	
	}

	for (++argv; --argc; ++argv)
	{	inname = *argv;
		outname = filename = malloc((unsigned) strlen(inname) + 3);
		strcpy(filename, inname);
		u = &filename[strlen(filename) - 2]; /* check last 2 chars */

		encrypting = (strcmp(".n", u) != 0);
		if (!encrypting) *u = 0; /* strip .n from output filename */
		else strcat(filename, ".n");  /* or add .n to output file */

		if ((infile = fopen(inname, "rb")) == NULL)
		{	fprintf(stderr,"Can't read %s.\n", inname);
			exit(1);
		}
		if ((outfile = fopen(outname, "rb")) != NULL)
		{	fprintf(stderr, "%s would be overwritten.\n",outname);
			exit(1);
		}
		if ((outfile = fopen(outname, "wb")) == NULL)
		{	fprintf(stderr,"Can't write %s.\n", outname);
			exit(1);
		}

		key_get("Type password for ");
		for (;;)
		{	strcpy(keyx, keyy);
			key_get("Verify password for ");
			if (strcmp(keyx, keyy) == 0) break;
		}
		desinit(keyx);	  /* set up tables for DES	*/

		if (pfile() == 0) unlink(inname);
		else	fprintf(stderr,
			   "%s: I/O Error -- File unchanged\n", inname);

		fclose(outfile);
		fclose(infile);
	}
	exit(0);
}

key_get(mes)			/* get file key */
char *mes;
{	register int i, j;
	char linebuf[256];
	int count;

	for (i=0; i<14; i++) keyy[i]=0;

#ifdef LATTICE
#else
	gtty(0, &ttybuf);
	ttybuf.sg_flags &= ~ECHO;  /* turn off echoing */
	signal(SIGINT, bye);	/* catch ints */
	stty(0, &ttybuf);
#endif

	printf("%s%s: ", mes, inname);
	fflush(stdout);

	count = read(0, linebuf, 256);  /* read input line */
	printf("\n");

#ifndef LATTICE
	ttybuf.sg_flags |= ECHO;  	/* restore echo */
	stty(0, &ttybuf);
#endif

	linebuf[count] = 0;  /* null terminate */
	if (linebuf[count-1] == '\n')  /* ignore any terminating newline */
	{  linebuf[count-1] = 0;
	   count--; 	
	}
	if (count > 8) count = 8;	/* only use 8 chars */
	for (i = j = 0; count--;)
	   keyy[i++] = linebuf[j++];
}

pfile()				 /* process the file		*/
{	register int m, nsave;
	register char *b;
	int j;

	while (m = fread(buf, 1, 512, infile))
	{
		if ((nsave = m) < 0)	/* read error			*/
		return(-1);
		for (b=buf; m>0;	/* encrypt/decrypt 1 buffer-full*/
		m -= 8, b += 8)		/* 8-byte blocks		*/
		{   if (encrypting)
		{   if (m<8)		/* don't have a full 64 bits	*/
			{   for (j=0; j<8-m; j++)
				b[m+j]=garbage(); /* fill block with trash  */
			nsave += 8-m;   /* complete the block		*/
			}
			else j=0;	/* number of nulls in last block*/
			endes(b,b);	/* don't need diff input, output*/
		}
		else			/* decrypting			*/
		{   if (m < 8) deout(b, 1); /* last byte in file: count */
			else
			{   dedes(b, b); /* decrypt and output block	*/
			deout(b, 0);
			}
		}
		}
		if (encrypting) if (fwrite(buf, 1, nsave, outfile) != nsave)
			return(-1);
	}
	/* have now encrypted/decrypted the whole file;
	 * need to append the byte count for the last block if encrypting.
	 */
	if (encrypting) fputc(8 - j, outfile);  /* how many good bytes? */
	return(0);
}

int outcount = 0;			/* see when caught up with delay*/

deout(block,flag)			/* 1-block delay on output	*/
char *block,flag;			/* 64-bit block, last block flag*/
{	static char last[8];		/* previous input block		*/
	register int i;
/*	register char *c,*j;	*/	/* rwo: unused	*/

	if (flag)			/* output the last few bytes	*/
	{
		fwrite(last, 1, block[0] & 0377, outfile);
		return;
	}
	if (outcount++)			/* seen any blocks before?	*/
		fwrite(last, 1, 8, outfile);
	for (i = 0; i < 8; i++) last[i] = block[i]; /* copy the block   */
}

garbage()				/* generate garbage for filling */
/* This garbage should be as random as possible.  We're using subsequent calls
 * on the timer, but ideally each byte should be uncorrelated.  Preferable
 * would be to call the timer once and use it to initialize a dumb random
 * number generator.
 */
{
#ifdef LATTICE
	long timer(), ltime;

	ltime = timer();
	return (int) ltime & 0377;
#else
	struct timeb tp;

	ftime(&tp);			/* get current time		*/
	return tp.millitm;		/* return time in milliseconds  */
#endif
}

#ifndef LATTICE

/* restore echo to tty and exit */
bye()
{
	ttybuf.sg_flags |= ECHO;  /* restore echoing */
	stty(0, &ttybuf);
	exit(2);
}

#endif

#else	   /* validation */

#define VALFILE "valid.triples"

FILE *fd;

char key[8], plain[8], cipher[8], processed[8];

main()  /* read key/plain/cipher triples until exhausted */
{	int count, i;

	if ((fd = fopen(VALFILE, "r")) == NULL)
	{	fprintf(stderr, "Can't read %s.\n", VALFILE);
		exit(1);
	}
	count = 0;
	desinit(key);		/* initialize most of the arrays */
	while (readvals())
	{	kinit(key);	/* initialize key stuff		*/
		printf("Key: "); writehex(key);
		printf("  Plain: "); writehex(plain);
		printf("  Cipher: "); writehex(cipher);
		printf("\n");
		endes(plain, processed); /* encipher the plaintext */
		printf("Encry:  "); writehex(processed);
		printf("\n");
		for (i = 0; i < 8; i++)
			if (processed[i] != cipher[i])
				printf("Encryption failed.\n");
		dedes(cipher, processed); /* decipher the ciphertext */
		printf("Decry:  "); writehex(processed);
		printf("\n");
		for (i = 0; i < 8; i++)
			if (processed[i] != plain[i])
				printf("Decryption failed.\n");
		count++;
	}
	printf("Processed %d tests.\n", count);
}

readvals()	/* get the next legit triple */
{	int r;

	r = readhex(key);
	readhex(plain);
	readhex(cipher);
	return r;
}

writehex(str)   /* write the 64-bit hex string */
char *str;
{	int i;

	for (i = 0; i < 8; i++)
		printf("%02x", str[i] & 0377);
}

hex(n)  /* convert hex nibble into integer */
int n;
{
	if (n >= 'A' && n <= 'F') return n - 'A' + 10;
	return n - '0';
}

readhex(str)	/* read 64 bits of hex code */
char *str;
{	int i, c;

	for (i = 0; i < 8; i++)
	{	c = hex(getc(fd)) << 4;
		str[i] = c | hex(getc(fd));
	}
	while ((c = getc(fd)) == ' ' || c == '\t' || c == '\n');
	ungetc(c, fd);  /* skip to next field */
	return c != EOF;
}

#endif

/************ end scrydes ************/

