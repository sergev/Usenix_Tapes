mexp(body,arglist)
	char *body,**arglist;
{
	char line[80];
	register char *p,*k,c;
	int i;
	k = line;
	while(*body != '\0'){
		switch(c = *body++){
			case '\n':
				*k++ = '\n';
				*k = '\0';
				printf("%s",line);
				k = line;
				break;
			case '&':
				c = *body++;
				i = c - '0';
				p = arglist[i];
				while(*k++ = *p++);
				k --;
				break;
			default:
				*k++ = c;
			}
		}
}
