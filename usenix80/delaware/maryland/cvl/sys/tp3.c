#include "tp.h"
/*	c-version of tp?.s
 *
 *	M. Ferentz
 *	August 1976
 *
 *      2.1 dlm  14 Dec 1977
 *      Change of format of tv option to allow the printing of
 *      information about large sized files and large block
 *      numbers
 *
 *      2.2 dlm  12 Jun 1978
 *      Change to have no-existent directories make if not already.
 */

#define	BSIZE	((d->d_size0&0377)<<7) | (((d->d_size1)>>9)&0177)

gettape(how)
int (*how)();
{
	register char *ptr0, *ptr1;
	register struct dent *d;
	int count;

	do {
		d = &dir[0];
		count = 0;
		do {
			if (d->d_namep == 0)  continue;
			decode(name,d);
			if (rnarg > 2) {
				ptr0 = &name;
				ptr1 = *parg;
				while (*ptr1)
					if (*ptr0++ != *ptr1++)  goto cont;
				if (*ptr0 && *ptr0 != '/')       goto cont;
			}
			(*how)(d);  /* delete, extract, or taboc */
			++count;
cont:			
			continue;
		}  
		while (++d <= lastd);
		if (count == 0 && rnarg > 2)
			printf("%s  not found\n", *parg);
		++parg;
	} 
	while (--narg > 2);
}

delete(dd)
{
	if (verify('d') >= 0)
		clrent(dd);
}

/**/

update()
{
	register struct dent *d;
	register b, last;
	int first, size;


	bitmap();
	d = &dir[0];
	do {
		if (d->d_namep == 0 || d->d_mode >= 0)    continue;
		if (d->d_size1 == 0 && d->d_size0 == 0)	  continue;
		/* find a place on the tape for this file */
		size = BSIZE;
		if (d->d_size1 & 511)   ++size;
		first = ndentd8;
toosmall:	
		++first;
		if ((last = first + size) >= tapsiz)	maperr();
		for (b = first; b < last; ++b)
			if (map[(b>>3) & ~0160000] & (1<<(b&7))) {
				first = b;
				goto toosmall;
			};
		d->d_tapea = first;
		setmap(d);
	}  
	while (++d <= lastd);
	wrdir();
	update1();
}

/**/

update1()
{
	register struct dent *d;
	register index, id;

	for (;;) {
		d = &dir[0];
		index = 32767;
		id = 0;
		do {	/* find new dent with lowest tape address */
			if (d->d_namep == 0 || d->d_mode >= 0) continue;
			if (d->d_tapea < index) {
				index = d->d_tapea;
				id = d;
			}
		} 
		while (++d <= lastd);
		if ((d = id) == 0)	return;
		d->d_mode =& ~0100000;  /* change from new to old */
		if (d->d_size1 == 0 && d->d_size0 == 0)  continue;
		decode(name,d);
		wseek(index);
		if ((id = open(name,0)) < 0) {
			printf("Can't open %s\n", name);
			continue;
		}
		for (index = BSIZE; index != 0; --index)  {
			if (read(id,tapeb,512) != 512)	    phserr();
			twrite();
		}
		if (index = d->d_size1 & 511) {
			if (read(id,tapeb,index) != index)  phserr();
			twrite();
		}
		if (read(id,tapeb,1) != 0)		    phserr();
		close(id);
	}
}

phserr()
{	
	printf("%s -- Phase error \n", name);  
}

/**/

bitmap()	/* place old files in the map */
{
	register *m, count;
	register struct dent *d;

	m = &map;
	count = sizeof map/2;
	do *m++ = 0;
	while (--count);
	count = ndirent;
	d = &dir[-1];
	do {
		d++;
		if (d->d_namep == 0 || d->d_mode < 0)
			continue;
		if (d->d_size1 || d->d_size0)
			setmap(d);
	}  
	while (--count);
}

setmap(dd)
{
	register char *c, *block, *d;
	char bit;

	d = dd;  /* used for dent pointer then as map index */
	c = BSIZE;
	if (d->d_size1 & 511)  c++;
	block = d->d_tapea;
	if ((c =+ block) >= tapsiz)		maperr();
	do {
		bit = 1 << (block & 7);
		d = (block>>3) & ~0160000;
		if (bit & map[d])		maperr();
		map[d] =| bit;
	} 
	while (++block < c);
}

maperr(nnnnn)
{
	printf("Tape overflow\n");
	done();
}

/**/

usage()
{
	register reg,count,d;
	int	nused, nentr, nfree;
	static lused;

	bitmap();
	d = &dir[0];
	count = ndirent;
	reg = 0;
	do {
		if (d->d_namep !=0)  reg++;
		d =+ sizeof dir[0];
	}  
	while (--count);
	nentr = reg;
	d = 0;		/* used to count nused */
	reg = ndentd8;
	++reg;		/* address of first non-directory tape block */
	count = tapsiz - reg;
	do {
		if (reg >= tapsiz) {
			printf("Tape overflow\n");
			done();
		}
		if (map[(reg>>3) & ~0160000] & (1 << (reg&7))) {
			d++;
			lused = reg;
		} 
		else {
			if (flags & flm)   break;
			nfree++;
		}
		reg++;
	} 
	while (--count);
	nused = d;
	printf("%5d entries\n%5d used\n", nentr, nused);
	if ((flags & (flm | fls))==0)
		printf("%5d free\n", nfree);
	printf("%5d last\n", lused);
}

