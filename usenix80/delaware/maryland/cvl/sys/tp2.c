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
 */

clrdir()
{
	register j, *p;

	j = ndirent * (DIRSIZE/2);
	p = &dir;
	do (*p++ = 0);  
	while (--j);
	lastd = 0;
}

clrent(ptr)
{
	register *p, j;

	p  = ptr;
	j = DIRSIZE/2;
	do *p++ = 0;
	while (--j);
	if (p == lastd) do {
		if (--lastd < &dir) {
			lastd = 0;
			return;
		}
	} 
	while (lastd->d_namep == 0);
}

/**/

rddir()
{
	register *p1, *tp, reg;
	struct dent  *dptr;
	struct tent  *tptr;
	int	count, sum;

	sum = 0;
	clrdir();
	rseek(0);
	tread();        /* Read the bootstrap block (special magtape added (R.C.S.) */
	if (((reg = tpentry[7].cksum) != 0) && (flags & (flm | fls))) {
		ndirent = reg;
		ndentd8 = ndirent>>3;
	}
	dptr = &dir[0];
	count = ndirent;
	do {
		if ((count & 07) == 0) {	/* next block */
			tread();
			tptr = &tpentry[0];
		}
		tp = tptr;
		p1 = tp + (sizeof tpentry[0])/2;
		reg = 0;
		do  reg =+ *tp++;
		while (tp<p1);
		sum =| reg;
		p1 = dptr;
		if (reg == 0) {
			tp = tptr;
			if (*tp != 0) {
				lastd = p1;
				encode(tp,p1);
				tp = &(tp->mode);
				++p1;		/* skip namep */
				reg = (sizeof dir[0]/2) - 1;
				do *p1++ = *tp++;
				while (--reg);
			}
		}
		++tptr;		/* bump to next tent */
		reg = dptr;
		reg->d_mode =& ~0100000;	/* mark previous */
		dptr = (reg =+ sizeof dir[0]);
	} 
	while (--count);
	if (sum != 0) {
		printf("Directory checksum\n");
		if ((flags & fli) == 0)		done();
	}
	bitmap();
}

/**/

wrdir()
{
	register reg, *tp,*dp;
	struct dent *dptr;
	struct tent *tptr;
	int	count;

	wseek(0);
	if (flags & (flm | fls))  /* Special magtape added (R.C.S.) */
		reg = open("/usr/mdec/mboot",0);
	else	reg = open("/usr/mdec/tboot",0);
	if (reg >= 0) {
		read(reg,tapeb,512);
		close(reg);
		tpentry[7].cksum = ndirent;
	}
	dptr = &dir[0];
	count = ndirent;
	for (;;) {
		twrite();
		if (count == 0)  return;
		tp = &tpentry[0];
		do {
			tptr = tp;
			dp = dptr++;	/* dptr set to next entry */
			if (dp->d_namep)  {
				decode(tp,dp);
				tp = &(tp->mode);  /* point to mode  */
				++dp;		   /* point to d_mode */
				do *tp++ =  *dp++;
				while (dp < dptr);
				tp = tptr;
				dp = &tp[31];	/* points to checksum */
				reg = 0;
				do reg =- *tp++;   /* form checksum */
				while (tp< dp);
				*tp++ = reg;
			} 
			else {
				reg = sizeof tpentry[0]/2;
				do *tp++ = 0;		/* clear entry */
				while (--reg);
			}
		} 
		while (--count & 07);
	}
}
/**/

tread()
{
	register j, *ptr;

	if (read(fio,tapeb,512) != 512) {
		printf("Tape read error\n");
		if ((flags & fli) == 0)		done();
		ptr = tapeb;
		j = 256;
		do *ptr = 0;
		while (--j);
	}
	rseeka++;
}

twrite()
{
	if (write(fio,tapeb,512) != 512) {
		printf("Tape write error\n");
		done();
	}
	++wseeka;
}

rseek(blk)
{
	rseeka = blk;
	if (seek(fio,blk,3) < 0)	seekerr();
}

