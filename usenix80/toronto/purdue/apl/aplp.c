char c,ct,clast,lablst[500];
int lstp,lstpt;		/* points to lablst*/
int lincnt, labcnt,lstlst[50],linnum;
int labn[50];		/* line # of labels */
int obpt, obj[100];	/* object buffer  */
int labnum;		/* # of labels  */
int nt, m;

	/*   pass 1  */

main(argc, argp)
char **argp;
{

	if(argc > 1){
		close(0);
		if(open(argp[1], 0) != 0){
			printf("can't open input file: %s\n", argp[1]);
			exit(1);
		}
		if(argc > 2){
			close(1);
			if(creat(argp[2], 0644) != 1){
				printf("can't create output file: %s\n", argp[2]);
				exit(2);
			}
		}
	}

lstp=0;
labcnt=0;
	while ((c= getchar()) != '\n');	/* skip first line*/
	lincnt = 1;
	while ((c = getchar()) != '\0') {
		while (c == ' ') c= getchar ();
		lstpt = lstp;
		while ( ccheck(c) != 0) {
			lablst[lstpt++] = c;
			c=getchar ();
			}
		while (c == ' ') c=getchar();
		if (c == ':'){
			labn[labcnt]=lincnt;
			lstlst[labcnt]=lstp;
			lablst[lstpt++]='\0';
			lstp = lstpt;
			labcnt++;
			}
		while (c != '\n') c=getchar ();
		lincnt++;
	}
	labnum = labcnt;
	labn[++labcnt] = 10000;


	/*  pass 2 */

	seek(0,0,0);

	while (putchar(getc()) != '\n') {}	/* 1st line */
	lincnt = 1;
	lstpt = 0;
	nt = labn[lstpt];
	while( (c=getc()) != '\0') {
		if (nt < lincnt) nt = labn[++lstpt];
		if (nt == lincnt) {	/* get rid of label */
			while( getc() != ':' );
			c= getc();
			}

			/* check for a comment  */
		if ( c == 'C' || c == 'J') {
			while ( c != '\n' ) {
				putchar (c);
				c = getc();
			}
		}
			/* examin whats left */
		while (c != '\n') {
			if (ccheck(c) != 2){
				if ( c== '\'' && clast  != '\b') {
				putchar (c);
				if ((c=getc()) != '\b') {/* its a string*/
				putchar(c);
				while (c != '\'' | clast == '\b' |
					(ct= getc())  == '\b') {
				putchar (c=ct);}/* output the string */
				c=ct;
				}
				}
				else {
					putchar(c);
					c=getc();
				}
			}
			else {
				obpt=0;
				obj[obpt++] = c;
				while(ccheck(c=getc()) != 0)
					obj[obpt++] = c;
				obj[obpt++]='\0';
				m = match () ;
				if (m != 0)
					printf(" %d ",m);
				else { 
					obpt=0;
					while ((ct=obj[obpt++])!='\0')
						putchar (ct);
				}
			}
		}
		putchar('\n');
		lincnt++;
	}
	exit(0);		/* successful exit */
}
ccheck (cc) 
	char cc ;
{
	int res ;
	if (cc >= '0' && cc <= '9' )
		res = 1;
	else if(cc >='a' && cc <= 'z')
		res = 2;
	else
		res =0;
	return (res);
}


match() {
	int mm;
	char cu,cs;
	mm=0;
	labcnt = 0;
	while (labcnt <= labnum-1 && mm==0) {
		lstp = lstlst[labcnt];
		obpt =0;
		while ((cu=lablst[lstp++])==(cs= obj[obpt++]) && cu !='\0') {}
		if (cu==cs && cu == '\0')
			mm = labn[labcnt];
		else
			labcnt++;
	}
	return (mm);
}
char
getc()
{
	clast=c;
	return (getchar());
}
