/*  MAXIMAL NODES
*
*   program to mark maximal nodes of a quad tree
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
int blocks 0 ;
int maximal 0 ;
double dsum,rsum,dtemp,white,black ;
char picture[64][64] ;          /* input picture */
int area[16] ;                  /* Grinnell area        */
char buf[64] ;                  /* input buffer         */
int head[8] ;                   /* tree file header     */
int prof[10] ;                  /* profile of tree      */
int *tree ;                     /* the root             */
int *fptr ;                     /* pointer to new node  */
int space[9800] ;               /* space for many nodes */
int stack[100] ;                /* stack for 'findn'    */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */

int opp[5] {0,3,4,1,2} ;        /* opposite sides       */
int bnodes 0 ;                  /* no of leaf nodes     */
int nbrs[10] ;                  /* neighbour sizes      */

/*
*  Section 2
*
*  This section will read and write trees from disc
*
*  readq will read the tree and make pointers into real addresses
*  writeq will make pointers relative and write the tree
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


writeq(){
int base ;
base = &space[0] ;
head[4] = 1 ;
if ((write(file,head,10)) == -1)
  {
   printf("couldn't write roped file\n") ;
   cexit() ;
  }
for (i=0 ;i<9800 ;i++)
   if (space[i] >= base) space[i] = space[i] - base + 500 ;

for (i=0 ;i< blocks ; i++)                           /* write to disc*/
    if (write(file,&space[i*128],256) < 0)
      {
       printf("couldn't write block %d of qfile\n",i) ;
       cexit() ;
      }
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


mark(node)
int *node ;{
int k1,k2,nsz,csz,*nb,flag ;
if (leaf(node) && node[0] == black)
  {
   bnodes ++ ;
   prof[node[1]] ++ ;
   node[15] = 2 ;
   for (k1=7 ;k1<15 ;k1++)
      {
       nb = node[k1] ;
       if (node[k1] != -1)
	 {
	  if ((nb[0] == black) && (nb[1] < (node[1]+tol) )) node[15] = 0 ;
	 }
      }
   if (node[15] == 2) maximal ++ ;
  }
else if (leaf(node)) return ;
else
  {
   mark(node[3]) ;
   mark(node[4]) ;
   mark(node[5]) ;
   mark(node[6]) ;
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
      printf("usage is : qmax qfile tol \n") ;
      cexit() ;
     }
   file = open(options[1],2) ;
   tol = atoi(options[2]) ;
   readq() ;
   mark(tree) ;                                 /* mark max. nodes */
   file = creat(options[1],0777) ;
   writeq() ;                                   /* write to disc   */
   printf("black nodes: %d  maximal nodes: %d\n",bnodes,maximal) ;
   printf("\ndistribution of node sizes : \n") ;
   for (i=0 ;i<8 ;i++)
      printf("%d  %d\n",b[i],prof[i+1]) ;
   putchar('\n') ;
   cexit() ;
}
