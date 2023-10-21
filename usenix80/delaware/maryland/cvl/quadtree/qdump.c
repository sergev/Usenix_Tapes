/* quad tree dump
   program to dump N quad tree cells of a quad tree file created by
   'qmake',one cell per line,starting with M th cell
   arguments :
   a1 = number of cells to dump
   a2 = first cell to dump
*/

extern cgoof ;
int i,j,k,nstart,ncells,file,cellsize,temp,blocks ;

int  head[8] ;                  /* file header buffer   */
int space[9800] ;               /* space for many nodes */
int bound 9800 ;               /* available space      */

readq(){
if ((read(file,head,10)) == -1)
  {
   printf("error in reading tree file\n") ;
   cexit() ;
  }
blocks = head[0] ;
cellsize = head[3] ;
temp = 256*bound ;

if (blocks*128 > bound)
  {
   printf("tree too large %d\n",temp-bound) ;
   cexit() ;
  }
for (i=0 ;i< blocks ; i++)
   {
    if ((read(file,&space[i*128],256)) == -1)
      {
       printf("error in reading block %d\n",i) ;
       cexit() ;
      }
   }
}


show(){
int t,count ;
printf("\ndump of %d cellsfrom cell %d\n",ncells,nstart) ;
count = 0 ;
k = nstart*cellsize ;
for (i=0 ;i< ncells ; i++)
   {
    count ++ ;
    t = (&space[k] - &space[0])*2  + 500 ;
    printf("%3d %4d ",count,t) ;
    for (j=0 ;j<cellsize ;j++)
       printf("%d ",space[k++]) ;
    printf("\n") ;
   }
}



main(argc,options)
int argc ;
char **options ; {
if (argc < 4)
  {
   printf("usage : qdump <qtree> <no.of cells> <start cell> \n") ;
   cexit() ;
  }
if ((file = open(options[1],0)) == -1)
  {
   printf("error in opening file : %s\n",options[1]) ;
   cexit() ;
  }
readq() ;
ncells = atoi(options[2]) ;
nstart = atoi(options[3]) ;
show() ;
cexit() ;
}
