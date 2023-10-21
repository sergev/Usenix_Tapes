char err[] "dup: wrong number of args";

main(argc,argv)
char **argv;
{
	register i, j;

	if(argc != 3) write(2,err,sizeof err -1);
	else {
		i = atoi(argv[1]);
		for(j=0;j<i;j++) printf("%s\n",argv[2]);
	}
}
