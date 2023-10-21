sscan(world,string)
	char *world,**string;
{
	register char *p,**c;
	register i;
	p = world;
	*p = '\0';
	c = string;

	while((island(**c)!= 1)&&(**c!= '\n')&&(**c != '\0')&&(**c != ';'))(*c)++;

	if(**c == '\n') return(-1);
	if(**c == '\0') return(-2);
	if(**c == ';')return(**c);
	*p++ = *(*c)++;
	i = 1;
	while(island(*p++ = *(*c)++))
			if(++i > 20) --p;
	(*c)--;
		if(*(--p) == '\0') return(-2);
		if(*p == '\n'){
			*p = '\0';
			return(-1);
			}
	else {i = *p;	*p = '\0';}
	return(i);
}
