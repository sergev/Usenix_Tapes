	extern	fin,fout;
main()
{
	char buff[100];
	register k;
	register char *i;
	register char c;
	fin = dup(0);	fout = dup(1);
  loop:	i = buff;
	*i = '\0';
	while((c = getchar())!= '\n'){
		if(c == '\0'){flush();	return;}
		*i++ = c;
		}
	*i++ = ' ';
	*i++ = '\0';
	if(lfs("login",buff))goto loop;
	if(buff[50] == '-')goto loop;
	printf("%s\n",buff);
	goto loop;
}
