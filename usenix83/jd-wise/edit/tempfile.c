/*
 * temporary file managment routines
 */

#include "ed.h"
#include "window.h"	/* so window won't be undefined in edit.h */
#include "edit.h"	/* for type declarations & errline */
#include "file.h"
#include "process.h"
#include "shell.h"
#include "spell.h"


blkio(fn, b, buf, iofcn)
	char *buf;
	int (*iofcn)();
	{
	extern errno;
	long l;
	int n,t;
	register struct filedata *fp;

	fp = &filedata[fn];
	errno=0;
#ifdef eunice
	l=lseek(fp->tfile, (long)b<<9, 0);
#else
	l=lseek(fp->tfile, (long)b<<9, 3);
#endif
	if ((n=(*iofcn)(fp->tfile, buf, 512)) != 512) {
t=alarm(0);
saveloc();
topt(20,0,(char*)0);
printf("errno=%d tfile=%d n=%d iofcn=0%o l=%D b=%d t=%d",
errno,fp->tfile,n,iofcn,l,b,t);
flush();
restorloc();
alarm(30);
		errmsg("Temporary file error");
		l=lseek(fp->tfile, (long)b<<9, 3);
		if ((n=(*iofcn)(fp->tfile, buf, 512)) != 512) 
{
			errfunc("Retry failed");
}
		}
	}

char*
getblock(fn, atl, iof)
	{
	extern read(), write();
	register bno, off;
	register struct filedata *fp;
	
	fp = &filedata[fn];
	bno = (atl>>8)&0377;
	off = (atl<<1)&0774;
	if (bno >= 255) {
		errfunc("Temporary file block address too large");
		}
	fp->nleft = 512 - off;
	if (bno==fp->iblock) {
		fp->ichanged |= iof;
		return(fp->ibuff+off);
		}
	if (bno==fp->oblock)
		return(fp->obuff+off);
	if (iof==READ) {
		if (fp->ichanged)
			blkio(fn, fp->iblock, fp->ibuff, write);
		fp->ichanged = 0;
		fp->iblock = bno;
		blkio(fn, bno, fp->ibuff, read);
		return(fp->ibuff+off);
		}
	if (fp->oblock>=0)
		blkio(fn, fp->oblock, fp->obuff, write);
	fp->oblock = bno;
	return(fp->obuff+off);
	}

char*	eolp;	/* for buffer routines */

char*
getline(fn, tl)
	{
	register char *bp, *lp;
	register nl;
	struct filedata *fp;
int atl;
atl=tl;

	fp = &filedata[fn];
	lp = linebuf;
	bp = getblock(fn, tl, READ);
	nl = fp->nleft;
	tl &= ~0377;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(fn, tl+=0400, READ);
			nl = fp->nleft;
			}
#ifdef debugging
debug(23,"getline: tl=0%o line=%s",atl,linebuf);
#endif
	eolp = lp;	/* so getbuf can find next line */
	return(linebuf);
	}

putline(fn)
	{
	register char *bp, *lp;
	register nl;
	int tl;
	struct filedata *fp;

	fp = &filedata[fn];
	lp = linebuf;
	tl = fp->tline;
	bp = getblock(fn, tl, WRITE);
	nl = fp->nleft;
	tl &= ~0377;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
			}
		if (--nl == 0) {
			bp = getblock(fn, tl+=0400, WRITE);
			nl = fp->nleft;
			}
		}
	nl = fp->tline;
	fp->tline += (((lp-linebuf)+03)>>1)&077776;
	return(nl);
	}


gettemp(fn)	/* create or read in temporary files */
	{
	register pid;
	register struct filedata *fp;
	register struct window *wp;
	int *oldzero;

	fp = &filedata[fn];
	wp = w[fn];
	if(!fp->gottemp){
#ifdef eunice
		pid=(crashpid?crashpid:getpid())%1000000;
#else
		pid=crashpid?crashpid:getpid();
#endif
		sprintf(fp->tfname,"/tmp/e%05d%ca",pid,wname(fn));
		sprintf(fp->sfname,"/tmp/e%05d%cb",pid,wname(fn));
		/** this should be done in such a way that the
		 ** memory can be freed when the window is closed */
		fp->ibuff = malloc(512);
		fp->obuff = malloc(512);
		fp->gottemp++;
		}
	if(crashpid==0){
		if((fp->tfile=creat(fp->tfname,0600))<0
		 ||(fp->sfile=creat(fp->sfname,0600))<0)
			errxit("Can't create temporary files.");
		close(fp->tfile);
		close(fp->sfile);
		}
	if((fp->tfile=open(fp->tfname,2))<0 || (fp->sfile=open(fp->sfname,2))<0)
		errxit("Can't open temporary files.");
	if(crashpid){
		read(fp->sfile,(char*)&oldzero,sizeof oldzero);
		read(fp->sfile,(char*)&wp->wi_dot,sizeof wp->wi_dot);
		read(fp->sfile,(char*)&wp->wi_dol,sizeof wp->wi_dol);
		read(fp->sfile,(char*)&fp->nleft,sizeof fp->nleft);
		read(fp->sfile,(char*)&fp->tline,sizeof fp->tline);
		wp->wi_nlall=(wp->wi_dol-oldzero)+1;
		wp->wi_zero=(int*)malloc(wp->wi_nlall*sizeof(wp->wi_dol));
		wp->wi_dot += wp->wi_zero-oldzero;
		wp->wi_dol += wp->wi_zero-oldzero;
		read(fp->sfile,(char*)wp->wi_zero,wp->wi_nlall*sizeof(wp->wi_dol));
		globp="x";
		crashpid=0;	/* only do this once */
		}
	}

synctemp()	/* update text and sync files */
	{
	extern write();
	int fn;
	register struct filedata *fp;
	register struct window *wp;

	for(fn=1; fn<=3; fn++){	/* only sync edit files */
		wp = w[fn];	/** poor sync */
		fp = &filedata[fn];
		if(fp->tfile == -1 || wp==(struct window *)0) continue;
		syncscreen(wp);
		if (fp->ichanged)
			blkio(fn, fp->iblock, fp->ibuff, write);
		if (fp->oblock>=0)
			blkio(fn, fp->oblock, fp->obuff, write);
		lseek(fp->sfile,0L,0);
		write(fp->sfile,(char*)&wp->wi_zero,sizeof wp->wi_zero);
		write(fp->sfile,(char*)&wp->wi_dot,sizeof wp->wi_dot);
		write(fp->sfile,(char*)&wp->wi_dol,sizeof wp->wi_dol);
		write(fp->sfile,(char*)&fp->nleft,sizeof fp->nleft);
		write(fp->sfile,(char*)&fp->tline,sizeof fp->tline);
		write(fp->sfile,(char*)wp->wi_zero,(wp->wi_dol-wp->wi_zero+1)*sizeof(wp->wi_dol));
		}
#ifndef eunice
	sync();
#endif
	}

errxit(msg)
	char *msg;
	{
	printf("%s\n",msg);
	flush();
	cleanup();
	exit(1);
	}
