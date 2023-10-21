/*
	n = ptr to name
	vp= ptr to var with current value
	wv= default value
*/
bytefix(n, vp, wv)
char	*n, *vp;
int	wv;
{
	*vp = (char)fixup(n, ((int)*vp & 0377), wv);
}
