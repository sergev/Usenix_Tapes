seedin(world,seed)
	char *world;
	char seed;
{
	register char *p,c;
	register i;
	p = world;
	c = seed;

	*p++  = c;
	i = 1;
	while(island(*p++ = getchar()))
			if(++i > 20) --p;
		if(*(--p) == '\0') return(-2);
		if(*p == '\n'){
			*p = '\0';
			return(-1);
			}
	else {i = *p;	*p = '\0';}
	return(i);
}
