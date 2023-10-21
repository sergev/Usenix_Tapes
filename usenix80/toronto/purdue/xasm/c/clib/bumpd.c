bumpd(table,fmto,tmfrom,ltable)
	char *table;
	int fmto,tmfrom,ltable;
{
	register char *f,*t;
	register c;
	if((fmto >= tmfrom)||(tmfrom > ltable))return(-1);
	t = table + fmto;
	f = table + tmfrom;
	c = ltable- tmfrom + 1;
	for(;c;c--)*t++ = *f++;
	return(0);
}
