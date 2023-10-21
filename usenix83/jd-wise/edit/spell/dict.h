#include <stdio.h>
int traceflag;
#define SHIFT	4	/* log2(sizeof(short)) */

/* nbits=SIZE*sizeof(short) */
#define DICTSIZE 24000
short	dicttab[DICTSIZE];
#define STOPSIZE 1000
short	stoptab[STOPSIZE];

short	*tab[]={
	dicttab,
	stoptab,
	};

#define NT	2
#define	NP	11
#define	NW	30

long	P[NT][NP] = {
	{
	383869,
	383891,
	383909,
	383917,
	383923,
	383941,
	383951,
	383963,
	383969,
	383983,
	383987,
	},{
	15889,
	15901,
	15907,
	15913,
	15919,
	15923,
	15937,
	15959,
	15971,
	15973,
	15991,
}
};

/*
 * Hash table for spelling checker has n bits.
 * Each word w is hashed by k different (modular) hash functions, hi.
 * The bits hi(w), i=1..k, are set for words in the dictionary.
 * Assuming independence, the probability that no word of a d-word
 * dictionary sets a particular bit is given by the Poisson formula
 * P = exp(-y)*y**0/0!, where y=d*k/n.
 * The probability that a random string is recognized as a word is then
 * (1-P)**k.  For given n and d this is minimum when y=log(2), P=1/2,
 * whence one finds, for example, that a 25000-word dictionary in a
 * 400000-bit table works best with k=11.
 */

long	POW2[NT][NP][NW];

prime(dfile,sfile)
	char *dfile,*sfile;
	{
	int i, j, t;
	long h;
	register long *lp;
	register long *p;

	if (dfile) {
		int fd;
		if ((fd = open(dfile,0)) < 0)
			return(0);
		if (read(fd, dicttab, sizeof dicttab) != sizeof dicttab)
			return(0);
		close(fd);
		}
	if (sfile) {
		int fd;
		if ((fd = open(sfile,0)) < 0)
			return(0);
		if (read(fd, stoptab, sizeof stoptab) != sizeof stoptab)
			return(0);
		close(fd);
		}
	for (t=0; t<NT; t++) {
		p=P[t];
if(traceflag)
fprintf(stderr,"prime: p=%o P=%o POW2=%o\n",
p,P,POW2);
		for (i=0; i<NP; i++) {
			h = *(lp = POW2[t][i]) = 1<<14;
			for (j=1; j<NW; j++){
				h = *++lp = (h<<7) % p[i];
if(traceflag)
fprintf(stderr,"prime: t=%d i=%d j=%d lp=%o h=%D p[i]=%D\n",
t,i,j,lp,h,p[i]);
				}
			}
		}
	return(1);
	}

#define get(tab,h)	(tab[h>>SHIFT]&(1<<((int)h&((1<<SHIFT)-1))))
#define set(tab,h)	tab[h>>SHIFT] |= 1<<((int)h&((1<<SHIFT)-1))
