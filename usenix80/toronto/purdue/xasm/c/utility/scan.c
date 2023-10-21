	extern	fin,fout;
main(argc,argv)
	int argc;
	char **argv;
{
	char buff[100];
	register k;
	register char *i;
	register char c;

	fout = dup(1);	fin = dup(0);
  loop:	i = buff;
	*i = '\0';
	while((c = getchar())!= '\n'){
		if(c == '\0'){flush();	return;}
		*i++ = c;
		}
	*i++ = ' ';
	*i++ = '\0';
	k = argc - 1;
	while(k){
		if(lfs(argv[k],buff)){
			printf("%s\n",buff);
			goto loop;
			}
		k--;
		}
	goto loop;
}
