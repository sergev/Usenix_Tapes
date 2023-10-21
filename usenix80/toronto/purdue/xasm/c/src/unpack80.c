/*	8080 ':0' format cleaner

	Peter D Hallenbeck

	(c) copyright Feb 1978

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
	int idone;		/* 0 = input not done	*/


main(argc,argv)
	int argc;	char **argv;
{
	char opener[50],s[50],ss[50],*p;
	register i,j;	register c;
	int ii,index,curmod;

	pcost = 1;	pcocnt = 0;	pcoch = 0;
	if(argc == 1)copystr(opener,"i.out.pack");
	    else {
		copystr(opener,argv[1]);
		cats(opener,".pack");
		}
	fdi = open(opener,0);
	if(fdi == -1){printf("Can't find '%s'.\n",opener);return;}


	ibuffc = 0;	obuffc = 0;	obuffp = obuff;	idone = 0;
	if(argc == 1){
		copystr(s,"i.out.norm");
		}
	    else {
		copystr(s,argv[1]);
		cats(s,".norm");
		}
	fdo = creat(s,0604);
	if(fdo == -1){printf("Can't make '%s'.\n",s);return;}
	curmod = 0;	pc = 0;		fb2 = 0;

	while(i = get()){
		curmod++;
		if(idone){
			goto booboo;
			}
		pc = get() << 8;
		if(idone){curmod = 0;	goto booboo;}
		pc =+ get();
		if(idone){curmod = 0;	goto booboo;}
		while(i--){
		    b1 = get();
		    if(idone){curmod = 0;	goto booboo;}
		    prb();
		    pc++;
		    }
	    }
booboo:	if(pcocnt)pdump();
	put(':');
	for(i = 1;i != 11;i++)put('0');
	put('\n');
	fput();
	if(curmod)printf("'%s' cleaned and in '%s'.\n",opener,s);
	    else printf("Not binary file -'%s'\n",opener);
}

get()
{
	if(ibuffc == 0){
		ibuffc = read(fdi,&ibuff,512);
		if(ibuffc == 0)idone = 1;
		if(ibuffc == -1)idone =1;
		ibuffp = ibuff;
		}
	ibuffc--;
	return((*ibuffp++) & 0377);
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

	put(':');
	i = 0;
	putbyte(pcocnt);
	pcoch = pcocnt - 1;
	putwrd(pcobs);
	putbyte(0);
	pcoch =+ pcobs >> 8;
	pcoch =+ pcobs & 0377;
	while(pcocnt){
		putbyte(pcob[i]);
		pcocnt--;
		pcoch =+ pcob[i++];
		}
	putbyte(~pcoch);
	put('\n');
}
