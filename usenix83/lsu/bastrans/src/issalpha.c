issalpha(c)  /* return true for upper case or 0-9, false for all else */
char c;
{
	if ((c < '0' || c > '9') && (c < 65 || c > 90)) return(0);
	return(1);
}
