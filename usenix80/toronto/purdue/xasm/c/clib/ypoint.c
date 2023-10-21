ypoint(x,y,onoff,page)
	int x,y,onoff;	char page;
{
	register char c;

	c = (y >> 3) & 017;
	c =+ ' ';
	putchar(033);	putchar('<');
	putchar(c);
	putchar(' ' + (y & 7));
	c = (x >> 3) & 017;
	c =+ ' ';
	putchar(c);
	putchar(' ' + (x & 7));
	putchar(onoff + ' ' + (((page - '1') << 1) & 6));
}
