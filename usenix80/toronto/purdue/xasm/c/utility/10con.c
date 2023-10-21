main(argc,argv)
	int argc;
	char **argv;
{
	register i,j;
	i = 1;
	while(argv[i][0]){
		j = basin(argv[i++],10);
		printf("%d decimal is %o octal and ",j,j);
		basout(j,16);
		printf(" hex");
		if((islet(j) <= 2)||(j == ' ')){
			printf(" and an ascii '%c'.\n",j);
			}
		  else printf(".\n");
		}
}
