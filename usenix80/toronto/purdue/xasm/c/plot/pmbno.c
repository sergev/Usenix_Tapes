pmbno(pstat,n)
	int pstat[],n;
{
/*	print out n in HP MBN format	*/
	register char c;

	n =& 0177777;
	if(n < 16){
		plotp(pstat,n + '`');
		return;
		}
	if(n < 1024){
		c = '`' + ((n >> 6)& 017);
		plotp(pstat,c);
		c = n & 077;
		if(!(c & 040))c =+ 0100;
		plotp(pstat,c);
		return;
		}
	c = (n >> 12) & 07;
	plotp(pstat,c + 0140);
	c = (n >> 6) & 077;
	if(!(c & 040))c =+ 0100;
	plotp(pstat,c);
	c = n & 077;
	if(!(c & 040))c =+ 0100;
	plotp(pstat,c);
}
