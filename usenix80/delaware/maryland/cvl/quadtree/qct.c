extern cgoof ;
/*
* Ropes 'btree' from corresponding 'qtree'
* or something similar
*/

int f1,f2,f3,i,m,j ;
int tr[16],br[16] ;

main(argc,argv)
int argc ;
char **argv ;{

m = 0 ;
if (argc != 4)
  {
   printf("usage: qct t1 t2 t3\n") ;
   cexit() ;
  }
if ((f1 = open(argv[1],0)) == -1)
  {
   printf("couldn't open %s\n",argv[1]) ;
   cexit() ;
  }
if ((f2 = open(argv[2],0)) == -1)
  {
   printf("couldn't open %s\n",argv[2]) ;
   cexit() ;
  }
if ((f3 = open(argv[3],2)) == -1)
  {
   printf("couldn't open %s\n",argv[3]) ;
   cexit() ;
  }
/* get average gray level etc */

if ((read(f1,tr,10)) == -1)
  {
   printf("couldn't read header of %s\n",argv[1]) ;
   cexit() ;
  }
if ((read(f2,br,10)) == -1)
  {
   printf("couldn't read header of %s\n",argv[2]) ;
   cexit() ;
  }
if ((write(f3,tr,10)) == -1)
  {
   printf("couldn't write header to %s\n",argv[3]) ;
   cexit() ;
  }
for (i=0 ;i < 10000 ; i++)
   {
    if ((read(f1,tr,32)) == -1)
      {
       printf("couldn't read cell %4d of %s\n",i,argv[1]) ;
       cexit() ;
      }
    if ((read(f2,br,32)) == -1)
      {
       printf("couldn't read cell %4d of %s\n",i,argv[2]) ;
       cexit() ;
      }
    if (tr[15] == 1) m ++ ;
    tr[2] = br[2] ;
/*
    for (j=0 ;j < 16 ; j++)
       printf("%d ",tr[j]) ;
    putchar('\n') ;
    for (j=0 ;j < 16 ; j++)
       printf("%d ",br[j]) ;
    printf("\n\n") ;
*/

    if ((write(f3,tr,32)) == -1)
      {
       printf("couldn't write cell %2d to %s\n",i,argv[3]) ;
       cexit() ;
      }
    if (tr[1] == 0 && i != 0)
      {
       printf("%4d cells in tree, %2d maximal nodes\n",i,m) ;
       cexit() ;
      }
    }
cexit() ;
}

















































