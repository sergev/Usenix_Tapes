	extern	fin;
main(argc,argv)
	int argc;	char **argv;
{
	register lcount;
	register char c;
	int ccount,oflag;
	int lncount;

	if(argc == 2){
		lcount = open(argv[1],0);
		if(lcount == -1){
			printf("Can't find '%s'.\n",argv[1]);
			return;
			}
		fin = lcount;
		}
	    else fin = dup(0);
	lcount = 0;
	ccount = 0;
	lncount = 0;
more:	while(c = getchar()){
		lncount++;
		if(c == '\n')goto more;
		if((c != '*')&&(c!= ';')&&(c!= '\t')&&(c!= ' ')){
			lcount++;	ccount++;
			oflag = 1;
			}
		   else oflag = 0;
		while(((c = getchar())!= '\n') && c){
			if(oflag){
				if(island(c) == 1)ccount++;
				   else oflag = 0;
				}
			}
		}
	printf("There are %d lines.\n",lncount);
	printf("There are %d lables.\n",lcount);
	printf("There are %d charactors in the lables.\n",ccount);
}

island(ch)
	char ch;
{
	if(((ch >= 'A')&&(ch <= 'Z'))||((ch >= 'a')&&(ch <= 'z'))) return (1);
	if((ch >= '0')&&(ch <= '9')) return(1);
	if(ch == '_')return(1);
	return(0);
}
