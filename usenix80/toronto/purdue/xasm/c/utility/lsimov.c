lsimov(x,y)
	int x,y;
{
	putchar('\033');	putchar('=');
	putchar(y + 040);	putchar(x + 040);
}
