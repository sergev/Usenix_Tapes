#define tabsize 25000
int tab[tabsize];
main(argc,argv)
	char **argv;
	{
	register int *ip,i,j;
	long count=0L;

	read(open("/usr/dict/hlista",0),tab,sizeof tab);
	for(ip=tab; ip<&tab[tabsize]; ip++){
		i = *ip;
		for(j=1; j; j<<=1)
			if(i&j)
				count++;
		}
	printf("count=%D\n",count);
	}
