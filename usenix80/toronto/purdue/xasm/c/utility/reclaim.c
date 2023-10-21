	extern	fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	register char c;
	register i,j;

/*	This program reclaims the soucre from an assembler output.
	Good only for mas80 and mas68 series assemblers.	*/

	if(argc == 1){
		printf("Usage:  reclaim 'filename'\n");
		return;
		}
	if(argc != 2){
		printf("Too many arguments- only one file at a time.\n");
		return;
		}
	j = open(argv[1],0);
	if(j == -1){
		printf("Can't find %s.\n",argv[1]);
		return;
		}
	fin = j;	fout = dup(1);
lop:	i = 0;
	while((i++ < 19) && ((c = getchar()) != '\n') && (c != '\0'));
	if(i >= 19){
		putchar(c);
		while((c != '\n') && (c != '\0'))putchar(c = getchar());
		goto lop;
		}
	if(c == '\n'){
		putchar(c);
		goto lop;
		}
	flush();
}
