	extern	fin,fout;

#define	MAXFILL	140	/* max. number of fill chars	*/
	char *speed[]{
		"#Dfw45x",
		"9600",
		"4800",
		"2400",
		"1200",
		"600",
		"300",
		-1
		};
main(argc,argv)
	int argc;	char **argv;
{
	register char c;
	register i,number;
	int dtime,fp,divam,mfill;
	char	openers[20];

	if(argc == 1){
		printf("Usage: sdl [terinal baud rate] filename\n");
		return;
		}
	fp = 1;	divam = 1; mfill = MAXFILL;
	if(argc == 3){
		fp++;
		divam = llu(argv[1],speed);
		if(divam == -1)divam = 1;
		}
	mfill =/ divam;
	copystr(openers,argv[fp]);
	cats(openers,".sym");
	fin = open(openers,0);
	if(fin == -1){
		printf("Can't find '%s'.\n",openers);
		return;
		}
	sleep(3);	/* give time to throw switch */
	fout = dup(1);

	number = 2;
	while(c = getchar()){
		if(c == '\n'){
			putchar(' ');
			dtime = number/divam;
			if(dtime > mfill)dtime = mfill;
			for(i = 0;i != dtime;i++)putchar(1);
			number++;
			}
		    else if(c == ':'){
			putchar(c);
			for(i = 0;i != 5;i++)putchar(1);
			}
		    else putchar(c);
		}
	flush();
}
