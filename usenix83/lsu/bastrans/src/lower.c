lower(c)  /* put alphabetics in lower case - machine dependent ! */
int c;
{
	if (c>=65 && c<=90)
		return(c+32);
	else return (c);
}
