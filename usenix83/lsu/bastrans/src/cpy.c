cpy(s,t)  /* duplicate t in s */
char *s, *t;
{
	int i=0;
	while((*(s+i) = *(t+i)) != '\0') i++;
}
