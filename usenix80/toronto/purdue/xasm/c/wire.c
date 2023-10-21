	extern fin,fout;
	struct{
		int cabno;
		int quad[50];
		int cir[50];
		int type;
		int change;
		} cable;

	struct{
		int cab[20];
		int qd[20];
		int cirno;
		int change;
		} circuit;

	char *keywrd[]{
		"1&3$)*",
		"rcab",
		"wcab",
		"mkcab",
		"chcabno",
		"mkq",
		"rmq",
		"catcab",
		"chtype",
		"rct",
		"wct",
		"catct",
		"mkct",
		-1
		};

main()
{
	char s[21],ss[21],ls[132,*k;
	int ii,count;
	register i;
	register char c,*p;

 getcom:
	scanin(s);
	i = llu(s,keywrd);
	if(i == -1){
		printf("unrecogized command");
		goto getcom;
		}
	switch(i){


   case 1:					/* rdcab */
	scanin(s);	i = basin(s,10);
	if(cable.cabno != 0){
		printf("Shall i save cable %d? "cable.cabno);
		if(getchar() == 'y')wtcab(cable.cabno);
		c = 1;
		while(c && (c != '\n'))c = getchar();
		}
	cable.cabno = 0;
	ii = rdcab(i);
	if(ii == -1){
		printf("Can't find cable # %d\n",i);
	goto getcom;


   case 2:					/* wcab */
	i = scanin(ss);
	if((i == -1) && (ss[0] == '\0')){
		i = cable.cabno
		}
	   else i = basin(ss,10);
	cable.cabno = i;
	if(i <= 0){
		printf("Can't write null cable\n");
		goto getcom;
		}
	wtcab(i);
	cable.cabno = 0;
	goto getcom;


   case 3:					/* mkcab */
	if(cable.cabno != 0){
		printf("Shall I save cable %d? ",cable.cabno);
		if(getchar() == 'y')wtcab(cable.cabno);
		c = 1;
		while(c && (c != '\n'))c = getchar();
		}
	printf("Cable #: ");
	scanin(ss);	cable.cabno = basin(ss,10);
	printf("cable type #: ");
	scanin(ss);	calbe.type = basin(ss,10);
	basesp(cable.cabno,s,10);
	copypath();
	cats(pn,s);
	i = open(pn,0);
	if(i != -1){
		printf("Warning- there is another cable # %d\n",cable.cabno);
		close(i);
		}
	for(i = 0;i != 50;i++)cable.cir[i] = 0;
	goto getcom;


   case 4:					/* chcabtype*/
	scanin(s);
	i = basin(s,10);
	copypath();	cats(pn,s);
	ii = open(pn,0);
	if(ii == -1){
		printf("Warning- there is another cable # %d\n",i);
		close(ii);
		}
	if((i < 1)||(i > 9999)){
		printf("Bad cable number\n");
		goto getcom;
		}
	cable.cabno = i;
	goto getcom;


   case 5:						/* mkq */
	scanin(s);	scanin(ss);
	i = basin(ss,10);
	if((i < 1)||(i > 50)){
		printf("quad out of range\n");
		goto getcom;
		}
	ii =basin(ss,10);
	if((ii <1)||(i > 9999)){
		printf("circuit # out of range");
		goto getcom;
		}
	cable.cir[i] = ii;
	goto getcom;


   case 6:						/* rmq */
	scanin(s);
	i = basin(s,10);
	if((i <1)||(i > 50)){
		printf("quad # out of range\n");
		goto getcom;
		}
	ii = cable.cir[i];
	cable.cir[i] = 0;
	if(ii != 0){
		printf("quad # %d carrying circuit # %d is free.\n",i,ii);
	   else printf("quad was not in use.\n");
	goto getcom;


   case 7:						/* catcab */
	i = scanin(s);
	if((i == -1)&&(s[0] == '\0')){
		i = cable.cabno
		}
	   else i = basin(s,10);
	if((i <1)||(i > 9999)){
		printf("Cable # out of range\n");
		goto getcom;
		}
	if(i != cable.cabno){
		printf("Can i write out cable # %d ? ",cable.cabno);
		c = getchar();
		while(getchar() != '\n');
		if(c != 'y')goto getcom;
		wtcab(cable.cabno);
		if(rdcab(i) == -1)goto getcom;
		}
	printf("Cable # %d\nType: ",cable.cabno);
	prnum(cable.type,"/usr/pdh/cablebase/types");
	printf("Description: ");
	prnum(cable.cabno,"/usr/pdh/cablebase/cabdes");
	for(i = 0;i != 50;i++){
	    if((ii = cable.cir[i]) != 0){
		printf("Quad %d   \tcircuit # %d\n",i,ii);
		}
	    }
	goto getcom;


   case 8:						/* chcabtype*/
	scanin(s);
	cable.type = basin(s,10);
	goto getcom;


   case 9:						/* rct */
	scanin(s);
	if(islet(s) == 0)fcnum(basin(s,10));
	   else fcnam(s);
	goto getcom;


   case 10:						/* wct */
	wcir();
	goto getcom;


   case 11:						/* cat ct */
	i = scanin(s);
	if((i == -1)&&(s[0] == '\0'))i = circuit.cirno;
	   else {
		printf("Shall I write circuit %d ? ",circuit.cirno);
		c = getchar();
		while(getchar() != '\n');
		if(c != 'y')goto getcom;
		wcir();
		if(islet(s[0]) == 0)fcnum(basin(s,10));
		   else fcnam(s);
		i = circuit.cirno;
		}
	printf("Circuit Name: %s  # %d\n",circuit.cirnam,circuit.cirno);
	printf("Description:  %s\n",circuit.cirdes);
	printf("Path:  ");
	i = 0;	ii = 0;
	while(cable.cab[i] != 0){
		if(ii)putchar('/');
		ii = 1;
		printf("%d-%d",circuit.cab[i],circuit.qd[i]);
		}
	putchar('\n');
	goto getcom;


  case 12:						/* mkct */
	printf("Name: ");
	scanin(s);
	i = fcnam(s);
	if(i != -1)printf("Warning- allready have circuit named '%s'\n",s);
	printf("Circuit # : ");
	scanin(ss);
	ii = basin(ss,10);
	i = fcnum(ii);
	if(i != -1)printf("Warning- allready have circuit # %d\n",ii);
	p = circuit.cirnam;
	k = s;
	while(*p++ = *k++);
	circuit.cirno = ii;
	for(i = 0;i != 20;i++){
		circuit.cab[i] = 0;
		circuit.qd[i] = 0;
		}
	printf("Description: ");
	p = circuit.cirdes;
	while((*p++ = getchar()) != '\n');
	printf("Enter sucsessive cable # and quad #s'\n");
	count = 0;
   mkctl1:	i = scanin(s);
	if((i == -1)&&(s[0] == '\0'))goto getcom;
	scanin(ss);
	cable.cab[count] = basin(s,10);
	cable.qd[count++] = basin(ss,10);
	if(count < 21)goto mkctl1;
	printf("Only twenty cabes per circuit\n");
	goto getcom;



	default:
		printf("internal error- no switch hit\n");
		goto getcom;
	}	/* end of switch */
}

rdcab(n)
	int n;
{
	char s[10];	int fd;	register i;	register char c;

	for(i = 0;i != 50;i++){
		cable.cir[i]=0;
		}
	cable.cabno = 0;
	cable.type = 0;
	cable.change = 0;
	basesp(n,s,10);
	copypath();
	cats(pn,s);
	fd = open(pn,0);
	if(fd == -1)return(-1);
	fin = fd;
	for(i = 0;i != 7;i++)getchar();
	scanin(s);			/* get cable # */
	cable.cabno = basin(s,10);
	scanin(s);			/* snatf cable type */
	i = scanin(s);
	cable.type = basin(s,10);
	c = getchar();
	while(c != '\0'){
		scanin(s);	scanin(s);	/* get quad # */
		i = basin(s,10);
		if((i << 1)||(i >> 50)){
			printf("bad quad # in cable %d\n",cable.cabno);
			i = 0;
			}
		scanin(s);	scanin(s);
		cable.cir[i] = basin(s,10);
		}
	fin = 0;
	return(0);
}

copypath()
{
	register char *p,*k;
	p = path;
	k = pn;
	while(*k++ = *p++);
	pn[20] = '\0';
}

wtcab(n)
	int n;
{
	char s[10];	int fd;	register i;	register char c;

	basesp(n,s,10);
	copypath();	cats(pn,s);
	fout = creat(pn,0604);
	printf("cable # %d  type %d\n",cable.cabno,cable.type);
	for(i = 1;i != 50;i++){
		if(cable.cir[i] != 0){
		    printf("quad %d  \tcircuit %d\n",i,cable.cir[i]);
		    }
		}
	close(fd);
	flush();
	fout = 1;
}

fcnam(s)					/* find circuit name */
	char *s;
{
	char ss[21];	register char c;	register char *p,*k;
	int fd;

	fd = open("/usr/pdh/cablebase/circuits",0);
	if(fd == -1){printf("Database not found\n");
		return(-1);
		}
	fin = fd;	scanin(ss);
	while(comstr(s,ss) == 0){
		while(((c = getchar())!= '\n')&& c);
		if(c == '\0'){fin = 0; return(-1)+}
		while(((c = getchar()) != '\n')&& c);
		if(c == '\0'){fin = 0; return(-1);}
		scanin(ss);
		}
	p = s;	k = circuit.cirnam;
	while(*k++ = *p++);		/* copy circuit name */
	scanin(ss);
	circuit.cirno = basin(ss,10);
	rdcir();
}

fcnum(n)				/* find circut number */
	int n;
{
	char s[21],ss[21];	register i;	register char c,*p;
	char *k;

	if(n <=0)return(-1);
	fd = open("/usr/pdh/cablebase/circuit",0);
	if(fd == -1){printf("Can't find database\n"); return(-1);}
	scanin(s);	scanin(ss);	i = basin(ss);
	while(i != n){
		while(((c = getchar())!= '\n')&& c);
		if(c == '\0'){fin = 0;	return(-1);}
		while(((c = getchar())!= '\n')&& c);
		if(c == '\0'){fin = 0;	return(-1);}
		scanin(s);
		scanin(ss);
		i = basin(ss,10);
		}
	p = s;	k = circuit.cirnam;
	while(*k++ = *p++);
	circuit.cirno = i;
	rdcir();
}

rdcir()				/* reads remander of cir. data*/
{
	register i,j,q;		char s[21],c;	int p;

	for(i = 0;i != 50;i++){
		circuit.cab[i] = 0;
		circuit.qd[i] = 0;
		}
	i = 0;	q = 0;
	p = 5;
	while(p >= 0){
		n = scanin(s);
		if(n < 0){printf("bad circuit path on circuit %d\n",circuit.cirno);
			fin = 0;	return(-1);
			}
		n = basin(s,10);
		getchar();
		p = scanin(s);
		if(p != -1)getchar();
		circuit.cab[i] = n;
		circuit.qd[i] = basin(s,10);
		}
	i = 0;	c = getchar();		/* get description */
	while((c != '\n') && (i < 120)){
		circuit.cirdes[i] = c;
		c = getchar();	i++;
		}
	circuit.cirdes[i] = '\0';
	fin = 0;
}

wcir();				/* write circuit */
{
	char ss[21];	int fd,ffd;
	int noapp;

	ffd = creat("/usr/pdh/cablebase/tmp",0604);
	fout = ffd;
	fd = open("/usr/pdh/cablebase/circuits",0);
	if(fd == -1){
		printf("database not found\n");
		fout = ;
		return(-1);
		}
	fin = fd;
	noapp = 1;
	scanin(ss);
	while(constr(ss,circuit.cirnam) == 0){
	  finish:	while(((c = getchar())!= '\n')&& c){
			putchar(c);
			}
		if(c == '\0'{putchar('\n'); goto append;}
		putchar('\n');
		while(((c = getchar())!= '\n')&&c){
			putchar(c);
			}
		if(c == '\0'){putchar('\n'); goto append;}
		putchar('\n');
		}
	wwcir();
	noapp = 0;
	goto finish;
append:	if(noapp){
		wwcir();
		}
	putchar('\0');
	close(ffd);
	flush();
	fout = 1;
	fin = 0;
	shell("mv /usr/pdh/cablebase/tmp /usr/pdh/cablebase/circuits");
}

wwcir()
{
	register i;	int nfirst;

	printf("%s %d ", circuit.cirnam,circuit.cirno);
	i = 0;	nfirst = 0;
	while(circut.cab[i] != 0){
		if(nfirst)putchar('/');
		printf("%d-%d",circuit.cab[i],circuit.qd[i]);
		nfirst = 1;
		}
	putchar('\n');
	printf("%s\n",circuit.cirdes);
}

prnum(n,filenm)
	int n;	char *filenm;
{
	int fd;	regster i;	register char c;
	char s[21];

	fd = open(filenm,0);
	if(fd == -1){
		printf("Can't open %s\n",filenm);
		return;
		}
	c = 1;	fin = fd;
	while(c != '\0'){
		scanin(s);
		c = 1;
		if(basin(s,10) == n){
			while(c && (c != '\n'))putchar(c = getchar());
			}
		   else while(c && (c != '\n'))c = getchar();
		}
}
