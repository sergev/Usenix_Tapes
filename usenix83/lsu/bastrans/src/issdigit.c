issdigit(c)  /* NOTE 2 */
char c;
{
	if (c < '0' || c > '9') return(0);
	return(1);
}
