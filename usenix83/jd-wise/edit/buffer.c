#include "ed.h"
#include "file.h"
#include "window.h"
#define buffile 5	/* same as command window */
#define firsttl()	filedata[buffile].tline

struct buffer{
	int	bu_file;
	int	bu_first;
	int	bu_last;
	struct buffer *bu_link;
	};

#define nbuffers 32
struct buffer buffer[nbuffers];

static struct buffer *gbp;	/* for passing the buffer to getbuf */
static int gtl;			/* to keep track of current line */
int getbuf();

/*
 * buffers are implemented in the following way:
 * for each group of lines in the buffer which are contiguous
 * in the temp file, the addresses of the first and last
 * are remembered.
 * the trick is that since each line in the temp file ends in a null
 * and that the next line begins at the next even multiple of 4,
 * it is always possible to find all the lines in the range
 * given only the first and last.
 * if there is only one such block, it is kept in the buffer array.
 * if there are more than one, they are kept on a chain linked by bu_link.
 *
 * unfortunately, the only accurate means of testing for contiguity
 * is to scan the entire range of lines. this adds considerable overhead
 * to yank which in turn adds overhead to delete.
 */

initbuf()
	{
	register struct buffer *bp;
	for(bp=buffer; bp < &buffer[nbuffers]; bp++){
		bp->bu_file=bp->bu_last = -1;
		bp->bu_first=0;
		bp->bu_link=(struct buffer *)0;
		}
	}

syncbuf(fn)
	{
	register struct buffer *bp;
	for(bp=buffer; bp < &buffer[nbuffers]; bp++)
		if(bp->bu_file==fn){
			if(bp==buffer)	/* just invalidate unnamed buffer */
				bp->bu_file=bp->bu_first=bp->bu_last = -1;
			else{	/* copy out the others */
				int ftl,ltl;
				ftl=firsttl();
				primebuf(bp-buffer);
				while(getbuf()==0)
					ltl=putbline();
				bp->bu_file=buffile;
				bp->bu_first=ftl;
				bp->bu_last=ltl;
				}
			freechain(bp);	/* release any links */
			}
	}

nexttl(tl)
	{
	extern char *eolp;

#ifdef debugging
debug(84,"nexttl: tl=0%o eolp=0%o linebuf=0%o",tl,eolp,linebuf);
#endif
	return(tl + ((((eolp-linebuf)+03)>>1)&077776));
	}

getbuffile()
	{
	register struct filedata *fp = &filedata[buffile];

	/** this code is common with part of init and
	 ** should probably be merged with gettemp */
	fp->tline = 0;
	fp->iblock= -1;
	fp->oblock= -1;
	fp->ichanged=0;
	gettemp(buffile);
	}

putbline()
	{
	if(filedata[buffile].tfile == -1)
		getbuffile();
	return(putline(buffile));
	}

int loading;	/* to communicate with getcc */
loadbuf(n)
	{
	register struct buffer *bp;
	register char *lp;
	register c;
	int tl;

	if(n<0 || n>=nbuffers)
		errfunc("Bad buffer name");
	message("Loading buffer %c",wname(n));
	loading=n+'`';
	bp = &buffer[n];
	freechain(bp);
	bp->bu_file=buffile;
	bp->bu_first=firsttl();
	for(;;){
		for(lp=linebuf; c=getchr(); lp++){
			*lp=c;
			if(c=='\n'){
				*lp='\0';
				if(lp==linebuf+1 && linebuf[0]=='.')
					goto done;
				tl=putbline();
				break;
				}
#ifdef debugging
debug(85,"loadbuf: line=%s",linebuf);
#endif
			}
		}
done:
	bp->bu_last=tl;
	message("");
	loading=0;
	}

/** yank(0) from delete is all it takes! */
yank(n)
	{
	register struct window *wp;
	register struct buffer *bp;
	register int *a;
	int ltl,ntl;

	if(n<0 || n>=nbuffers)
		errfunc("Bad buffer name");
	wp = addr1.ad_window;
	bp = &buffer[n];
	freechain(bp);
	bp->bu_file=wp->wi_fileno;
	bp->bu_first = *addr1.ad_addr;
	/** this is the gruesome test for contiguity
	 ** it could be made more efficient by testing addresses
	 ** against the saved value of $ after initial readin.
	 ** any addresses less than that are automatically contiguous */
	for(ltl = *(a=addr1.ad_addr); a<addr2.ad_addr; a++, ltl=ntl){
		getline(bp->bu_file,ltl);
#ifdef debugging
debug(95,"yank: ltl=%o ntl=%o a[1]=%o",ltl,nexttl(ltl),a[1]);
#endif
		if((ntl=nexttl(ltl))!=a[1]){
			bp->bu_last=ltl;
			bp=(bp->bu_link=(struct buffer *)malloc(sizeof *bp));
			bp->bu_link=0;
			bp->bu_file=wp->wi_fileno;
			bp->bu_first=ntl=a[1];
			}
		}
	bp->bu_last  = *addr2.ad_addr;
#ifdef debugging
debug(81,"yank: first=0%o last=0%o",bp->bu_first,bp->bu_last);
#endif
	}

put(n)
	{
	primebuf(n);
	append(addr1.ad_window,getbuf,addr1.ad_addr);
	}

getbuf()
	{
#ifdef debugging
debug(82,"getbuf: gtl=0%o",gtl);
#endif
	if(gtl>gbp->bu_last){
		if(gbp->bu_link==0)
			return(EOF);
		else
			gtl=(gbp=gbp->bu_link)->bu_first;
		}
	getline(gbp->bu_file, gtl);
	gtl=nexttl(gtl);
	return(0);
	}

primebuf(n)
	{
	register struct buffer *bp;

	if(n<0 || n>=nbuffers)
		errfunc("Bad buffer name");
	gbp=bp = &buffer[n];
	if(bp->bu_file<0)
		errfunc("Text buffer %c empty",wname(n));
	gtl=bp->bu_first;
	}

freechain(bp)	/* release any links in a buffer */
	register struct buffer *bp;
	{
	if(bp->bu_link==0) return;
	freelink(bp->bu_link);
	bp->bu_link=0;
	}

freelink(bp)
	register struct buffer *bp;
	{
	/* crude but effective */
	if(bp->bu_link)
		freelink(bp->bu_link);
	else
		free(bp);
	}
