	extern	fin,fout;

main(argc,argv)
	int argc;	char **argv;
{
	register char c;
	register char *p;
	register i;
	char	buffer[200];
	int	lenofbuff;

/*	This program removes any spaceing (space or tab) charactors
	from a file that are at the ends of lines, i.e. all lines end
	on a printing charactor and a new-line.			     */

	if(argc != 1){
		i = open(argv[1],0);
		if(i == -1){
			printf("Can't read '%s',\n",argv[1]);
			return;
			}
		fin = i;
		}
	    else fin = dup(0);
	fout = dup(1);

	p = buffer;
	while(c = getchar()){
		*p++ = c;
		if((c == '\n')||(c == '\r')){
			p =- 2;
			while((*p == ' ') || (*p == '\t'))--p;
			p++;
			*p++ = c;
			*p = '\0';
			printf("%s",buffer);
			p = buffer;
			}
		}
	flush();
}
