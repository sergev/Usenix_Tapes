scanin(world)
	char *world;
{
	register char *p,c;
	register i;
	p = world;
	*p = '\0';

	while((island(c = getchar())!= 1)&&(c!= '\n')&&(c != '\0')&&(c != ';'));

	if(c == '\n') return(-1);
	if(c == '\0') return(-2);
	if(c == ';')return(c);
	*p++ = c;
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
