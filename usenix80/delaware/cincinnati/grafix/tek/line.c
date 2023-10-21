extern int plmode;
line(x0,y0,x1,y1)
	register x0, y0, x1;
{
		putch(035);
		plmode = 1;
		pt(xsc(x0),ysc(y0));	/* dark vector to first point */
		pt(xsc(x1),ysc(y1));	/* line to 2d point */
		return;
}
