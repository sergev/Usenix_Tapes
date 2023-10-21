USELESS (OR NEARLY SO)
/*  CENTROID FROM QTREE
*
*   program to locate the centroid of the shape
*   represented by the quad tree
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
int ipts,jpts,nodes,offset,gx,gy,error,words,tol,x1,y1 ;
int nlev ;
int blocks 0 ;
int maximal 0 ;
int p1,p2,p3 ;
double dsum,rsum,dtemp,white,black ;
int head[8] ;                   /* tree file header     */
int prof[10] ;                  /* profile of tree      */
int *tree ;                     /* the root             */
int space[9800] ;               /* space for many nodes */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */
float XSUM,YSUM,CGX,CGY,MASS ;


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
*  this section will find the centroid
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

cent(tx,ty,lx,ly,node,lev)
int tx,ty,lx,ly,*node,lev ;{

if (leaf(node) && node[0] == black && lev <= nlev)
  {
   x1 = tx + abs((tx - lx) / 2 ) ;
   y1 = ly + abs((ty - ly) / 2 ) ;
   XSUM = XSUM + (x1 * node[2]) ;
   YSUM = YSUM + (y1 * node[2]) ;
   MASS = MASS + node[2] ;
  }
else if (leaf(node)) return ;
else
    {
     cent(tx,ty,tx+b[lev],ty-b[lev],node[3],lev+1) ;
     cent(tx+b[lev],ty,lx,ty-b[lev],node[4],lev+1) ;
     cent(tx+b[lev],ty-b[lev],lx,ly,node[5],lev+1) ;
     cent(tx,ty-b[lev],tx+b[lev],ly,node[6],lev+1) ;
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
if (argc != 5)
  {
   printf("usage is : qcg qtree lev s1 s2\n") ;
   cexit() ;
  }

nlev = atoi(options[2]) ;
printf("%s,%s    %d   ",options[3],options[4],nlev) ;
file = open(options[1],2) ;
readq() ;   /* read tree from disc file */
XSUM = YSUM = CGX = CGY = MASS = 0.0 ;
cent(0,64,64,0,tree,0) ; /* call centroid routine */
if (MASS != 0.0)
  {
   CGX = XSUM/MASS ;
   CGY = YSUM/MASS ;
   printf("  %f,%f\n",CGX,CGY) ;
  }
else printf("  no level %d nodes\n",nlev) ;
putchar('\n') ;
cexit() ;
}
