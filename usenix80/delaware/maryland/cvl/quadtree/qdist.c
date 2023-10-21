/*
*  quad tree distance transform   (Chessboard distance)
*  see TR - 780 by Samet
*
*  Word 15 of the node cell will be the distance field
*
*/

extern cgoof ;
int i,j,k,l,m,n,file,sum,stackptr,nospace,cellsize ;
int blocks,t1,t2,x,y,z ;
double dsum,rsum,dtemp,white,black ;
int head[10] ;                  /* tree file header     */
int *tree ;                     /* the root             */
int space[9800] ;               /* space for many nodes */
int opp[5] {0,3,4,1,2} ;        /* opposite sides      */
int quads[17] {0,0,2,0,1,2,0,3,0,0,3,0,4,1,0,4,0} ;
int coms[17]  {0,0,1,0,4,1,0,2,0,0,2,0,3,4,0,3,0} ;
int cs[] {0,2,3,4,1} ;
int ccs[] {0,4,1,2,3} ;
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


quad(side1,side2)                        /* quad which touches the  */
int side1,side2 ; {                      /* corner formed by s1 & s2*/
return(quads[(4*side1) - 4 + side2]) ;
}

opquad(qd)
int qd ;{
return(opp[qd]) ;
}

son(node,type)
int *node,type ; {
return(node[type]) ;
}

opside(side)
int side; {
return(opp[side]) ;
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

null(node)
int *node ;{
return(node == tree) ;
}

ccside(side1)                            /* next side anti-clock  */
int side1 ;{
return(ccs[side1]) ;
}

cside(side1)                             /* next side , clock       */
int side1 ;{
return(cs[side1]) ;
}

dstadj(node,q1,q2,w,c,d)
int *node,q1,q2,w,c,d ; {
int r,r1,l1,l2 ;
r1 = node - tree ;
if (r1 <= 0)
  {
   printf("error in dstadj\n") ;
   return(0) ;
  }
printf("dstadj: %d %d %d %d %d node: %d\n",q1,q2,w,c,d,r1) ;
if (c >= d) r = d ;             /* min already found */
else if (light(node)) r = c ;
else if (dark(node)) r = d ;
else if (dark(son(node,q1)) && dark(son(node,q2)))
       {
	printf("dstadj  ... 1\n") ;
	l1 = dstadj(son(node,opquad(q2)),q1,q2,w/2,c+w,d) ;
	r = dstadj(son(node,opquad(q1)),q1,q2,w/2,c+w,l1) ;
       }
else
   {
    printf("dstadj  ... 2\n") ;
    l2 = dstadj(son(node,q1),q1,q2,w/2,c,d) ;
    r = dstadj(son(node,q2),q1,q2,w/2,c,l2) ;
   }
printf("dstadj out %d\n",r) ;
return(r) ;
}


dstcor(node,s1,s2,w,c,d)
int *node,s1,s2,c,d,w ; {
int temp,r ;
r = node - tree ;
if (r <= 0)
  {
   printf("error in dstadj\n") ;
   return(0) ;
  }
printf("dstcor %d %d %d %d %d node: %d\n",s1,s2,w,c,d,r) ;
if (c >= d) r = d ;
else if (light(node)) r = c ;
else if (dark(node)) r = d ;
else
   {
    temp = dstcor(son(node,opquad(quad(s1,s2))),s1,s2,w/2,c,d) ;
    temp = dstadj(son(node,quad(s1,opside(s2))),
		  opquad(quad(s1,s2)),
		  quad(opside(s1),s2),w/2,c+w,temp) ;
    temp = dstadj(son(node,quad(opside(s1),s2)),
		  quad(s1,opside(s2)),
		  opquad(quad(s1,s2)),w/2,c+w,temp) ;
    temp = dstcor(son(node,quad(s1,s2)),s1,s2,w/2,c+w,temp) ;
    r = temp ;
   }
printf("dstcor out: %d\n",r) ;
return(r) ;
}


chess(node,level)
int *node,level ;{
int *q,d,s,p1,t ;
printf("chess %d\n",level) ;
if (level == 0) return(0) ;
if (dark(node))
  {
   d = b[level] ;
   for (s=1 ;s < 5 ; s++)
      if (d != 0)
	{
	 q = node[s+6] ;                /* side nbrs */
	 printf("nbr: %d  s: %d  d:%d \n",q-tree,s,d) ;
	 if (null(q) || light(q)) d = 0 ;
	 else
	     {
	      printf("dstadj on node : %d\n",q - tree) ;
	      d = dstadj(q,quad(opside(s),ccside(s)),
			   quad(opside(s),cside(s)),
			   b[level-1],0,d) ;
	     }
	 if (d != 0)
	   {
	    t = quad(s,cside(s)) ;
	    q = node[10 + t] ;          /* corner nbrs */
	    if (null(q) || light(q)) d = 0 ;
	    else d = dstcor(q,s,cside(s),b[level-1],0,d) ;
	    s = cside(s) ;
	   }
	}
   node[15] = d + b[level - 1] ;
  }
else if (gray(node))
    {
     chess(node[3],level-1) ;
     chess(node[4],level-1) ;
     chess(node[5],level-1) ;
     chess(node[6],level-1) ;
    }
else node[15] = 0 ;
printf("chess out %d\n",level) ;

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

   i = tree ;
   printf("base : %d\n",i) ;
   chess(tree,6) ;

   file = creat(options[2],0777) ;
   writeq() ;                                   /* write to disc   */
   cexit() ;
}
