#include <stdio.h>
#include <fcntl.h>
#define strlen 256
 
main(argc,argv)  /* rev- lists file. */
		/* argc - number of args*/
		/* argv - pointer to array of char strings of args*/
int argc;
char *argv[];
{
 int fp, open(), creat() , strcmp() ;
 char template[30], *mktemp() , *strcpy() ;
 char *tmp;
char *tmpname() ;
 long pos;
 int n;
 
 if( argc == 1 ) /* no args - halt */
    { printf("rev- no argument given\n");
      printf("usage-   rev file1 [{filen}]\n");
	printf(" alt:	rev -    reads from stdin (pipes etc...) \n");
      exit(1); }
 else
	while( --argc > 0 )
		if( (*++argv)[0] == '-' )
		{
			if( ( fp = open(( tmp = ( mktemp("/tmp/revXXXXXX\0")) ),O_RDWR | O_CREAT , 0755)) == -1 )
			{	printf("rev- cant open %s <<<<\n",*template);
			} else {
				filecopy(0,fp);
				filerev(fp) ;
				close(fp);
				unlink(tmp);
			}
		} else {
			if (( fp = open(*argv,0)) == -1 ) {
				printf("rev- cant open %s ###################\n",*argv);
			} else {
				filerev(fp) ;
				close(fp);
			}
		}
 }
 filecopy(fpi,fpo)	/* list a file			*/
 int fpi , fpo;		/* the file			*/
{ char c ;
 while ( ( read(fpi,&c,1) ) > 0 ) 
 	write(fpo,&c,1) ; 
 }
 filerev(fp)	/* list a file			*/
 int fp;		/* the file			*/
{ char c ;
  long p ;
	int flag;
 long lseek() ;
 int whence;
 p=0L ;
 whence=2;
 flag=1;
  while ( ( lseek(fp,p,whence ) ) != -1L )  {
	p=0L-2L;
	whence=1;
 	read(fp,&c,1) ; 
	if( flag==0 ) 	write(1,&c,1) ; 
	flag=0;
	}
	printf("\n");
 }
