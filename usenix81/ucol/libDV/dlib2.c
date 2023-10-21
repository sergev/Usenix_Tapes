/*
 *	C disvec package  CIRES  August 1979  Martin Smith
 *
 */

/*	Global stuff	*/

#define	BUFSIZE	1024	/* bytes */
#define	MAXBUF	50	/* for kicks */
#define	MAXVECS	100	/* limit on number of vectors */

#include <stdio.h>

typedef double thing;	/* element type */

/*	Global variables	*/

static struct bustat {
	thing *b;	/* ptr to buffer area */
	short stat;	/* +1 clean; 0 empty; -1 dirty */
	short use;	/* usage count for age */
	long recno;	/* thing-record, from 0 */
}	
bu[MAXBUF];

static struct vecstat{
	long first;	/* first thing-record */
	long last;	/* last  thing-record */
}	
vec[MAXVECS];

static int maxbuff = MAXBUF;	/* current buffer count ceiling */
static int curbuff = -1;	/* current valid buffer index limit */
static int v = -1;		/* need an impossible index */
static int f = -1;	/* impossible file */
static int new = 1;	/* call for buffer prefill */

static char type = 'X';		/* scratch file default */
static char *scname = "/rl/DVTMPXXXXXX";
static short toolate = 0;
static char name[40];		/* actual file name */
static short nacc = 0;
static long pop = BUFSIZE/sizeof(thing);		/* things per buffer */
static long nread = 0;		/* I/O counters */
static long nwrite = 0;

/*
 *
 *	User entries
 *
 */

long set_(size)	/* returns C-index + 1 = fortran-index */
long *size;
{
	long nrecs, temp, lseek();
	int i;

	if( *size <= 0 ) return(0);	/* skip silly requests */
	if( v++ >= MAXVECS ) {	/* too many */
		fprintf(stderr,"dlib:too many vectors\n");
		by_bye(-2);
	}
	vec[v].first =  v==0?0:(vec[v-1].last+1);
	temp = *size + pop -1;
	nrecs = temp/pop;
	vec[v].last = vec[v].first + nrecs -1;

	if( ! toolate) {
		toolate = 1;
		buf_get();	/* allocate buffers */
		fil_get();	/* open disk file */
	}

/*	leave an existing file undisturbed if possible 
 *	but make sure its all there.
 */

	temp = BUFSIZE * vec[v].first;
	lseek(f,temp,0);
	for( i=0; i<nrecs; i++ ) {
		if( read(f, (char *)bu[0].b, BUFSIZE ) < BUFSIZE ) {
			lseek(f,temp,0);
			write(f, (char *)bu[0].b, BUFSIZE);
		}
		temp += BUFSIZE;
	}
	return( v + 1 );	/* fortran index */
}


long dsave_(q,z)
char *q;
long z;
{
	int i;

	if( toolate ) by_bye(-2);
	for( i=0; (name[i] = q[i]) != '\0'; i++) {
		if( i == 39 ) {
			fprintf(stderr,"dlib:name too big %s", q);
			by_bye( -2 );
		}
		if( i > (int)z ) { 
			name[i] = '\0'; 
			break; 
		}
	}
	type = 'U';
	return (0);
}

thing get_(i,j)	/* get vi(j) */
long *i, *j;
{
	struct bustat *p, *fet_ch();
	long rec, grec;
	int k;

	if( *i  > (v+1) || *j <= 0 ) by_bye ( -2 );
	rec = (*j + pop -1 );
	rec /= pop;
	grec = rec - 1 + vec[*i-1].first;
	if( grec < vec[*i-1].first || grec > vec[*i-1].last ) {
		fprintf(stderr,"dlib:out of range %D %D %D %D\n", rec, grec, *i, *j);
		by_bye( -2 );	/* force a core dump */
	}
	p = fet_ch(grec);
	k = *j - pop * ( rec - 1 ) -1 ;
	return ( (p->b)[k] );
}

long put_(i,j,x)	/* put vi(j) = x */
long *i, *j;
thing *x;
{
	struct bustat *p, *fet_ch();
	long rec, grec;
	int k;


	if( *i  > (v+1) || *j <= 0 ) by_bye(-2);
	rec = (*j + pop - 1 );
	rec /= pop;
	grec = rec - 1 + vec[*i-1].first;
	if( grec < vec[*i-1].first || grec > vec[*i-1].last ) {
		fprintf(stderr,"dlib:out of range %D %D %D %D\n", rec, grec, *i, *j);
		by_bye( -2 );	/* force a core dump */
	}
	p = fet_ch(grec);
	k = *j - pop * ( rec - 1 ) - 1;
	(p->b)[k] = *x;
	p->stat = -1;
	return ( 1 );
}


int dflush_()
{
	int i;
	if( type == 'X' ) return(-1);	/* stupid to flush scratch */
	for( i = 0; i <= curbuff; i++) {
		if( bu[i].stat == -1 ) {
			dv_out( bu + i );
			bu[i].stat = 1;
		}
	}
	return(0);
}


int dreset_()
{
	dflush_();
	close(f);
	v = -1;
	toolate = 0;
	new = 1;
	nacc = 0;
	f = -1;
	nread = 0;
	nwrite = 0;
	type = 'X';
	return (0);
}


long dsvprt_()	/* print some interesting numbers */
{
	static char *l1 = "%d buffers of length %d bytes (%D things)";
	static char *l2 = " allocated for %d vectors\n";
	static char *l3 = "issued %D reads and %D writes\n";

	fflush(stdout);
	fprintf(stdout, l1, curbuff+1, BUFSIZE, pop);
	fprintf(stdout, l2, v+1);
	if(nread>0 || nwrite>0)
		fprintf(stdout, l3, nread, nwrite);
	nread = nwrite = 0;
	return(0);
}

long dmaxbf_(n)
long *n;
{
	if( toolate ) by_bye(-2);
	maxbuff = *n < maxbuff ? *n : maxbuff ;
	return ( maxbuff );
}

#include "dlib2a.c"

