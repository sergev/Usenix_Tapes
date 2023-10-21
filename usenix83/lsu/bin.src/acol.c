#
/* acol
 | - automatically fill columns across a page 
 |	from the standard input
 |	ls | acol [-l# -w# -f# -lp]
 |		- l  - specify page length
 |		- w  - page width
 |		- f  - fill between columns
 |		- s  - spaces between pages
 |		- lp - page for line printer
 |	Purdue University - Agricultural Engineering Dept.
 |	D.Fouts		1 Aug 77
*/
#define LNGTH  66
#define WIDTH 136
#define BUFL  180
 extern	atoi();
 char	page[LNGTH][WIDTH];
 int	mline	22;
 int	mcol	72;
 int	fil	3;
 int	ispce	1;
 int	iflag	0;
 int	lcol,rcol;
main(argc,argv)
 int	argc;
 char	**argv;
{
	char	buf[BUFL];
	int	bptr,cptr,lptr,i,cnt;
	for( i=1; i<argc; i++){
		if( argv[i][0] != '-' ){
			write(2,"Syntax error\n",13);
			exit(-1);
		}
		switch( argv[i][1] ){
		    case 'l':
			if( argv[i][2] == 'p' ){
				mline= 60;
				mcol=  70;
				iflag= 1;
				ispce= 3;
			  }else{
				mline= atoi( &argv[i][2] );
				if( mline < 0 ) mline = -mline;
				if( mline > LNGTH ) mline= LNGTH;
			}
			break;
		    case 'w':
			mcol= atoi( &argv[i][2] );
			if( mcol <  0 ) mcol= -mcol;
			if( mcol > WIDTH ) mcol= WIDTH;
			break;
		    case 'f':
			fil= atoi( &argv[i][2] );
			if( fil < 0 ) fil= -fil;
			break;
		    case 's':
			ispce= atoi( &argv[i][2] );
			if( ispce < 0 ) ispce= -ispce;
			break;
		    default:
			write(2,"Illegal flag\n",13);
			exit(-1);
		}
	}
	lptr= cptr= lcol= rcol= 0;
	while( ( cnt= read(0,&buf,BUFL) ) > 0 ){
		for( bptr=0; bptr<cnt; bptr++){
				if( cptr > rcol ) rcol= cptr;
			if( buf[bptr] == '\n' ){
   nxt:
				if( ++lptr >= mline ){
					adjust();
					lptr= 0;
				}
				cptr= lcol;
			  }else{
				page[lptr][cptr++]= buf[bptr];
			}
				if( rcol > mcol ){
					if( lcol == 0 ){
						rcol= mcol;
						goto nxt;
					}
					cptr= cptr- lcol;
					nxtpage();
				}
		}
	}
	if( cnt < 0 ){
		write(2,"System error\n",13);
		exit(-1);
	}
	if( iflag == 1 ) write(1,"\n\n\n",3);
	for( lptr=0; lptr<mline; lptr++){
		if( iflag == 1 ) write(1,"     ",5);
		write(1,&page[lptr][0],rcol);
		putchar('\n');
	}
	for( lptr=1; lptr<=ispce; lptr++)
		putchar('\n');
}
 adjust()
 {
	int	aline,acol;
	if( rcol > mcol ) nxtpage();
	rcol= rcol+ fil;
	for( aline=0; aline<mline; aline++){
		acol= lcol;
		while( page[aline][acol] != '\0')	acol++;
		while( acol<rcol ) page[aline][acol++]= ' ';
	}
	lcol= rcol;
 }
 nxtpage()
 {
	int	bline,bcol,ccol;
	if( iflag == 1 ) write(1,"\n\n\n",3);
	for( bline=0; bline<mline; bline++){
		if( iflag == 1 ) write(1,"     ",5);
		write(1,&page[bline][0],lcol);
		putchar('\n');
		bcol= lcol;
		ccol= 0;
		while( page[bline][bcol] != '\0' ) 
			page[bline][ccol++]= page[bline][bcol++];
		while( ccol < WIDTH ) page[bline][ccol++]= '\0';
	}
	for( bline=1; bline<=ispce; bline++)
		putchar('\n');
	rcol= rcol- lcol;
	lcol= 0;
 }
