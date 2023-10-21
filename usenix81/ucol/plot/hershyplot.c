/*
 *	These routines were taken from /usr/src/cmd/vroff/hershy/makec.c
 *	which were intended to be used to make bit map font sets for 
 *	the vcat phototypsetter simulator.  They have been modified here
 *	to be used for actual line plotting so that the numerous hershy
 *	can be made available for plot labeling
 */

double sinangle,cosangle,tanslant,xstart,ystart,hscale,vscale;
int fixflag = 0;
int dir, n, nk, kb;
int yref,yht;
int xlenoff,ylenoff;
int xoff = 50;
int yoff = 50;
int MAXwidth 192;
int charwidth;
long int ixstore,iystore;
long int ccnum;
struct inbuf
	{
	int fildes;
	int nleft;
	char *nextp;
	char buff[512];
	int seek;
	} cinbuf, finbuf;
int csptr[2] {0};
int fsptr[2] {0};
int array[400];
int bxoff;
int axoff;
int xabs, yabs;

fopen(fil, io)
	char *fil;
	struct inbuf *io;
	{

	if((io->fildes = open(fil, 0)) < 0)
		return(-1);
	io->nleft = 0;
	io->seek = -1;
	return(io->fildes);
	}

initfonts()
	{
	if((dir = open("/usr/src/cmd/vroff/hershy/cfd", 0)) < 0)
		error("Directory not found");
	if(fopen("/usr/src/cmd/vroff/hershy/fontlib", &finbuf) < 0)
		error("Fontlib not found");
	if(fopen("/usr/src/cmd/vroff/hershy/charlib", &cinbuf) < 0)
		error("Charlib not found");
	}

rchar(cnum)
	int cnum;
	{
	register blk;
	register i, *ap;
	int off;
	char cbuf[40];

	ccnum = cnum;
	lseek(dir, (ccnum-1)<<3, 0);
	if(read(dir, csptr, 4) != 4 ||
			read(dir, fsptr, 4) != 4) {
				sprintf(cbuf,"not a good integer, %d\n",cnum);
						error(cbuf);
				return(-1);
				}
	blk = csptr[0]<<7 | ((csptr[1]>>9) & 0177);
	off = csptr[1] & 0777;
	if(blk == 0 && off == 0 && cnum != 1)
		return(0);
	cseek(&cinbuf, blk, off);
	getint(20, &cinbuf);
	bxoff = getint(5, &cinbuf);
	getint(6, &cinbuf);
	axoff = getint(4, &cinbuf);
	blk = fsptr[0]<<7 | ((fsptr[1]>>9) & 0177);
	off = fsptr[1] & 0777;
	cseek(&finbuf, blk, off);
	getint(4, &finbuf);
	n = getint(4, &finbuf);
	if(cnum != n) {
		sprintf(cbuf, "invalid font number %d", cnum);
		error(cbuf);
	}
	getint(11, &finbuf);
	kb = getint(4, &finbuf);
	getint(4, &finbuf);
	ap = array;
	nk = kb*8;
	while(nk > 0)
		{
		while((i = getint(2, &finbuf)) >= 0 && nk > 0)
			{
			*ap++ = i;
			nk--;
			}
		getint(11, &finbuf);
		}
	return(1);
	}

getint(nchars, io)
	int nchars;
	struct inbuf *io;
	{
	register i, val;
	int sign;
	char cbuf[30];
	register char *cptr;

	val = 0;
	sign = 1;
	cptr = cbuf;
	for(i = 0; i < nchars; i++)
		{
		if((*cptr = getc(io)) == -1)
			error("EOF");
		if(*cptr == '\n')
			return(-1);
		if(*cptr == '-')
			sign = -1;
		if(*cptr != ' ' && *cptr != '+' && *cptr != '-')
			val = val*10+(*cptr)-'0';
		cptr++;
		}
	return(val*sign);
	}



