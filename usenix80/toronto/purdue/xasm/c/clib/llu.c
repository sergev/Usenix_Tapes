llu(world,keylist)		/*   List Look Up	*/
	char *world,*keylist[];
{
	register char *p,*m;
	register i;

	i = 0;				/*	point to first keylist			*/
	while(keylist[i] != -1){	/*	and if not all done with them:	*/
		p = world;		/*	point to input string		*/
		m = keylist[i++];	/*	point to current keylist string	*/

		while((*p++ == *m++)&&(*(p-1)));	/* eat up equal char.	*/

		if((*--p == '\0')&&(*--m == '\0')) return(--i);
	}
	return(-1);
}
