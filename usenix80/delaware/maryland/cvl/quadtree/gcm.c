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
*   Approximation to shape is either
*     1) 'b' specifying black nodes up to a given level
*        (inside approximation)
*     2) 'g' specifying black + gray nodes up to a given level
*        (inside + outside approx)
*
*   These options are specified on the command line - see format below
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,file,sum,stackptr,nospace,cellsize,ofile ;
int ipts,jpts,nodes,offset,gx,gy,error,words,gn,li,lj ;
int nlev ;
int blocks 0 ;
int maximal 0 ;
int p1,p2,p3 ;
double dsum,rsum,dtemp,white,black,fact,mass,fr ;
int head[8] ;                   /* tree file header     */
int prof[10] ;                  /* profile of tree      */
int *tree ;                     /* the root             */
int space[9800] ;               /* space for many nodes */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */
double SUM,XSUM,YSUM,CGX,CGY,MASS,M,x1,y1,n2,I,J,t1,t2,t ;
double pow() ;

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

gray(node)
int *node ;{
return(node[0] != white && node[0] != black) ;
}

cent(tx,ty,lx,ly,node,lev)
int tx,ty,lx,ly,*node,lev ;{

if ((leaf(node) && node[0] == black && lev <= nlev)  ||
    (gray(node) && lev == nlev && gn == 1 )          ||
    (node[15] == 1))
  {
   if (lev == 6)
     {
      x1 = tx + 0.5 ;
      y1 = ly + 0.5 ;
     }
   else
     {
      x1 = tx + abs((tx - lx) / 2 ) ;
      y1 = ly + abs((ty - ly) / 2 ) ;
     }
   n2 = node[2] ;
   XSUM = XSUM + (x1 * n2) ;
   YSUM = YSUM + (y1 * n2) ;
   MASS = MASS + n2 ;
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


cmom(tx,ty,lx,ly,node,lev)      /* central moments routine */
int tx,ty,lx,ly,*node,lev ;{

if ((leaf(node) && node[0] == black && lev <= nlev)  ||
    (gray(node) && lev == nlev && gn == 1 )          ||
    (node[15] == 1))
  {
   /* find centroid of this node (x1,y1) */
   if (lev == 6)
     {
      x1 = tx + 0.5 ;
      y1 = ly + 0.5 ;
     }
   else
     {
      x1 = tx + abs((tx - lx) / 2 ) ;
      y1 = ly + abs((ty - ly) / 2 ) ;
     }
   /* find the mass of this node n2  */
   mass = node[2] ;
   /* find (x - x1) (y - y1) */
   t1 = x1 - CGX - 0.5 ;
   t2 = y1 - CGY - 0.5 ;
   t = pow(t1,I) * pow(t2,J) ;
   t = t * mass ;
   SUM = SUM + t ;
/* printf("CGX= %f, CGY= %f t1= %f t2= %f, mass= %f, t= %f,SUM= %f \n",
	   CGX,CGY,t1,t2,mass,t,SUM) ;  */
  }
else if (leaf(node)) return ;
else
    {
     cmom(tx,ty,tx+b[lev],ty-b[lev],node[3],lev+1) ;
     cmom(tx+b[lev],ty,lx,ty-b[lev],node[4],lev+1) ;
     cmom(tx+b[lev],ty-b[lev],lx,ly,node[5],lev+1) ;
     cmom(tx,ty-b[lev],tx+b[lev],ly,node[6],lev+1) ;
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
      printf("usage is : gcm qtree <g/b> I J\n") ;
      cexit() ;
     }

   if ((file = open(options[1],0)) == -1)
     {
      printf("couldn't open  %s\n",options[1]) ;
      cexit() ;
     }

   /* find whether to use inside or outside approx */
   if (options[2][0] == 'g') gn = 1 ;
   else gn = 0 ;

   /* find which moments required */
   li = atoi(options[3]) ;
   lj = atoi(options[4]) ;
   I = li ;
   J = lj ;

   /* read tree from disc & print out name*/
   readq() ;
   printf("%s  M(%d,%d)\n\n",options[1],li,lj) ;
   /* for all levels find M(I,J) */
   for (nlev=1 ; nlev < 7 ; nlev ++)
      {
       /* find centroid for this approximation */
       SUM = XSUM = YSUM = CGX = CGY = MASS = 0.0 ;
       cent(0,64,64,0,tree,0) ; /* call centroid routine */
       if (MASS != 0.0)
	 {
	  CGX = (XSUM/MASS) - 0.5 ;
	  CGY = (YSUM/MASS) - 0.5 ;
	  /* now find I,J central moment */
	  cmom(0,64,64,0,tree,0) ;
	  M = SUM / MASS ;
	  printf("%d    %f,%f %f  M(%d,%d) = %f\n",
		  nlev,CGX,CGY,MASS,li,lj,M) ;
	  putchar('\n') ;
	 }
       else printf("%d    no nodes\n\n",nlev) ;
      }
cexit() ;
}
