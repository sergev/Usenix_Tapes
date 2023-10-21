repl(str, c1, c2)
register char *str;
char c1, c2;
{
register char c;
char *p1;

	p1 = str;
	while(c = *str){
		if(c == c1)
			*str = c2;
		str++;
	}
	return(p1);
}
