loadch_(s,j,l,ls)
char *s;
long *j,*l,ls;
{

	*(s+ (int)*j-1) = (char) *l;
	return(0);
}
