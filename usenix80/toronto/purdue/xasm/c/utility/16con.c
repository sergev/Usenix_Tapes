main(argc,argv)
	int argc;
	char **argv;
{
	register i,j;
	i = 1;
	while(argv[i][0]){
		j = basin(argv[i++],16);
		basout(j,16);
		printf(" hex is %d decimal and %o octal",j,j);
		if((islet(j) <= 2)||(j == ' ')){
			printf(" and an ascii '%c'\n",j);
			}
		  else printf(".\n");
		}
}
