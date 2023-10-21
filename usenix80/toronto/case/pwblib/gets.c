char *gets()
{
static char line[256];
register char *sline;

	for(sline = line; (*sline = getchar()) != '\n' && *sline; sline++);
	*sline = '\0';
	return(line);
}
