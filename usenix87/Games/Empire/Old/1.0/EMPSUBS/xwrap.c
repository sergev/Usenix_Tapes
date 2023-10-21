extern	int	w_xsize, w_ysize;

xwrap(x)
int	x;
{
	register int	q, wxh;

	wxh = w_xsize;
	wxh >>= 1;
	q = x;
	q += wxh;
	while( q <= 0 ) q += w_xsize;
	return(((q-1)%w_xsize) + 1 - wxh );
}

ywrap(y)
int	y;
{
	register int	q, wyh;

	wyh = w_ysize;
	wyh >>= 1;
	q = y;
	q += wyh;
	while( q <= 0 ) q += w_ysize;
	return(((q-1)%w_ysize) + 1 - wyh);
}
