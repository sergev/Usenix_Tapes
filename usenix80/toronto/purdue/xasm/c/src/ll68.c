#
	extern	fout;
/*	Linking Loader for 6800

	Peter D Hallenbeck

	(c) copyright Feb 1978

	If progam tosses cookies without  explanation and you think
	you are using > NUMSYM worth of total charactors for all your
	external moduale names, try increasing 'NUMSYM'.  All other
	violations should be anounced to you.
*/
#define	NUMMOD	200
#define NUMSYM	2000
#define NMXREF	500
#define NUMGLOB 200
	int wlist[NUMMOD];	/* want list	*/
	char wwlist[NUMSYM];
	int hlist[NUMMOD];	/* have list	*/
	char hhlist[NUMSYM];
	int glist[NUMGLOB];	/* global symbol list	*/
	char gglist[NUMGLOB*8];
	int stlist[NUMMOD];	/* start address list	*/
	int flist[NUMMOD];	/* finish address list	*/
	int elist[NUMMOD];	/* entry point list	*/
	int rlist[NUMMOD];	/* ROM list. 1 = rom.	*/
	int galist[NUMGLOB];	/* global variable adress list	*/
	int rpc;		/* relative pc		*/
	int pc;			/* progam counter	*/
	int bc;			/* byte counter		*/
	int reloc;		/* relocation bits	*/
	int xloc[NMXREF];	/* external location data	*/
	int rloc[NMXREF];	/* external location variable	*/
	int wln;		/* want list number	*/
	int gli;		/* global list index	*/
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
	int ii,index,curmod,mask,high,low,dirpath,startadd,havedir,tmp;

	fout = dup(1);
	pcost = 1;
	mixerup(wlist,wwlist);
	mixerup(hlist,hhlist);
	mixerup(glist,gglist);
	for(i = 0;i != NUMMOD;i++)rlist[i] = 0; /* clear rom list */
	cement(argv[1],hlist);
	startadd = 256;	havedir = 0;	gli = 1;
	if((argc >= 3)&&(islet(argv[2][0])== 0)){
		startadd = basin(argv[2],16);
		if(argc == 4){
			havedir = 1;
			dirpath = 3;
			}
		    else havedir = 0;
			}
	    else {
		if(argc == 3){
			havedir = 1;
			dirpath = 2;
			}
		}
	stlist[1] = startadd;
	copystr(opener,argv[1]);
	cats(opener,".rel");
	fdi = open(opener,0);
	if(fdi == -1){printf("Can't find '%s'.\n",opener);flush();return;}
	ibuffc = 0;	obuffc = 0;	obuffp = obuff;
	while((c = get())!= '\0'){
	    if(c == 'S'){
		c = get();
		switch(c){
		    case '3':		/* load modual	*/
			p = s;
			while(((c= get())!= '\0')&&(c != '\n'))*p++ = c;
			*p = '\0';
			if(llu(s,wlist)== -1)cement(s,wlist);
			break;
		    case '4':		/* global */
			wln = getwrd() + startadd;
			p = s;
			while(((c= get())!= '\0')&&(c != '\n'))*p++ = c;
			*p = '\0';
			if(llu(s,glist) == -1)cement(s,glist);
			if(s[0] == '_')wln =- startadd;
			galist[gli++] = wln;
			break;
		    case '5':
			printf("Can not have ROM modual as main program.\n");
			break;
		    case '6':
			elist[1] = getwrd() + startadd;
			break;
		    case '7':
			getwrd();
			p = s;
			while(((c= get())!= '\0')&&(c != '\n'))*p++ = c;
			*p = '\0';
			if(llu(s,wlist)== -1)cement(s,wlist);
			break;
		    case '8':
			pc = getwrd() + startadd;
			flist[1] = pc - 1;
			break;
		    }
		}
	    }
	close(fdi);		/* now have mains' externals in list */
	flush();
	index = 2;	ibuffc = 0;	wln = 1;
  next:	while(wlist[wln] != -1){	/* get info on all externals */
	    if(havedir){
		copystr(s,argv[dirpath]);
		cats(s,"/");
		}
	      else s[0] = '\0';
	    cats(s,wlist[wln]);
	    cats(s,".rel");
	    i = open(s,0);
	    if(i == -1){
		    wln++;
		    goto next;
		    }
	    cement(wlist[wln],hlist);
	    chisel(wln,wlist);
	    fdi = i;
	    stlist[index] = pc;
globomb:    while((c = get()) != '\0'){
		if(c == 'S'){
		    c = get();
		    switch(c){
			case '3':	/* loadmodual		*/
			    p = s;
			    while(((c=get())!= '\0')&&(c != '\n'))*p++ = c;
			    *p = '\0';
			    if((llu(s,wlist)== -1)&&(llu(s,hlist)== -1))
				cement(s,wlist);
			    break;
			case '4':	/* global	*/
			    tmp = getwrd();
			    p = s;
			    while(((c=get())!= '\0')&&(c != '\n'))*p++ = c;
			    *p = '\0';
			    if(llu(s,glist)!= -1){
				printf("Double-defined global: %s.\n",s);
				goto globomb;
				}
			    cement(s,glist);
			    if(s[0] != '_')tmp =+ stlist[index];
			    galist[gli++] = tmp;
			    if((tmp = llu(s,wlist))!= -1)chisel(tmp,wlist);
			    break;
			case '5':	/* rom modual */
			    elist[index] = getwrd();
			    flist[index] = elist[index];
			    stlist[index] = elist[index];
			    rlist[index] = 1;
			    break;
			case '6':	/* ram entry point	*/
			    elist[index] = getwrd()+ stlist[index];
			    break;
			case '7':	/* external referance	*/
			    getwrd();
			    p = s;
			    while(((c = get()) != '\0')&&(c != '\n'))*p++ = c;
			    *p = '\0';
			    if((llu(s,hlist)== -1)&&(llu(s,wlist)== -1))
				cement(s,wlist);
			    break;
			case '8':	/* lenght of modual	*/
			    pc =+ getwrd();
			    flist[index] = pc - 1;
			    break;
			}
		    }
		}
	    wln = 1;
	    index++;
	    if(index == NUMMOD){
		printf("Too many moduales.  'NUMMOD' must be increased.\n");
		index--;
		chisel(index,hlist);
		}
	    close(fdi);	ibuffc = 0;
	    flush();
	    }
	if(wln != 1){
		wln = 1;
		while(wlist[wln] != -1){
			if(llu(wlist[wln],glist) == -1){
				printf("Can't find '%s.rel'.\n",wlist[wln]);
				}
			wln++;
			}
		}

