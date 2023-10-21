upper(c)  /* put alphabetics in upper case - machine dependent ! */
int c;
{
	if (c>=97 && c<=122)
		return(c-32);
	else return (c);
}
