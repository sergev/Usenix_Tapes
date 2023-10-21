eatspace(p)
	char **p;
{
	while((**p == ' ')||(**p == '\t'))(*p)++;
}
