main()
{
	int u, g;

	u = getuid();
	g = getgid();
	printf("user:\t%d,%d\n", u&0377, u>>8);
	printf("group:\t%d,%d\n", g&0377, g>>8);
}
