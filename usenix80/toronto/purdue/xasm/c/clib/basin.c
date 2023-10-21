basin(string,base)
	char *string;
	int base;
{
	char s[21];
	register number;
	register p;
	int i;
	register exponent;
	for(i=0;(s[i]= string[i])!='\0';i++);
	for(p=0;s[p]!='\0';p++);
	for(i=0;(s[i]!='\0');i++){
		if((s[i] == 'o') || (s[i] == 'O'))s[i] = '0';
		if((s[i] >= 'A') && (s[i] <= 'Z'))s[i] =+ 32;
		if(s[i] > '9')s[i]=
			(s[i]-('a'-':'));
		}
	number=0;exponent=1;p=p-1;
		while(p!=(-1)){
			number=number+(((s[p--])-48)*exponent);
			exponent=exponent*base;}
	return(number);
}
