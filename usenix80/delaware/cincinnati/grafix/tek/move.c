extern int plmode;
move(xi,yi)	/* dark vector to xi,yi */
	register xi, yi;
{
		putch(035);
		plmode = 1;
		pt(xsc(xi),ysc(yi));
		return;
}
