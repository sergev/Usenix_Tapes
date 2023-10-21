main(argc,argv)
	int argc;
	char **argv;
{
	register i,j;
	i = 1;
	while(argv[i][0]){
		j = basin(argv[i++],8);
		printf("%o octal is %d decimal and ",j,j);
		basout(j,16);
		printf(" hex");
		if((islet(j) <= 2)||(j == ' ')){
			printf(" and an ascii '%c'.\n",j);
			}
		  else printf(".\n");
		}
}
