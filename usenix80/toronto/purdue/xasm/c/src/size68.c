/*	6800 'S1' format cleaner

	Peter D Hallenbeck

	(c) copyright Feb 1978

*/
	int pc;			/* progam counter	*/
	int bc;			/* byte counter		*/
	int fdi;		/* input file descriptor	*/
	int ibuff[513];		/* input buffer		*/
	int ibuffc;		/* input buffer counter	*/
	char *ibuffp;		/* input buffer pointer	*/
	int bcount;		/* bytecount		*/
	int b1,b2;		/* punchout bytes	*/
	int fb2;		/* 2nd byte flag	*/


main(argc,argv)
	int argc;	char **argv;
{
	char opener[50],s[50],ss[50],*p;
	register i,j;	register char c;
	int ii,index,curmod,;

	if(argc == 1){
		copystr(opener,"m.out");
		}
	    else copystr(opener,argv[1]);
	fdi = open(opener,0);
	if(fdi == -1){printf("Can't find '%s'.\n",opener);return;}


	ibuffc = 0;
	curmod = 0;	pc = 0;		fb2 = 0;

	while(c = get()){
	    if(c == 'S'){
	      if(get() == '1'){
		curmod++;
		i = getbyte();
		if(i == -1){curmod = 0;	goto booboo;}
		i =- 3;
		pc = getword();
		while(i--){
		    b1 = getbyte();
		    if(b1 == -1){curmod = 0;	goto booboo;}
		    bcount++;
		    pc++;
		    }
		}
	      }
	    }
booboo:
	if(curmod)printf("'%s' is %d (%x hex) bytes long.\r\n",opener,bcount,bcount);
	    else printf("Not binary file - '%s'\n",opener);
}

get()
{
	if(ibuffc == 0){
		ibuffc = read(fdi,&ibuff,512);
		if(ibuffc == 0)return('\0');
		if(ibuffc == -1)return('\0');
		ibuffp = ibuff;
		}
	ibuffc--;
	return((*ibuffp++) & 0177);
}

getbyte()
{
	register i;	register char c;

	c = get();
	if((c < '0') || ((c > '9') && (c < 'A'))){
		return(-1);
		}
	if(c > 'F')return(-1);
	c =- 060;
	if(c > 9)c =- ('A' - ':');
	i = (c << 4) & 0360;
	c = get();
	if((c < '0') || ((c > '9') && (c < 'A')))return(-1);
	if(c > 'F')return(-1);
	c =- 060;
	if(c > 9)c =- ('A' - ':');
	return(i + c);
}

getword()
{
	register i;
	i = getbyte() << 8;
	return(i + getbyte());
}

