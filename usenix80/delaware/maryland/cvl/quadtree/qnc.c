/* quad tree maker (1)
   program to 'rope' a quad tree i.e. insert pointers at each black
   leaf to its eight neighbours
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,file,sum,stackptr,nospace,cellsize ;
int ipts,jpts,nodes,offset,gx,gy,error,size,words ;
int blocks ;
double dsum,rsum,dtemp,white,black ;
int head[10] ;                  /* tree file header     */
int *tree ;                     /* the root             */
int *fptr ;                     /* pointer to new node  */
int space[9800] ;               /* space for many nodes */
int stack[100] ;                /* stack for 'findn'    */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */
int bsun 0 ;                    /* debugger aid  ? ? !  */

int adjm[17]  {0,1,1,0,0,0,1,1,0,0,0,1,1,1,0,0,1} ;
int quads[17] {0,0,2,0,1,2,0,3,0,0,3,0,4,1,0,4,0} ;
int refl[17]  {0,4,2,4,2,3,1,3,1,2,4,2,4,1,3,1,3} ;
int coms[17]  {0,0,1,0,4,1,0,2,0,0,2,0,3,4,0,3,0} ;

int opp[5] {0,3,4,1,2} ;        /* opposite sides      */
int north 1 ;                   /* sides of quadrant   */
int east 2  ;
int south 3 ;
int west 4  ;
int leaves 0 ;                  /* no of leaf nodes     */

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

/* this section finds pointers to */
/* each of eight neighbours .     */

father(node)                            /* returns father of node */
int *node ; {
return(node[2]) ;
}

adj(side,quad)                           /* if quad is adj to side */
int side,quad ; {
if (quad < 1) return(quad == 1) ;
else return(adjm[4*side - 4 + quad] == 1)  ;
}

fquad(side1,side2)                       /* quad which touches the  */
int side1,side2 ; {                      /* corner formed by s1 & s2*/
return(quads[(4*side1) - 4 + side2]) ;
}

cside(side1,side2)                       /* common side             */
int side1,side2 ;{
return(coms[(4*side1) - 4 + side2]) ;
}


son(node,qd)                             /* returns appropriate sin  */
int *node,qd ;{
if (qd < 1)
  {
   error = -1 ;
   return(-1) ;
  }
else return(node[qd+2]) ;
}

nwson(node)
int *node ;{
return(node[3]) ;
}

neson(node)
int *node ;{
return(node[4]) ;
}

seson(node)
int *node ;{
return(node[5]) ;
}

swson(node)
int *node ;{
return(node[6]) ;
}

sontype(nd)
int *nd ; {
int f,s1,s2,s3,s4,node ;
node = nd ;
if (nd == tree) return(bsun) ;

f = father(nd) ;
s1 = nwson(f) ;
s2 = neson(f) ;
s3 = seson(f) ;
s4 = swson(f) ;
if (nwson(f) == node) return(1) ;
else if (neson(f) == node) return(2) ;
else if (seson(f) == node) return(3) ;
else if (swson(f) == node) return(4) ;
else
    {
     printf("error in sontype\n") ;
     error = -2 ;
     cexit() ;
    }
}

opside(side)
int side; {
return(opp[side]) ;
}

opquad(qd)
int qd ;{
return(opp[qd]) ;
}


reflect(side,qd)
int side,qd ;{
if (qd < 1)
  {
   error = -3 ;
   return(-1) ;
  }
else return(refl[4*qd - 4 + side]) ;
}

leaf(node)                               /* is arg a leaf ?         */
int *node ; {
return(node[3] == 0 &&
       node[4] == 0 &&
       node[5] == 0 &&
       node[6] == 0
      ) ;
}


gray(node)
int *node ;{
return(node[0] != black && node[0] != white) ;
}

notnull(node)
int *node ;{
return(node != tree) ;
}

dark(node)
int *node ;{
return(leaf(node) && node[0] == black) ;
}


findn(node,side)
int *node,side ; {
int *q,f,s,r,x,y ;

f = father(node) ;
s = sontype(node) ;
x = adj(side,s) ;
y = notnull(f) ;
/*
printf("1) father: %d son: %d side: %d adj: %d nnul: %d\n",f,s,side,x,y) ;
*/
if (notnull(f) && adj(side,s))
  {
   q = findn(f,side) ;
  }
else q = f ;

x = notnull(q) ;
y = gray(q) ;
/*
printf("2) node: %d gray: %d nnul: %d\n",q,y,x) ;
*/
if (gray(q))
  {
   r = reflect(side,s) ;
   q = son(q,r) ;
  }
return(q) ;
}

findc(node,corner)
int *node,corner ;{
int *q,f,s,r,x,y,z,c ;

f = father(node) ;
s = sontype(node) ;
y = notnull(f) ;
z = opquad(corner) ;
c = cside(s,corner) ;

if (notnull(f) && (s != z))
  if (s == corner) q = findc(f,corner) ;
  else q = findn(f,c) ;
else q = f ;

if (gray(q))
  {
   r = opquad(s) ;
   q = son(q,r) ;
  }
return(q) ;
}




pointers(node)
int *node ;{
if (dark(node))
  {
   node[7] = findn(node,1) ;
   node[8] = findn(node,2) ;
   node[9] = findn(node,3) ;
   node[10] = findn(node,4) ;
   node[11] = findc(node,1) ;
   node[12] = findc(node,2) ;
   node[13] = findc(node,3) ;
   node[14] = findc(node,4) ;
  }
else if (gray(node))
  {
   pointers(node[3]) ;
   pointers(node[4]) ;
   pointers(node[5]) ;
   pointers(node[6]) ;
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
   pointers(tree) ;
/*
   for (i=1 ;i< 5 ; i++)
      {
       j = findc(tree[3],i) ;
       printf("*** %d\n",j) ;
      }
*/
   file = creat(options[2],0777) ;
   writeq() ;                                   /* write to disc   */
   cexit() ;
}
