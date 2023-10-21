extern cgoof ;
/*
*  Program to print out a window on the Grinnell
*/

int i,j,winx,winy,a ;
int area[16] ;
int bufi[64] ;

main(argc,argv)
int argc ;
char **argv ;{

if (argc < 3)
  {
   printf("usage : qpr winx winy < p >\n") ;
   cexit() ;
  }
winx = atoi(argv[1]) ;
winy = atoi(argv[2]) ;

gopen(area,winx,winy,64,64) ;
for (i=0 ;i < 64 ;i++)
   {
    if (argc == 4) putchar('\n') ;
    grcol(area,bufi,i,0,64,1) ;
    for (j=0 ;j<64 ;j++)
       if (bufi[j] == 0)
	 {
	  if (argc == 4) putchar('X') ;
	  a ++ ;
	 }
       else if (argc == 4) putchar('.') ;
   }
putchar('\n') ;
printf("area = %6d\n",a) ;
cexit() ;

}
