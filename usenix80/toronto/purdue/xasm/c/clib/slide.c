slide(table,fmto,tmfrom,ltable)
	int *table;
	int fmto,tmfrom,ltable;
{
	register *f,*t;
	register c;
	if((fmto >= tmfrom)||(tmfrom > ltable))return(-1);
	t = table + fmto;
	f = table + tmfrom;
	c = tmfrom;
	c = ltable - c + 1;
	for(;c;c--)*t++ = *f++;
	return(0);
}
