cat(s,t)
char *s, *t;
{
	register i,j;
	for (i=j=0;*(s+i)!='\0' && *(s+i)!='\n';i++);
	while ((*(s+i++)= *(t+j++)) != '\0');
}
