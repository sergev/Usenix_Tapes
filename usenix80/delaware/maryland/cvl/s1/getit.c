char    buff[2048];
main(argc, argv)
char *argv[];
{
	register  tape, size, i;

	if(argc != 2) {
		write(2,"\n\tUSAGE --- getit rowsize > picfile\n\n",37);
		return;
	}

	if((size = atoi(argv[1])) <= 0) {
		write(2,"\n\tUSAGE --- getit rowsize > picfile\n\n",37);
		return;
	}

	if((tape = open("/dev/srmt4",0)) < 0) {
		write(2,"Error during tape open\n",23);
		return;
	}

	for(;;) {
		if((i = read(tape,buff,size)) <= 0) {
			write(2,"\n\07BAD READ\n\n",12);
			return;
		}
		write(2,"\07WRITE\n",7);
		write(1,buff,i);
	}
}
