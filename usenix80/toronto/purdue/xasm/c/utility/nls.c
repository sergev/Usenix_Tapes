	extern	fin,fout;
main()
{
	register i,j;
	register char c;

	fin = dup(0);	fout = dup(1);
	i = 1;	j = 0;
loop:	while((c = getchar()) != '\0'){
		j++;
		if(c == '\n'){
			if((i % 4) == 0){
				putchar(c);
				}
			    else {
				while(j++ != 19){
					putchar(' ');
					}
				}
			i++;
			j = 0;
			goto loop;
			}
		putchar(c);
		}
	if((i % 4) != 1)putchar('\n');
	flush();
}
