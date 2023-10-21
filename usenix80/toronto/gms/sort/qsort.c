/*
 * Qsort - quick sort (see QSORT(III))
 * see also KNUTH, VOL.2 for the algorithm
 */

/*
 * Externals
 */

static	(*qscmp)();		/* user supplied comparison routine */
static	unsigned qssiz;		/* size of a record (in bytes) */

/*
 * Functions
 */

/*
 * The main routine for qsort.
 */

qsort(base,cnt,siz,cmp)
char	*base;
int	(*cmp)();
unsigned cnt,siz; {
/* save the record size and the address of the comparison routine */
	qssiz = siz;
	qscmp = cmp;
/* start the recursive quick sort */
	qs(base,base + (cnt * siz));
}

/*
 * Qs is a recursive function that does the actual quick sort.
 */

static	qs(base,top)
char	*base,*top; {
	register char *himid,*lomid;
	register char *hp,*lp;
	int	c;
	unsigned len;

/* calculate the length of the buffer to sort */
	len = top - base;
/* if the buffer is 0 or 1 records long, return */
	if(len <= qssiz) {
		return;
	}
/* calculate the middle record of the buffer */
	len = (len / (2 * qssiz)) * qssiz;
	lomid = himid = base + len;
/* set the high and low pointers */
	hp = top - qssiz;
	lp = base;
/* the main sorting loop */
	for(;;) {
/* move the low pointer up as far as possible */
		while(lp < lomid) {
			c = (*qscmp)(lp,lomid);
/* if they're equal, put the record in the middle */
			if(c == 0) {
				lomid -= qssiz;
				qsexc2(lp,lomid);
			}
/* if it's less, it's ok, so go on */
			else if(c < 0) {
				lp += qssiz;
			}
/* if it's greater, than we can go no further, so break out */
			else {
				break;
			}
		}
/* move the high pointer down as far as possible */
		while(hp > himid) {
			c = (*qscmp)(hp,himid);
/* if they're equal, put the record in the middle */
			if(c == 0) {
				himid += qssiz;
				qsexc2(hp,himid);
			}
/* if it's greater, it's ok, so go on */
			else if(c > 0) {
				hp -= qssiz;
			}
/* if it's less, than we can go no further, so break out */
			else {
				break;
			}
		}
/* if neither side has reached the middle, than just swap the two */
		if((lp != lomid) && (hp != himid)) {
			qsexc2(lp,hp);
			lp += qssiz;
			hp -= qssiz;
		}
/* if the high side but not the low side has reached the middle, then */
/* do a three way shift which moves the middle down */
		else if((lp != lomid) && (hp == himid)) {
			lomid -= qssiz;
			qsexc3(hp,lomid,lp);
			himid -= qssiz;
			hp = himid;
		}
/* if the low side but not the high side has reached the middle, then */
/* do a three way shift which moves the middle up */
		else if((lp == lomid) && (hp != himid)) {
			himid += qssiz;
			qsexc3(lp,himid,hp);
			lomid += qssiz;
			lp = lomid;
		}
/* if both sides have reached the middle, then this sort is complete */
		else {
			break;
		}
	}
/* now sort the low side and the high side recursively */
	qs(base,lomid);
	qs(himid + qssiz,top);
}

/*
 * Qsexc2 exchanges two records in the buffer.
 */

static	qsexc2(ap,bp)
register char	*ap,*bp; {
	register char	c;
	unsigned i;

	for(i = qssiz;i;i--) {
		c = *bp;
		*bp++ = *ap;
		*ap++ = c;
	}
}

/*
 * Qsexc3 exchanges three records in the buffer.
 * NOTE: In the calls to qsexc3 in qs above, the order of the arguments
 *       is significant because some may point to the same location.  If
 *       the order there or here is changed, it may not work properly.
 */

static	qsexc3(ap,bp,cp)
register char	*ap,*bp,*cp; {
	char	c;
	unsigned i;

	for(i = qssiz;i;i--) {
		c = *cp;
		*cp++ = *bp;
		*bp++ = *ap;
		*ap++ = c;
	}
}
