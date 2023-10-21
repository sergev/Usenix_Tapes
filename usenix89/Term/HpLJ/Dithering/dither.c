
#include <stdio.h>
#define WHITE 0
#define BLACK 1
#define ENDSCAN 2

int xsize,ysize;

main()
{
	int x,y,level;
	scanf("%d %d",&ysize,&xsize);
	printf("\033*t100R\n");
	printf("\033*r0A\n");
	for (x=0;x<xsize;x++) {
		for (y=0;y<ysize;y++) {
			level = getchar();
			if (dither(x, y, level)) makedot(WHITE);
			else makedot(BLACK);
		}
		makedot(ENDSCAN);
	}
	printf("\033*rB\n");
	
}
dither (x,y,level) /* dithering function */
register int x, y, level;
{
	 /* 8 x 8 dithering matrix */
	static int matrix[8][8] = {
	{  0,128, 32,160,  8,136, 40,168},
	{192, 64,224, 96,200, 72,232,104},
	{ 48,176, 16,144, 56,184, 24,152},
	{240,112,208, 80,248,120,216, 88},
	{ 12,140, 44,174,  4,132, 36,164},
	{204, 76,236,108,196, 68,228,100},
	{ 60,188, 28,156, 52,180, 20,148},
	{252,124,210, 92,244,116,212, 84}
	};



/* dithering by area access to matrix by using
	the MOD function on the x and y coordinates */
	return(level > matrix[x % 8][y % 8]);
}

makedot(color)
int color;
{
	static int	i,
			whichbyte = 0,
			whichbit = 0,
			nbytes,
			thisbyte = 0;

	nbytes = (ysize/8)+1;
	if (color == ENDSCAN) {
		/* end of scan line.  We have to send 'thisbyte',
		   and any other bytes required to make up a full
		   line (nbytes - whichbyte) */
		putchar(thisbyte);
		for (i=whichbyte+1;i<nbytes;i++) putchar(0);
		whichbyte = 0;
		whichbit = 0;
		thisbyte = 0;
	} else {
		if ((whichbyte == 0) & (whichbit == 0))
			printf("\033*b%dW",nbytes);
		thisbyte |= (color << (7 - whichbit++));
		if (whichbit > 7) {
			whichbyte++;
			whichbit = 0;
			putchar(thisbyte);
			thisbyte = 0;
		}
		if (whichbyte >= nbytes) {
			whichbyte = 0;
		}
	}
}

