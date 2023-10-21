/*  PROFILE QTREE
*
*   program to count black white and gray nodes
*   read the file /mnt/class/smr/qtrees/doc before
*   doing anything further !
*
*   Section 1
*   This section contains the data definitions for the
*   quad tree programs. It is the basic framework in which
*   all these programs operate.
*
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,file,sum,stackptr,nospace,cellsize ;
int ipts,jpts,nodes,offset,gx,gy,error,words,tol ;
int bnodes,wnodes,gnodes,tnodes,mnodes ;
int blocks 0 ;
int maximal 0 ;
int p1,p2,p3 ;
double dsum,rsum,dtemp,white,black ;
int head[8] ;                   /* tree file header     */
int bnd[10] ;                   /* black node dist      */
int gnd[10] ;                   /* gray                 */
int wnd[10] ;                   /* white                */
int mnd[10] ;                   /* maximal              */
int *tree ;                     /* the root             */
int space[9800] ;               /* space for many nodes */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */


/*
*  Section 2
*
*  This section will read tree from disc
*
*  readq will read the tree and make pointers into real addresses
*
*/


readq(){
int base ;
base = &space[0] ;
if ((read(file,head,10)) == -1)
  {
   printf("error in reading tree file\n") ;
   cexit() ;
  }
blocks = head[0] ;
white = head[1] ;
black = head[2] ;
cellsize = head[3] ;
for (i=0 ;i< blocks ; i++)
   if (read(file,&space[i*128],256) < 0)
     {
      printf("error in reading block %d\n",i) ;
      cexit() ;
     }
for (i=0 ;i< 9800 ; i++)
   if (space[i] >= 500) space[i] = space[i] + base - 500 ;
tree = space    ;
}




/*
*  Section 3
*
*  this section will count nodes in this tree
*
*  leaf returns 'true' if node is a leaf
*
*/


leaf(node)
int *node ; {
return(node[3] == 0 &&
       node[4] == 0 &&
       node[5] == 0 &&
       node[6] == 0
      ) ;
}

/* find black node distribution */
trav(node)
int *node ;{

if (leaf(node) && node[0] == black)
  {
   bnodes ++ ;
   bnd[node[1]] ++ ;
   if (node[15] == 1)
     {
      mnd[node[1]] ++ ;
      mnodes ++ ;
     }
  }
else if (leaf(node))
    {
     wnodes ++ ;
     wnd[node[1]] ++ ;
    }
else
    {
     gnodes ++ ;
     gnd[node[1]] ++ ;
     trav(node[3]) ;
     trav(node[4]) ;
     trav(node[5]) ;
     trav(node[6]) ;
    }
}


/*
* Section 4
*
* This is the control section which reads the command
* line and does the usual preliminaries
*
*/

main(argc,options)
int argc ;
char **options ; {
   if (argc < 2)
     {
      printf("usage is : qp qfile\n") ;
      cexit() ;
     }
   file = open(options[1],2) ;
   readq() ;
   trav(tree) ;
   tnodes = bnodes + wnodes + gnodes ;
   printf("%s  %3d\n",options[1],tnodes) ;

   /* print black node dist  */
   printf("black : %3d        ",bnodes) ;
   for (i=0 ;i< 7 ;i++)
      printf("%3d ",bnd[i]) ;
   putchar('\n');

   /* print white node dist  */
   printf("white : %3d        ",wnodes) ;
   for (i=0 ;i< 7 ;i++)
      printf("%3d ",wnd[i]) ;
   putchar('\n') ;

   /* print gray node dist  */
   printf("gray  : %3d        ",gnodes) ;
   for (i=0 ;i< 7 ;i++)
      printf("%3d ",gnd[i]) ;
   putchar('\n') ;

   /* print maximal node dist  */
   printf("mxml  : %3d        ",mnodes) ;
   for (i=0 ;i< 7 ;i++)
      printf("%3d ",mnd[i]) ;
   putchar('\n') ;

   putchar('\n') ;
   cexit() ;
}
