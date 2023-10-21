/* CREATE QUAD TREE
*
*  quad tree maker (1)
*  program to make a quad tree from a given input picture (64 x 64)
*  input picture (with GXAP header) is specified on command line
*  writes out 'portable' quad tree file
*  full of inefficient code , needs to be cleaned up
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,file1,file2,sum,stackptr,nospace ;
int ipts,jpts,nodes,offset,gx,gy,error,size,words ;
double dsum,rsum,dtemp,white,black ;
char picture[64][64] ;          /* input picture */
int area[16] ;                  /* Grinnell area        */
char buf[64] ;                  /* input buffer         */
int head[8] ;                   /* tree file header     */
int cellsize 8 ;                /* size of node         */
int *tree ;                     /* the root             */
int *fptr ;                     /* pointer to new node  */
int space[9800] ;               /* space for many nodes */
int bound 9800 ;                /* words available      */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */

int leaves 0 ;                  /* no of leaf nodes     */
int count 0 ;                   /* counter              */

/********************************/
/* initialization routine       */
/********************************/

init() {
white = 63.0 ;                          /* set black and white  */
black = 0.0 ;
read(file1,buf,12) ;                    /* skip header          */
if ((read(file1,picture,4096)) == -1)   /* read picture         */
  {
   printf("couldn't read picture file\n") ;
   cexit() ;
  }
tree = fptr = space ;                   /* define root as the   */
nospace = error = nodes = size = 0 ;    /* first location       */
words = 9800/cellsize ;
}


/***********************************/
/* returns average gl of quadrant  */
/***********************************/

double aver(x1,y1,x2,y2,q)
int x1,y1,x2,y2,q ; {
if (x1 > x2 || y1 < y2)
  {
   printf("program error %d %d %d %d %d\n",q,x1,y1,x2,y2) ;
   return(-1.0) ;
  }
else if (x1 == x2 && y1 == y2)
       {
	printf("returning pixel %d %d %d %d\n",x1,y1,x2,y2) ;
	if (picture[x1][y1] <= white) return(white) ;
	else return(black) ;
       }
rsum = dsum = 0.0 ;
sum = ipts = jpts = 0 ;
for (i = x1 ; i<x2 ; i++)
   {
    jpts = 0 ;
    rsum = 0.0 ;
    for ( j=y2 ; j<y1 ; j++)
       {
	rsum = rsum + picture[i][j] ;
	jpts ++ ;
       }
    dsum = dsum + rsum/jpts ;
    ipts ++ ;
   }
dsum = dsum/ipts ;
/* printf("quad: %d %d %d %d  aver: %f\n",x1,y1,x2,y2,dsum) ;
   printf("nodes used and quad %d %d\n",nodes,q) ;
*/ return(dsum) ;
}

/***************************************/
/* grows a tree from the input picture */
/* according to rules described        */
/***************************************/

grow(tx,ty,lx,ly,node,node1,type,lev)
int tx,ty,lx,ly,type,lev ;
int *node,*node1 ; {

dtemp = aver(tx,ty,lx,ly,type) ;
node[0] = dtemp ;                       /* average gray level */
node[1] = lev ;                         /* current level      */
node[2] = node1 ;                       /* pointer to father  */
if (dtemp >= white || dtemp <= black || lev > 5)
  {
   for (i=3 ;i<14 ;i++)
     node[i] = 0 ;                      /* was black or white */
   leaves ++ ;
  }
else
    {
     for (i=3; i<7 ;i++)
	{
	node[i] = getnode() ;                        /* has descendants */
	if (node[i] < 0) count ++ ;
	}
     grow(tx,ty,tx+b[lev],ty-b[lev],node[3],node,1,lev+1) ;
     grow(tx+b[lev],ty,lx,ty-b[lev],node[4],node,2,lev+1) ;
     grow(tx+b[lev],ty-b[lev],lx,ly,node[5],node,3,lev+1) ;
     grow(tx,ty-b[lev],tx+b[lev],ly,node[6],node,4,lev+1) ;
    }
}

/********************************/
/* returns pointer to new node  */
/********************************/

getnode() {
fptr = fptr + cellsize ;
nodes++ ;
if (nodes*cellsize > bound)
  {
   printf("ran out of space after %d nodes\n",nodes) ;
   cexit() ;
  }
return(fptr) ;
}

/*******************************/
/* writes out qtree to disc    */
/*******************************/

writeq(file)
int file ; {
int base,blocks ;
base = &space[0] ;
blocks = 1 + (nodes*cellsize)/128 ;             /* make qfile header    */
head[0] = blocks ;                              /* size in blocks       */
head[1] = white ;                               /* gray level range     */
head[2] = black ;
head[3] = cellsize ;                            /* node size            */
head[4] = 0 ;                                   /* unroped              */

write(file,head,10) ;                   /* 8-byte header        */

for (i=0 ;i<9800 ;i++)
   if (space[i] >= base) space[i] = space[i] - base + 500 ;

for (i=0 ;i< blocks ; i++)                           /* write to disc*/
    if ((write(file,&space[i*128],256)) == -1)
      {
       printf("error in writing output file\n") ;
       cexit() ;
      }
printf(" white: %d black: %d blocks: %d\n",head[1],head[2],blocks) ;

}

/*******************/
/* control section */
/*******************/

main(argc,options)
int argc ;
char **options ; {
   if (argc < 3)
     {
      printf("usage is : qmake <ifile> <qfile> / < r >\n") ;
      cexit() ;
     }
   file1 = open(options[1],2) ;
   if (argc == 4) cellsize = 16 ;
   init() ;
   grow(0,64,64,0,tree,tree,0,0) ;
   printf("nodes : %d leaves: %d",nodes-leaves,leaves) ;
   /* write out quad tree to disc file  */
   writeq(creat(options[2],0777)) ;
   printf("words used %d\n",128*head[0]) ;

   cexit() ;
}
