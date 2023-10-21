movecursor (term) char *term;
	{
	char	bp[1024];
	char	*ptr = bp;
	if (tgetent (bp, term) != 1) return (0);
	while (*ptr)
		{
		while (*ptr && *ptr != ':') ptr++;
		if (ptr[1] == 'c' && ptr[2] == 'm') return (1);
		ptr++;
		}
	return (0);
	}
