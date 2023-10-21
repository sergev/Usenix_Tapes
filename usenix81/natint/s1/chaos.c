main(){
	register int n;
	register char *s;
	char buf[5];

	buf[0]= 033;
	buf[1]= '=';

	for(;;){
		n=rand();
		buf[2]= ' '+ (n>>8)%24;
		n=rand();
		buf[3]= ' '+ (n>>8)%80;
		if(buf[2]==' '+23 && buf[3]==' '+79) continue;
		n=rand();
		buf[4]= (n>>8)&0377;
		if(buf[4]<' ') buf[4]= ' ';
		write(1,buf,5);
	}       } 
