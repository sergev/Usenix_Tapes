	extern	fin,fout;
main()
{
	register i,j;
	register char c;

	fin = dup(0);	fout = dup(1);
	i = 1;
loop:	while((c = getchar()) != '\0'){
		if(c == '\n'){
			if((i % 2) == 0){
				putchar(c);
				}
			    else {
				printf("         	  ");
				}
			i++;
			goto loop;
			}
		putchar(c);
		}
	if((i % 2) == 0)putchar('\n');
	flush();
}
