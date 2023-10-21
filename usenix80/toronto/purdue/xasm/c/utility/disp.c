	extern fin;
main(argc,argv)
	int argc;	char **argv;
{
	register char c;
	register i;
	register lc;

	if(argc == 2){
		i = open(argv[1],0);
		if(i == -1){
			printf("Can't find '%s'.\n",argv[1]);
			return;
			}
		fin = i;
		}

	lc = 8;
	printf("------------\n");
	while(putchar(c = getchar())){
		if(c == '\n')i = 1;
		    else i = 0;
		while(i < 8){
			putchar(c = getchar());
			if(c == '\n')i++;
			}
			printf("------------%d\n",lc);
			lc =+ 8;
		}
}
