main(argc,argv)
	int argc;
	char *argv[]; {
	int pp,p,exponent,number,newnum;
	if(argc!=1)
	{
	if(argc==2)
	{
	if((argv[1][0]>=48)&&(argv[1][0]<=57))
	{
	p=0;
	while((argv[1][p])!=(0))++p;
	number=0;exponent=1;p=p-1;
		while(p!=(-1)){
			number=number+(((argv[1][p--])-48)*exponent);
			exponent=exponent*8;}
	if((number<32)||(number>127)) {
		if(number>127)printf("Number too large.\n");
		else if(number==10)printf("12 is an ascii '^j'. (line feed)\n");
		else if(number==13)printf("15 is an ascii '^m'. (carrige return)\n");
		else if(number==7)printf("7 is an ascii '^g'. (bell)\n");
		else if(number==9)printf("11 is an ascii '^i'. (tab)\n");
		else {
			newnum=number+64;
			printf("%o is an ascii '^%c'.\n",number,newnum);
			}
		} else if(number==127) printf("177 is an ascii 'rubout'.\n");
		else if(number==32) printf("40 is an ascii ' '. (space)\n");
		else printf("%o is an ascii '%c'.\n",number,number);
	}
	else printf("Numbers only.\n");
	}
	else printf("One argument only, please.\n");
	}
	else printf("No argument.\n");
}
