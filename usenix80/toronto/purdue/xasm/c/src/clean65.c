/*	6502 ';' format cleaner

	Peter D Hallenbeck

	(c) copyright July 1978

*/
	int pc;			/* progam counter	*/
	int bc;			/* byte counter		*/
	int fdi;		/* input file descriptor	*/
	int fdo;		/* output file descriptor	*/
	int ibuff[513];		/* input buffer		*/
	int ibuffc;		/* input buffer counter	*/
	char *ibuffp;		/* input buffer pointer	*/
	int obuff[513];		/* output buffer	*/
	int obuffc;		/* output buffer counter	*/
	char *obuffp;		/* output buffer pointer	*/
	int pcocnt;		/* counter		*/
	int pcolpc;		/* last pc		*/
	int pcoch;		/* checksum		*/
	int pcob[30];		/* buffer		*/
	int pcobs;		/* buffer start location*/
	int pcost;		/* start-up flag	*/
	int b1,b2;		/* punchout bytes	*/
	int fb2;		/* 2nd byte flag	*/


main(argc,argv)
	int argc;	char **argv;
{
	char opener[50],s[50],ss[50],*p;
	register i,j;	register char c;
	int ii,index,curmod,;

	pcost = 1;	pcocnt = 0;	pcoch = 0;
	if(argc == 1){
		copystr(opener,"mt.out");
		}
	    else copystr(opener,argv[1]);
	fdi = open(opener,0);
	if(fdi == -1){printf("Can't find '%s'.\n",opener);return;}


	ibuffc = 0;	obuffc = 0;	obuffp = obuff;
	copystr(s,opener);	cats(s,".clean");
	fdo = creat(s,0604);
	if(fdo == -1){printf("Can't make '%s'.\n",s);return;}
	curmod = 0;	pc = 0;		fb2 = 0;

	while(c = get()){
	    if(c == ';'){
		curmod++;
		i = getbyte();
		if(i == -1){curmod = 0;	goto booboo;}
		if(i == 0)goto booboo;
		pc = getword();
		while(i--){
		    b1 = getbyte();
		    if(b1 == -1){curmod = 0;	goto booboo;}
		    prb();
		    pc++;
		    }
	        }
	    }
booboo:	if(pcocnt)pdump();
	put(';');	put('0');	put('0');
	put('\n');
	fput();
	if(curmod)printf("'%s' cleaned and in '%s'.\n",opener,s);
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

put(data)
	char data;
{
	if(obuffc == 512){
		obuffc = write(fdo,&obuff,512);
		obuffc = 0;	obuffp = obuff;
		}
	*obuffp++ = data;
	obuffc++;
}

putbyte(data)
	int data;
{
	register i;
	i = (data >> 4) & 017;	i =+ '0';
	if(i > '9')i =+ ('A' - ':');
	put(i);
	i = data & 017;	i=+ '0';
	if(i > '9')i =+ ('A' - ':');
	put(i);
}

putwrd(data)
	int data;
{
	putbyte((data >> 8) & 0377);
	putbyte(data & 0377);
}

fput()
{
	obuffc = write(fdo,&obuff,obuffc);
	close(fdo);
	flush();
}

prb()
{
	if(pcost){pcost = 0;	pcolpc = pc;}
 prbq:	if(pcocnt == 0){
		pcolpc = pc;
		pcobs = pc;
		}
	if(pc != pcolpc){
		pdump();
		goto prbq;
		}
	pcob[pcocnt++] = b1;
	pcolpc =+ 1;
	if(pcocnt > 25)pdump();
}

pdump()
{
	register i;

	put(';');
	i = 0;
	putbyte(pcocnt);
	pcoch = pcocnt;
	putwrd(pcobs);
	pcoch =+ pcobs >> 8;
	pcoch =+ pcobs & 0377;
	while(pcocnt){
		putbyte(pcob[i]);
		pcocnt--;
		pcoch =+ pcob[i++];
		}
	putwrd(pcoch);
	put('\n');
}
