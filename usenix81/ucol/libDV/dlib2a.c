
/*	internal functions	*/

int fil_get()
{		/* set up file */
	char *mktemp();

	if( type == 'X' ) {
		strcpy(name,scname);
		mktemp(name);
	}

	while( ( f = open(name, 2)) == -1 ) {
		if( ( f = creat(name, 0664) ) == -1 ) {
			by_bye(-2);
		}
		close(f);
	}

/*	unlinking ensures destruction upon exit
 *	but hides the existence of a (possibly large)
 *	file which is extant until the process exits.
 *	risky.
 */
	if( type == 'X' ) unlink(name);		/* ensure destruction */
	return(0);
}

int buf_get()		/* pig out on buffers */
{
	int i;
	char *malloc();
	unsigned sz = BUFSIZE;

	if( curbuff >= 0 ) return (0);	/* already allocated */
	for( i=0; i<=maxbuff-1; i++) {
		if( (bu[i].b = (thing *) malloc(sz)) <= 0 )
			break;
		else curbuff = i;
	}

	if( i == 0 ){
		fprintf(stderr, "dlib:couldn't get buffers\n");
		by_bye( -1 );
	}

	return(0);
}

int pre_buf()	/* prefills the buffers as a cheap setup */
{
	int i, j;
	j = curbuff < vec[v].last ? curbuff : vec[v].last;	/* ignore excess buffers */
	curbuff = j;
	for( i=0; i <= curbuff; i++ ) {
		bu[i].stat = 1;
		bu[i].use = i;
		bu[i].recno = (long)i;
		dv_in( bu + i );
	}

	nacc = i;
	return(0);
}

int dv_in(p)
struct bustat *p;
{
	long lseek();
	long rec;

	nread++;
	rec = BUFSIZE * (p->recno);
	if( (lseek(f,rec,0) == -1) ||
		  ( read(f, (char *) p->b, BUFSIZE) != BUFSIZE ) ) {
		fprintf(stderr,"dlib:read error\n");
		by_bye( -2 );
	}
	return(0);
}

int dv_out(p)
struct bustat *p;
{
	long lseek();
	long rec;

	nwrite++;
	rec = BUFSIZE * (p->recno);
	if( (lseek(f,rec,0) == -1) ||
		( write(f, (char *) p->b, BUFSIZE) != BUFSIZE )) {
		fprintf(stderr,"dlib:write error\n");
		by_bye( -2 );
	}
	return(0);
}

struct bustat *fet_ch(rec)
long rec;
{
	static struct bustat *ripe;
	struct bustat spare, *set_rip();
	int i, j, k, l;

	if( new ) {
		pre_buf();
		new = 0;
		ripe = bu;
	}

	if( nacc > 30000) {	/* cheap restore */
		nacc = curbuff;
		for( i=0; i<=curbuff; i++) 
			bu[i].use = i;
		ripe = bu;
	}

	nacc ++;
	for( l=0; l<2; l++) {

		j = curbuff;
		k = 0;
		while( j > (k + 1) ) {	/* search sorted list */
			i = (j + k)/2;
			if( bu[i].recno >= rec ) j = i;
			else k = i;
		}

		if( bu[k].recno == rec ) j = k;	/* special case */
		if( bu[j].recno == rec ) {	/* present */
			bu[j].use = nacc;
			if( ripe == bu + j )
				ripe = set_rip();
			return( bu + j );
		}

		else {					/* absent */
			if( ripe->stat < 0 ) dv_out(ripe);
			spare = *ripe;
			while( ripe > bu ) {
				*ripe = *( ripe - 1 );
				--ripe;
			}
			bu[0] = spare;
			bu[0].stat = 1;		/* load bu[0] */
			bu[0].use  = nacc;
			bu[0].recno = rec;
			dv_in (bu);
		}

	/* sort bu-list  only a single pass is ever needed */

		for( i = 0; i < curbuff; i++ ) {
			if( bu[i].recno > bu[i+1].recno ) {
				spare = bu[i];
				bu[i] = bu[i+1];
				bu[i+1] = spare;
			}
			else break;
		}
		ripe = set_rip();
	}

/* execution should never reach this point  */

	fprintf(stderr,"dlib:buffer manager error\n");
	by_bye( -2 );
	return(0);	/* to shut lint up */
}

struct bustat *set_rip()		/* find ripest bu-entry */
{
	int i,j,k;

	k=bu[0].use;
	j=0;
	for(i=1; i<=curbuff; i++) {
		if(bu[i].use < k ) {
			j=i;
			k=bu[i].use;
		}
	}
	return(bu + j);
}

int by_bye(n)
int n;
{		/* error handling  if n==-2 force an abort */

	perror("dlib");
	if( n == -2 ) abort();
	exit( -1 );
	return ( 0 );
}