/**/

taboc(dd)
{
	register  mode;
	register *m;
	register char *s;
	char *date();
	int count;

	if (flags & flv)  {
		mode = dd->d_mode;
		s = &catlb[9];
		*s = 0;
		for (count = 3; count; --count) {
			if (mode&1)	*--s = 'x';
			else		*--s = '-';
			if (mode&2)	*--s = 'w';
			else		*--s = '-';
			if (mode&4)	*--s = 'r';
			else		*--s = '-';
			mode =>> 3;
		}
		if (mode&4)		s[2] = 's';
		if (mode&2)		s[5] = 's';
		m = locv(dd->d_size0 & 255, dd->d_size1);
		printf("%s%4d%4d%6d%9s ",s,dd->d_uid, dd->d_gid,
		dd->d_tapea,m);
		/* funny business below is to make all digits pad with
				   leading zeros instead of blanks */
		printf("%s ",date(dd->d_time));
	}
	printf("%s\n", name);
}

/**/

extract(dd)
{
	register *d, count, id;

	d = dd;
	if (d->d_size0==0 && d->d_size1==0)	return;
	if (verify('x') < 0)			return;
	rseek(d->d_tapea);
	unlink(name);
	if ((id = creat(name,d->d_mode)) < 0) {
		mkdir(name);  /* make any necessary directories */
		if ((id = creat(name,d->d_mode)) < 0) {
			printf("%s -- create error\n", name);
			return;
		}
	}
	count = BSIZE;
	while (count--) {
		tread();
		if (write(id,tapeb,512) != 512)	goto ng;
	}
	if (count = d->d_size1 & 511) {
		tread();
		if (write(id,tapeb,count) != count) {
			smdate(name,"\0,\0");
ng:			
			printf("%s -- write error\n", name);
			close(id);
			return;
		}
	}
	close(id);
	count = (d->d_gid<<8) | (d->d_uid & 511);
	chown(name,count);
	smdate(name,d->d_time);   /* DOESN'T EXIST, use 'as' routine */
}

/* name:
	date

function:
	to get a date format as desired for tp

algorithm:
	Get the unix format date and jumble it around to be corect format
	Unix format is:
	day mmm dd hh:mm:ss yyyy
	our format is:
	dd mmm yyyy hh:mm

parameters:
	pointer to the doubleword time

returns:
	pointer to the date format corresponding to that time


calls:
	ctime
	localtime

called by:
*/
char *date(tvec)
int tvec[2];
{	
	register char *p;
	register char *t;

	t = ctime(tvec);
	p = dbuf;

	*p++ = t[8];    /* tens of day */
	*p++ = t[9];    /* units of day */
	*p++ = ' ';
	*p++ = t[4];    /* month */
	*p++ = t[5];    /* month */
	*p++ = t[6];    /* month */
	*p++ = ' ';
	*p++ = t[22];   /* year */
	*p++ = t[23];   /* year */
	*p++ = ' ';
	*p++ = ' ';
	*p++ = t[11];
	*p++ = t[12];
	*p++ = ':';
	*p++ = t[14];
	*p++ = t[15];
	*p   = '\0';
	printf("\0");
	return(dbuf);
}

mkdir(aname)
char   *aname;
{
	char   aname1[40], name2[40];
	char   *sp, *cp;
	int    statbuf[18];
	int astatb;

	copy(aname,aname1);
	copy(aname1,name2);

	cp = name2+1;               /* ignore starting / character */
	for (;;) {				/* forever loop */
		/* find next part of file name */
		for (; (*cp != '/') && (*cp != '\0');cp++);
		if (*cp == '\0')
			return(1);
		*cp = '\0';                     /* terminate string */
		if (stat(name2, statbuf) < 0) {
again:
			switch (fork()) {
			case 0: /* child */
				execl("/bin/mkdir", "mkdir", name2, 0);
				execl("/usr/bin/mkdir", "mkdir", name2, 0);
				printf("tp:Can't  find 'mkdir'!\n");
				exit(-1);
			case -1: /* error */
				sleep(10);
				goto again;
			default: /* error */
				wait(&astatb);
			} /* switch */
		} /* if stat */
next:
		*cp = '/';
		cp++;
	}  /* forever */
} /* mkdir */

copy(oldst, newst)
char *oldst, *newst;
{
	register count;
	register char *old, *new;

	count = 0;
	old = oldst;
	new = newst;
	while(*new++ = *old++) if(++count >= 40) {
		printf("directory name %s too long\n", oldst);
		exit(1);
	}
}