getc(iob)
	struct inbuf *iob;
	{
	register struct inbuf *io;

	io = iob;
	if(io->nleft <= 0)
		{
		io->nleft = read(io->fildes, io->buff, 512);
		if(io->nleft < 0)
			error("font read");
		io->nextp = io->buff;
		io->seek++;
		}
	if(io->nleft-- <= 0)
		return(-1);
	return(*(io->nextp++));
	}


cseek(io, blk, off)
	struct inbuf *io;
	int blk, off;
	{
	long int bblk;
	char cbuf[50];
	bblk = blk;
	bblk *= 512;

	lseek(io->fildes, bblk, 0);
	if(read(io->fildes, io->buff, 512) <= 0) {
		perror("strangeway error");
		sprintf(cbuf,"read err infontlib, bblk = %ld",bblk);
		error(cbuf);
	}
	io->nextp = &io->buff[off];
	io->nleft = 512-off;
	io->seek = blk;
	}

makec()
	{
	register nnu, i;
	register mode;
	char cbuf[40];

	nk = kb*8;
	for(i = 0; i < nk; i++)
		if(array[i] >= 50)
			array[i] =- 100;
	for(i = 3; i < nk; i =+ 2)
		array[i] = -array[i];
	for(i = 0; i < nk; i++)
		array[i] =+ 50;
	for(i = 0; i < nk; i =+ 2)
		if((array[i] == 0) && (array[i+1] == 100))
			break;
	nnu = i;
	mode = 0;
	xabs = 0;
	yabs = 0;
	for(i = 2; i < nnu; i =+ 2)
		{
		if((array[i] == 0) && (array[i+1] == 50))
			{
			mode = 0;
			continue;
			}
		if(mode) {
			scan(xabs-bxoff-xlenoff-xoff,yabs+yref-yoff-ylenoff,array[i]-xabs,array[i+1]-yabs);
			}
		   else
			mode++;
		xabs = array[i];
		yabs = array[i+1];
		}
	up_width(-bxoff+axoff);
	xstart += charwidth*hscale*cosangle;
	ystart += charwidth*hscale*sinangle;
	}
error(str)
	char *str;
	{

	write(2, "Mfont: ", 7);
	write(2, str, strlen(str));
	write(2, "\n", 1);
	exit(0);
	}


scan(ix,iy,idx,idy)
int ix,iy,idx,idy;
{
	int ixx,iyy,idxx,idyy;
	double xx,yy,dxx,dyy;
	double x,y,dx,dy;

	yy = iy; dyy = idy;
	xx = ix + yy*tanslant;
	dxx = idx + dyy*tanslant;
	xx *= hscale; dxx *= hscale;
	yy *= vscale; dyy *= vscale;
	x = xx*cosangle - yy*sinangle + xstart;
	y = xx*sinangle + yy*cosangle + ystart;
	dx = dxx*cosangle - dyy*sinangle;
	dy = dxx*sinangle + dyy*cosangle;
	ixx = x; iyy = y;
	idxx = x + dx; idyy = y + dy;
	if (fixflag == 0 && ixx == ixstore && iyy == iystore)
		cont(idxx,idyy);
	else
		line(ixx,iyy,idxx,idyy);
	ixstore = idxx;
	iystore = idyy;
	fixflag = 0;
}
up_width(w)
	register int w;
	{

	w = w*1.;
	if(w > MAXwidth)
		w = MAXwidth;
	charwidth = w;
	}

mline_(x)

float *x;
{

	yoff -= (*x) * yht;
	xstart = 0;
}
/*
 *	This routine will retreive the proper hershy font index given
 *	a font type number and an ascii character
 */

extern int fontab[][128];

int hindex(font,character)

int font;
char character;
{

	int i,j;

	if (font < 1) return(-font);
	if (font < 3) {
		yref=4; yht=9;
	}
	else if (font > 5 && font < 9) {
		yref=6; yht=13;
		}
		else  {
		yref=9; yht=21;
		}
	i = character;
	j = font - 1;
	return(fontab[j][i]);
}