/* have all data on routines in the hlist and other tables	*/

	copystr(s,argv[1]);	cats(s,".out");
	fdo = creat(s,0604);
	curmod = 1;	pc = 0;		rpc = 0;	fb2 = 0;
 loop2:	while(hlist[curmod] != -1){	/* relocate all moduals	*/
	    if((curmod != 1)&&(havedir)){
		copystr(s,argv[dirpath]);
		cats(s,"/");
		}
	      else s[0] = '\0';
	    cats(s,hlist[curmod]);	cats(s,".rel");
	    fdi = open(s,0);
	    if(fdi == -1){
		printf("Can't find '%s'.\n",s);
		curmod++;
		goto loop2;
		}
	    j = 0;
	    while((c = get())!= '\0'){	/* get mods. external referances*/
		if(c == 'S'){
		    if(get() == '7'){
			xloc[j] = getwrd();
			p = ss;
			while(((c= get())!= '\n')&&(c!= '\0'))*p++ = c;
			*p = '\0';
			i = llu(ss,hlist);
			if(i == -1){
			    i = llu(ss,glist);
			    if(i == -1){
				printf("Table error on %s.\n",ss);
				}
			      else rloc[j++] = galist[i];
			    }
			  else {
			    rloc[j++] = elist[i];
			    }
			if(j == NMXREF){
			    printf("Too many externals in moduale '");
			    printf("%s'.  Increase 'NMXREF.\n",s);
			    j--;
			    }
			}
		    }
		}
	    close(fdi);		xloc[j] = -1;
	    flush();	ibuffc == 0;

/* have read this moduales externals and can now relocate	*/

	    index = 0;
	    fdi = open(s,0);
	    if(fdi == -1)printf("Can't find '%s'\n",s);
	    rpc = 0;
	    high = stlist[curmod];
	    while((c = get()) != '\0'){		/* read object mod. */
		if(c == 'S'){
		    if(get() == '2'){
			bc = getbyte() - 5;
			reloc = getwrd();
			mask = 1 << (bc -1);
			rpc = getwrd();
			while(bc > 0){
			    pc = stlist[curmod] + rpc;
			    if((mask & reloc)||(xloc[index]== rpc)){
				ii = getwrd();
				if(mask & reloc){
				    ii =+ high;
				    }
				if(xloc[index] == rpc){
				    ii =+ rloc[index++];
				    }
				b1 = (ii >> 8) & 0377;
				prb();
				pc =+ 1;
				b1 = ii & 0377;
				prb();
				pc =+ 1;
				rpc =+ 2;
				bc = bc - 2;
				mask = (mask >> 2) & 077777;
				}
			      else {
				b1 = getbyte();
				prb();
				rpc =+ 1;
				bc =- 1;
				mask = (mask >> 1) & 077777;
				}
			    }
			}
		    }
		}
	    curmod++;
	    close(fdi);	ibuffc = 0;	flush();
	    }				/* while(hlist... done. */

