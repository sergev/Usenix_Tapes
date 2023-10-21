	extern fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	register i;
	register char *p,c;
	char buff[140];

	if(argc == 2){
		i = open(argv[1],0);
		if(i == -1){
			printf("Can't find '%s'.\n",argv[1]);
			return;
			}
		fin = i;
		}
	    else fin = dup(0);
	fout = dup(1);
    while(c = getchar()){
	p = buff;
	*p++ = c;
	i = 1;
	if(c != '\n'){
		while((*p++ = getchar()) != '\n')i++;
		}
	*p = '\0';
	if(i > 19){
		if(((buff[15] == ' ')&&(buff[16] == ' '))&&
		((buff[13] != ' ')||(buff[14] != ' '))){
			if(buff[13] == ' ')buff[13] = '0';
			}
		    else {
			if(buff[16] != ' '){
				if(buff[13] == ' ')buff[13] = '0';
				if(buff[14] == ' ')buff[14] = '0';
				if(buff[15] == ' ')buff[15] = '0';
				}
			}
		}
	if(buff[8] != ' '){
		if(buff[5] == ' ')buff[5] = '0';
		if(buff[6] == ' ')buff[6] = '0';
		if(buff[7] == ' ')buff[7] = '0';
		}
	if(buff[11] != ' ')if(buff[10] == ' ')buff[10] = '0';
	printf("%s",buff);
	}
	flush();
}
