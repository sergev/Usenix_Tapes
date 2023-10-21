ostr_(mdevot,s,len,ls)
long *mdevot;
long *len, *ls;
char *s;
{
	int i;
	int fot;

	fot = *mdevot;

	for(i=0; i< *len; i++)
		write(fot,s+i,1);
	return(0);
}
