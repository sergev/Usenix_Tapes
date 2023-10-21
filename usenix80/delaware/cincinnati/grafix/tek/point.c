extern int plmode;

point(xi,yi)
	register xi, yi;
{
	putch(035);
	pt(xsc(xi),ysc(yi));	/* dark vector to point */
	pt(xsc(xi),ysc(yi));	/* turn point on */
	plmode = 1;
	return;
}