wseek(blk)
{
	register amt, b;

	amt = b = blk;
	if ((amt =- wseeka) < 0)	amt = -amt;
	if (amt > 25 && b) {
		seek(fio, b-1, 3);	/* seek previous block */
		read(fio, &wseeka, 1);  /* read next block */
	}
	wseeka = b;
	if (seek(fio, b, 3) < 0)	seekerr();
}

seekerr()
{
	printf("Tape seek error\n");
	done();
}
/**/

verify(key)
{
	register c;

	if ((flags & (flw | flv)) == 0)
		return(0);
repeat:	
	printf("%c %s ", key, name);
	if ((flags & flw) == 0) {
		printf("\n");
		return(0);
	}
	c = getchar();
	if (c == 'n' && getchar() == '\n')
		done();
	if (c == '\n')
		return(-1);
	if (c == 'y' && getchar() == '\n')
		return(0);
	while (getchar() != '\n');
	goto repeat;
}

getfiles()
{
	register char *ptr1, *ptr2;

	if ((narg =- 2) == 0) {
		name->integer = '.\0';
		callout();
	} 
	else while (--narg >= 0) {
		ptr1 = *parg++;
		ptr2 = &name;
		while (*ptr2++ = *ptr1++);
		callout();
	}
}

/**/

expand()
{
	register  char *p0, *p1, *save0;
	int n, fid;

	if ((fid = open(name,0)) < 0)		fserr();
	for (;;) {
		if ((n = read(fid,catlb,16)) != 16){
			if (n == 0) {
				close(fid);
				return;
			}
			fserr();
		}
		if (catlb[0] == 0)	/* null entry */
			continue;
		p0 = &name;
		p1 = &catlb[1];
		if (*p1 == '.')		/* don't save .xxxx */
			continue;
		while (*p0++);
		save0 = --p0;		/* save loc of \0 */
		if (p0[-1] != '/')
			*p0++ = '/';
		while (*p0++ = *p1++);
		callout();
		*save0 = 0;		/* restore */
	}
}

fserr()
{
	printf("%s -- Cannot open file\n", name);
	done();
}

callout()
{
	register struct dent *d;
	register char *ptr1, *ptr0;
	char *empty;

	if (stat(name,&statb) < 0)	fserr(3);
	ptr0 = statb.s_flags;
	if ((ptr0 =& 060000) != 0) {
		if (ptr0 == 040000)  /* directory */
			expand();
		return;
	}
	/* when we reach here we have recursed until we found 
		 * an ordinary file.  Now we look for it in "dir".
		 */
	empty = 0;
	d = &dir[0];
	do  {
		if (d->d_namep == 0) {	/* empty directory slot */
			if (empty == 0) /* remember the first one */
				empty = d;
			continue;
		}
		decode(name1,d);
		ptr0 = &name;
		ptr1 = &name1;
		do	if (*ptr0++ != *ptr1)   goto cont;
		while (*ptr1++);
		/* veritably the same name */
		if (flags & flu) {  /* check the times */
			if (d->d_time[0] > statb.s_modtime[0])
				return;
			if (d->d_time[0] == statb.s_modtime[0] &&
			    d->d_time[1] >= statb.s_modtime[1])
				return;
		}
		if (verify('r') < 0)	return;
		goto copydir;
cont:		
		continue;
	}  
	while (++d <= lastd);
	/* name not found in directory */
	if ((d = empty) == 0) {
		d = lastd +1;
		if (d >= edir) {
			printf("Directory overflow\n");
			done();
		}
	}
	if (verify('a') < 0)		return;
	if (d > lastd)		lastd = d;
	encode(name,d);
copydir:
	d->d_mode = statb.s_flags | 0100000;
	d->d_uid = statb.s_uid;
	d->d_gid = statb.s_gid;
	if (flags &flf)	{	/* fake entry */
		statb.s_size0 = 0;
		statb.s_size1 = 0;
	}
	d->d_size0 = statb.s_size0;
	d->d_size1 = statb.s_size1;
	d->d_time[0] = statb.s_modtime[0];
	d->d_time[1] = statb.s_modtime[1];
}