/*	have relocated and dumped everything.			*/

	if(pcocnt)pdump();
	put('S');	put('9');	put('\n');
	fput();				/* flush & close buffer	*/

	/*	Dumpt out symbol table	*/
	copystr(opener,argv[1]);
	cats(opener,".rel");
	ibuffc = 0;	fdi = open(opener,0);
	if(fdi == -1){printf("Can't open %s.\n",opener);flush();return;}
	obuffp = obuff;	obuffc = 0;
	copystr(opener,argv[1]);	cats(opener,".sym");
	fdo = creat(opener,0604);
	if(fdo== -1){printf("Can't create '%s'.\n",opener);flush();return;}
	i = stlist[1];
	while((c = get()) != '\0'){	/* main mods symbols */
	    if(c == 'S'){
		if(get() == 'A'){
		    j = getwrd() + i;
		    c = get();
		    put('0');
		    if(c == '_'){	/* check for no re-location */
			putwrd(j - i);
			}
		      else putwrd(j);
		    put(':');
		    put(c);
		    while(put(get()) != '\n');
		    }
		}
	    }
	i = 1;
	while(hlist[i] != -1){		/* all mods' entry pnts */
		put('0');
		putwrd(elist[i]);
		put(':');
		p = hlist[i++];
		while(*p)put(*p++);
		put('\n');
		}
	i = 1;
	while(glist[i] != -1){		/* all global symbols */
		put('0');
		putwrd(galist[i]);
		put(':');
		p = glist[i++];
		while(*p)put(*p++);
		put('\n');
		}
	fput();
					/* symbol file done */

	printf("%s is %d bytes long ",argv[1],(pc - startadd + 1));
	if((pc - startadd) > 9){
		putchar('(');
		basout((pc - startadd + 1),16);
		printf(" hex bytes long)");
		}
	putchar('\n');
	i = 1;
	printf("Name\tfrom\tto\tentry\n");
	while(hlist[i] != -1){
	    if((i - 1)% 5 == 0){
		printf("+-------+-------+-------+----\n");
		}
	    printf("%s\t",hlist[i]);
	    if(rlist[i]){
		    printf("Rom\t");
		    printf("Modual\t");
		    }
		else {
		    basout(stlist[i],16);	putchar('\t');
		    basout(flist[i],16);	putchar('\t');
		    }
	    basout(elist[i++],16);	putchar('\n');
	    }
	printf("\nGlobals are:\n");
	i = 1;
	while(glist[i] != -1){
		printf("%s\t",glist[i]);
		basout(galist[i++],16);	putchar('\n');
		}
	if(glist[1] == -1)printf(" [ NONE ] \n");
	flush();
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

	c = get() - 060;
	if(c > 9)c =- ('A' - ':');
	i = (c << 4) & 0360;
	c = get() - 060;
	if(c > 9)c =- ('A' - ':');
	return(i + c);
}

getwrd()
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
	return(data);
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

	put('S');	put('1');
	i = 0;
	putbyte(pcocnt + 3);
	pcoch = pcocnt + 3;
	putwrd(pcobs);
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
