/*
 * return plural or singular endings depending on 'n'
 */

char	*
plur(n, singular, plural)
int n;
char *singular, *plural;
{
	if (n == 1)
		return(singular);
	else
		return(plural);
}

char	*
splur(n)
int n;
{
	if (n == 1)
		return("");
	else
		return("s");
}

char	*
esplur(n)
int n;
{
	if (n == 1)
		return("");
	else
		return("es");
}

char	*
iesplur(n)
int n;
{
	if (n == 1)
		return("y");
	else
		return("ies");
}
