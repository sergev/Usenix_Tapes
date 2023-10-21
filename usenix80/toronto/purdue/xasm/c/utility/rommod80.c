	extern fout;
main(argc,argv)
	int argc;	char **argv;
{
	register i,j,n;
	int fd;
	char name[30];

	if(argc != 3){
		printf("Usage: % rom modual_name entry_location\n");
		return;
		}
	n = basin(argv[2],16);
	copystr(name,argv[1]);
	cats(name,".rel");
	fd = creat(name,0604);
	if(fd == -1){
		printf("Can't create '%s'.\n",name);
		return;
		}
	fout = fd;
	printf(":0%s\n",argv[1]);
	printf(":5");
	j = 0170000;
	for(i = 3; i >= 0;--i){
		phdiget(((n & j) >> ( i * 4)) & 017);
		j =>> 4;
		j =& 07777;
		}
	printf("\n:9\n");
	flush();
	close(fd);
}

phdiget(num)
	int num;
{
	if(num > 9)num =+ 'A' - ':';
	num =+ '0';
	putchar(num);
}
