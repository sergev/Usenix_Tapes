/*	SCAME files.c				*/

/*	Revision 1.0.1  1985-02-23		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

# include "scame.h"
# include <sys/dir.h>
# include <pwd.h>
# include <ctype.h>

#ifndef BSD42
/*
 * Get working (current) directory
 */
char *getwd(path)
char path[];
{
int	file;
int	off	= 0;
struct	stat	d, dd;
struct	direct	dir;

int rdev, rino;

	path[0] = '/';
	stat("/", &d);
	rdev = d.st_dev;
	rino = d.st_ino;
	for (;;) {
		stat(".", &d);
		if (d.st_ino==rino && d.st_dev==rdev) break;
		if ((file = open("..",0)) < 0) return(NIL);
		fstat(file, &dd);
		chdir("..");
		if(d.st_dev == dd.st_dev) {
			if(d.st_ino == dd.st_ino) break;
			do {
				if (read(file, (char *)&dir,
				    sizeof(dir)) < sizeof(dir)) return(NIL);
			} while (dir.d_ino != d.st_ino);
		}
		else do {
				if(read(file, (char *)&dir,
				    sizeof(dir)) < sizeof(dir)) return(NIL);
				stat(dir.d_name, &dd);
			} while(dd.st_ino!= d.st_ino || dd.st_dev != d.st_dev);
		close(file);
		{ register i, j;
			i = -1;
			while (dir.d_name[++i] != 0);
			if ((off+i+2) > 511) break;
			for(j=off+1; j>=1; --j)
				path[j+i+1] = path[j];
			off += i+1;
			path[i+1] = '/';
			for(--i; i>=0; --i)
				path[i+1] = dir.d_name[i];
		}
	}
	if (off == 0) off = 1;
	path[off] = '\0';
	close(file);
	chdir(path);
	return(path);
}
#endif

buildfilename (fout,fin)
char	*fin, *fout;
{
register char  *p1, *p2, *p3;
	if (fin[0] == '~' && fin[1] == '/')
		sprintf(fout, "%s%s", getenv("HOME"), &fin[1]);
	else if (*fin != '/')
		sprintf(fout, "%s/%s", currentdir, fin);
	else strcpy(fout, fin);
	p2 = p1 = fout;
	while (*p1) {
		if ((*p2++ = *p1++) == '/' && p2 > fout + 1) {
			p3 = p2 - 2;
			if (*p3 == '/') --p2;	/* // */
			else if (*p3 == '.') {
			    	if  (*--p3 == '/') p2 -= 2;	/* /./ */
				else if (*p3 == '.' && *--p3 == '/') {
							/* /../ */
					while (p3 > fout && *--p3 != '/');
					p2 = p3 + 1;
				 }
			}
		}
	}
	if (*(p2 - 1) == '/') p2--;
	*p2 = '\0';
}

Bool fileexists(fname)
char *fname;
{
struct stat fst;
	return(stat(fname,&fst)==0);
}


time_t filemodified(fname)
char *fname;
{
struct stat fst;
	if (stat(fname,&fst)==0)
		return(fst.st_mtime);
	else return((time_t) 0);
}

off_t filesize(fname)
char *fname;
{
struct stat fst;
	if (stat(fname,&fst)==0)
		return(fst.st_size);
	else return((off_t) 0);
}

#ifndef BSD42
int rename(f1, f2)
char *f1, *f2;
{
	unlink(f2);
	if (link(f1, f2) == 0 && unlink(f1) == 0) return(0);
	else return(-1);
}
#endif

copyfile(from, to)
char *from,*to;
{
char s[2*FILENAMESIZE+4];
	sprintf(s,"cp %s %s", from, to);
	system(s);
}

Bool filncpy(file, tdot, n, mode)	/* Copy n chars from tdot to file. */
char *file;
char *tdot;
long n;
unsigned int mode;
{
int f;
Bool flg;
	flg = FALSE;
	if ((f=creat(file, mode)) >= 0) {
/* What if n > max int ? */
		if (write(f, tdot, (int) n) == n) flg = TRUE;
		close(f);
	}
	if (!flg) errmes(CCF);
	return(flg);
}

Bool filncat(file, tdot, n)	/* Append n chars from tdot to file. */
char *file;
char *tdot;
long n;
{
int f;
Bool flg;
	flg = FALSE;
	if ((f=open(file, 1)) >= 0) {
		lseek(f,0L,2);
/* What if n > max int ? */
		if (write(f, tdot, (int) n) == n) flg = TRUE;
		close(f);
	}
	if (!flg) echo("System error, can't save");
	return(flg);
}

Bool filnprep(file, tdot, n)	/* Prepend n chars from tdot to file. */
char *file;
char *tdot;
long n;
{
int f,tf;
Bool flg;
char tfile[FILENAMESIZE];
char tbuf[512];
int j;
	flg = FALSE;
	sprintf(tfile, tempfile, ppid, uid);
	strcat(tfile,"t");
	if ((tf=creat(tfile, 0600)) >= 0) {
/* What if n > max int ? */
		if (write(tf, tdot, (int) n) == n) {
			if ((f = open(file,0)) >= 0) {
				j = read(f, tbuf, (int)(512 - n % 512));
				write(tf, tbuf, j);
				do {
					j = read(f, tbuf, 512);
					write(tf, tbuf, j);
				} while (j == 512);
				close(f);
				unlink(file);
				flg = TRUE;
			}
			close(tf);
			rename(tfile,file);
		}
	}
	if (!flg) echo("System error, can't save");
	return(flg);
}

