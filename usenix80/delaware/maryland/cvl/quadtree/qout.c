#
/*  MARKED NODES
*
*   program to output marked nodes of a quad tree
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
#include        "qpars"
int i,j,k,l,m,n,temp,pts,file,sum,stackptr,nospace,cellsize ;
int ipts,jpts,nodes,offset,gx,gy,error,words,tol,next ;
int blocks 0 ;
int maximal 0 ;
double dsum,rsum,dtemp,white,black ;
int table[TSIZE][3] ;           /* marked  node table */
int head[8] ;                   /* tree file header     */
int prof[10] ;                  /* profile of tree      */
int *tree ;                     /* the root             */
int *fptr ;                     /* pointer to new node  */
int space[9800] ;               /* space for many nodes */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */

int bnodes 0 ;                  /* no of leaf nodes     */

/*
*  Section 2
*
*  This section will read and write trees from disc
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
if (cellsize < 16)
  {
   printf("can't find maximal nodes of unroped tree\n") ;
   cexit() ;
  }
if (head[4] != 1)
  {
   printf("tree is not roped\n") ;
   cexit() ;
  }
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
*  this section will mark maximal nodes in this tree
*
*  leaf returns 'true' if node is a leaf
*  size returns the size of the quadrant represented by the node
*  neigh returns the appropriate neighbour
*  mark will set node[15] = 0 if it is a maximal node
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



out(tx,ty,lx,ly,node,lev)
int tx,ty,lx,ly,lev ;
int *node ;{
if (node[15] == 1)
  {
   if (next > TSIZE)
     {
      printf("too many marked nodes for table\n") ;
      cexit() ;
     }
   table[next][0] = tx ;
   table[next][1] = ly ;
   table[next][2] = node[1] ;
   next ++ ;
   printf("%2d %2d %2d %2d\n",next,tx,ly,lev) ;
  }
else if (leaf(node)) return ;
else
    {
     out(tx,ty,tx+b[lev],ty-b[lev],node[3],lev+1) ;
     out(tx+b[lev],ty,lx,ty-b[lev],node[4],lev+1) ;
     out(tx+b[lev],ty-b[lev],lx,ly,node[5],lev+1) ;
     out(tx,ty-b[lev],tx+b[lev],ly,node[6],lev+1) ;
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
   if (argc != 3)
     {
      printf("usage is : qout qfile mfile\n") ;
      cexit() ;
     }
   if ((file = open(options[1],2)) == -1)
     {
      printf("couldn't open tree file\n") ;
      cexit() ;
     }
   readq() ;
   printf("no tx ly lev\n\n") ;
   out(0,64,64,0,tree,0) ;                     /* mark max. nodes */
   if ((file = creat(options[2],0777)) == -1)
     {
      printf("couldn't create table file\n") ;
      cexit() ;
     }
   if ((write(file,&table[0][0],TSIZE*3*2)) == -1)
     {
      printf("couldn't create mnode file\n") ;
      cexit() ;
     }
   printf("%d marked nodes in table\n",next) ;
   cexit() ;
}
