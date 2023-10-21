/*
*  quad tree medial axis transform
*  finds the QMAT of a quadtree
*  see TR by Samet
*
*  The quadtree handled by this program has already been treated by QNC (V)
*  Words 7 - 10 contain pointers to 4 neighbours
*  Words 11 - 14 contain pointers to 4 corners
*  Word 15  is the 'dist' field for the node (See Samet's 'distance' TR)
*
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,file,sum,stackptr,nospace,cellsize ;
int blocks,fcount,scount ;
double dsum,rsum,dtemp,white,black ;
int head[10] ;                  /* tree file header     */
int *tree ;                     /* the root             */
int space[9800] ;               /* space for many nodes */
int b[] {1,2,4,8,16,32,64} ;

/* read in tree */

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
if (cellsize != 16)
  {
   printf("this tree cannot be roped\n") ;
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


leaf(node)                               /* is arg a leaf ?         */
int *node ; {
return(node[3] == 0 &&
       node[4] == 0 &&
       node[5] == 0 &&
       node[6] == 0
      ) ;
}

notnull(node)
int *node ;{
return(node != tree) ;
}

gray(node)
int *node ;{
return(node[0] != black && node[0] != white) ;
}

light(node)
int *node ;{
return(leaf(node) && node[0] == white) ;
}

dark(node)
int *node ;{
return(leaf(node) && node[0] == black) ;
}

son(node,type)
int *node,type ; {
return(node[type]) ;
}


qmat(node,level)
int *node,level ;{
int *p,*y,*s1,*s2,*s3,*s4,p1,p2,p3,nbl ;

if (dark(node))
  for (p1=7 ; p1<15 ;p1++)
     {
      p = node[p1] ;                    /* neighbour */
      nbl = p[1] ;                      /* nbr lev   */
      if (notnull(p) && ((p[15] - b[nbl-1] - b[level-1]) == node[15]))
	{
	 node[0] = white ;              /* node subsumed by nbr q */
	 scount ++ ;
	}
     }
else if (gray(node))
    {
     for (p2=3 ;p2 < 7 ; p2++)
	qmat(node[p2],level-1) ;
     s1 = node[3] ;
     s2 = node[4] ;
     s3 = node[5] ;
     s4 = node[6] ;
     if (light(s1) && light(s2) && light(s3) && light(s4))
       {
	node[0] = 0 ;
	node[3] = node[4] = node[5] = node[6] = 0 ;
       }
    }
}


/* writes out quad tree in 'relative' form to disc file */

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

/* control section */

main(argc,options)
int argc ;
char **options ; {
   if (argc != 3)
     {
      printf("usage is : qmat tree file \n") ;
      cexit() ;
     }
   file = open(options[1],2) ;
   readq() ;
   scount = 0 ;
   qmat(tree,6) ;
   printf("%3d black nodes subsumed\n") ;
   file = creat(options[2],0777) ;
   writeq() ;                                   /* write to disc   */
   cexit() ;
}
