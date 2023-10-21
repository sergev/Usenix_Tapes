	int fd;
main(argc,argv)
	int	argc;	char **argv;
{
	register char c;
	register i,j;
	char	buff[22],first,second,third;
	int	data[10];

	fd = creat(argv[1],0600);
	if(fd == -1){
		printf("Can't open '%s'.\n",argv[1]);
		return;
		}
	c = 1;
    while(c){
	scanin(buff);
	first = buff[0];	second = buff[1];	third = buff[2];
	if(buff[3] == 'a'){
		second = second | 0200;
		}
	if(buff[3] == 'b'){
		third = third | 0200;
		}
	for(i = 0;i != 5;i++){
		c = scanin(buff);
		j = basin(buff,16);
		data[i] = j;
		}
	plops("	db	");
	dwr('\'');	dwr(first); dwr('\'');
	dwr(',');	basesp((second & 0377),buff,10);
	plops(buff);	dwr(',');
	basesp((third & 0377),buff,10);	plops(buff);
	for(i = 0;i != 5;i++){
		basesp(data[i],buff,10);
		dwr(',');
		plops(buff);
		}
	dwr('\n');
	}
}
dwr(thing)
	char thing;
{
	write(fd,&thing,1);
}
plops(string)
	char	*string;
{
	while(*string)write(fd,string++,1);
}
