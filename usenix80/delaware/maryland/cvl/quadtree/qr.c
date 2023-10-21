#

#include        "qpars"         /* parameters           */
/* quad tree relaxation (1)
*  program to match shapes by relaxation using two quad trees
*/

extern cgoof ;
int i,j,k,l,m,pts,rx,ry,x,y,sbx,sby,const1,const2,score ;
double csum,c,mc,total ;
int mlevel,nblevel,mbx,mby,nbx,nby,matches,nbrs,pnode,qnode,nnode ;
int mnodes,maximal,nbrx,nbry,white,black,mptr,nptr,tol,tf ;
int temp,mcnt,mcount,minx,miny,maxx,maxy,px,py,shftx,shfty ;
float tmp1,tmp2,tmp3,tmp4,hsize,ksize,count,bcount,pscore,fact,ratio ;

int profile[10] ;               /* no.of nodes on level */
int p[MNODES] ;                 /* vector of mnode adrs */
int q[MNODES][MATCHES] ;        /* vector of macth adrs */
float r[MNODES][MATCHES] ;      /* relaxation network   */
int mtchs[MNODES] ;             /* matches for each MN  */
int peaks[PX][PY] ;             /* display of scores etc*/
char work[64][64] ;             /* work space           */
int file[2] ;                   /* two tree files       */
int head1[10],head2[10] ;       /* file header buffers  */
int *tree1,*tree2 ;             /* the root             */
int t1[5500] ;                  /* space for tree 1     */
int t2[5500] ;                  /* space for tree 2     */
int bounds 5500 ;               /* max tree size        */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */
int s[] {64,32,16,8,4,2,1,0} ;  /* can do better        */
int cellsize 16 ;

int area[16],gx,gy ;            /* Grinnell fodder      */

/*
* routine to read quad tree disc file and make
* a usable tree out of it
*/

readq(space,head,n)
int *space,*head,n ;{

int base,blocks,k1 ;
base = &space[0] ;
if ((read(file[n],head,10)) == -1)
  {
   printf("error in reading tree file\n") ;
   cexit() ;
  }
blocks = head[0] ;
white = head[1] ;
black = head[2] ;
cellsize = head[3] ;
temp = blocks*128 - bounds ;
if (blocks*128 > bounds)
  {
   printf("tree too large %d\n",temp) ;
   cexit() ;
  }
for (i=0 ;i< blocks ; i++)
   {
    if ((read(file[n],&space[i*128],256)) == -1)
      {
       printf("error in reading block %d\n",i) ;
       cexit() ;
      }
   }
for (i=0 ;i< bounds ; i++)
   if (space[i] >= 500) space[i] = space[i] + (base - 500) ;

}


/*
*  routine to write quad tree to disc file in relative form
*/
writeq(space,head,n)
int *space,*head,n ;{
int base,blocks ;
base = &space[0] ;
blocks = head[0] ;
if ((write(file[n],head,10)) == -1)
  {
   printf("couldn't write roped file\n") ;
   cexit() ;
  }
for (i=0 ;i<bounds ;i++)
   if (space[i] >= base) space[i] = space[i] - base + 500 ;

for (i=0 ;i< blocks ; i++)                           /* write to disc*/
    if (write(file[n],&space[i*128],256) < 0)
      {
       printf("couldn't write block %d of qfile\n",i) ;
       cexit() ;
      }
}

leaf(node)                               /* is arg a leaf ?         */
int *node ; {
return(node[3] == 0 &&
       node[4] == 0 &&
       node[5] == 0 &&
       node[6] == 0
      ) ;
}

/*
* routine to get the position of the current node in
* in the MMNODES or MATCHES array. The offset returned
* is used to index the element in the relaxation array r
* which is being currently updated.
*/

getp(nadr)
int nadr ;{
int k1 ;
for (k1=0 ;k1<MNODES ;k1++)
   if (p[k1] == nadr) return(k1) ;
}

getq(nadr)
int nadr ;{
int k1 ;
for (k1=0 ;k1<MATCHES ;k1++)
   if (q[rx][k1] == nadr) return(k1) ;
}

/*
* routine to get the next maximal node of tree1
* sets mbx,mby and mlevel
*/

getmax(tx,ty,lx,ly,node,lev,numb)
int tx,ty,lx,ly,lev,numb,*node ;{
if (node[15] == numb)
  {
   mbx = lx ;
   mby = ly ;
   mlevel = node[1] ;
   pnode = node ;
  }
else if (leaf(node)) return ;
else
    {
     getmax(tx,ty,tx+b[lev],ty-b[lev],node[3],lev+1,numb) ;
     getmax(tx+b[lev],ty,lx,ty-b[lev],node[4],lev+1,numb) ;
     getmax(tx+b[lev],ty-b[lev],lx,ly,node[5],lev+1,numb) ;
     getmax(tx,ty-b[lev],tx+b[lev],ly,node[6],lev+1,numb) ;
    }
}

/*
* routine to get the next nbr of the current maximal node
*/

getnbr(tx,ty,lx,ly,node,lev,numb)
int tx,ty,lx,ly,lev,numb,*node ;{
if (node[8] == numb)
  {
   nbrx = tx ;  /* bottom left corner is (tx,ly) */
   nbry = ly ;
   nblevel = node[1] ;
   node[8] = 0 ;
   nnode = node ;
  }
else if (leaf(node)) return ;
else
    {
     getnbr(tx,ty,tx+b[lev],ty-b[lev],node[3],lev+1,numb) ;
     getnbr(tx+b[lev],ty,lx,ty-b[lev],node[4],lev+1,numb) ;
     getnbr(tx+b[lev],ty-b[lev],lx,ly,node[5],lev+1,numb) ;
     getnbr(tx,ty-b[lev],tx+b[lev],ly,node[6],lev+1,numb) ;
    }
}

nbrof(node)
int *node;{
return(node[1] >= mlevel-1 && node[1] <= mlevel+1 && node[15] != 0) ;
}

/*
*  routine to mark node if it is a possible neighbour of
*  the current maximal node
*/

markn(node,maxnd)
int *node,maxnd ;{
if (node[15] != 0 && node[15] != maxnd
    && node[1] >= mlevel-1
    && node[1] <= mlevel+1
   )
   node[8] = nbrs++ ;
if (leaf(node)) return ;
else
    {
     markn(node[3],maxnd) ;
     markn(node[4],maxnd) ;
     markn(node[5],maxnd) ;
     markn(node[6],maxnd) ;
    }
}

/*
* routine to mark all possible matches for the current
* maximal node
*/

markm(node)
int *node ;{
if (node[15] != 0 && node[1] >= mlevel-1 && node[1] <= mlevel+1)
  node[7] = matches ++ ;
if (leaf(node)) return ;
else
    {
     markm(node[3]) ;
     markm(node[4]) ;
     markm(node[5]) ;
     markm(node[6]) ;
    }
}


/*
*  routine to match the current maximal node to one of its
*  possible matches in t2 and set up co-ordinates of the
*  matched node. These will be used later for computing the
*  shift and doing the mapping.
*/

match(tx,ty,lx,ly,node,lev,numb)
int *node,tx,ty,lx,ly,lev,numb ;{
if (node[7] == numb)
  {
   nbx = lx ;
   nby = ly ;
   node[7] = 0 ;
   qnode = node ;
  }
else if (leaf(node)) return ;
else
    {
     match(tx,ty,tx+b[lev],ty-b[lev],node[3],lev+1,numb) ;
     match(tx+b[lev],ty,lx,ty-b[lev],node[4],lev+1,numb) ;
     match(tx+b[lev],ty-b[lev],lx,ly,node[5],lev+1,numb) ;
     match(tx,ty-b[lev],tx+b[lev],ly,node[6],lev+1,numb) ;
    }
}


/*
*  routine to map node of size nblevel with origin at (nbx,nby)
*  to work array with origin at (sbx,sby) .
*/

map() {
int k1,k2,x1,y1,newx,newy ;

for (k1=0 ;k1<64 ;k1++)
   for (k2=0 ;k2<64 ;k2++)
      work[k1][k2] = 0 ;

/* find the shift */
sbx = mbx - nbx ;
sby = mby - nby ;
/* find max & min shifts */
if (sbx < minx) minx = sbx ;
if (sbx > maxx) maxx = sbx ;
if (sby < miny) miny = sbx ;
if (sby > maxy) maxy = sbx ;

/* now set offset in peak array */
px = sbx + (PX-1)/2 ;
py = sby + (PY-1)/2 ;

/* find new coordinates of bottom left corner */
newx = nbrx + sbx - tol ;  /* allow tolerence */
newy = nbry + sby - tol ;
/* do the mapping */
for (k1=0 ;k1< s[nblevel]+2*tol ;k1++)
   for (k2=0 ;k2< s[nblevel]+2*tol ;k2++)
      {
       x1 = newx + k1 ;
       y1 = newy + k2 ;
       if (x1 < 64 && y1 < 64 && x1>0 && y1>0) work[x1][y1] ++ ;
      }
}

/*
*  routine to calculate the max contribution of (Ph,Qk)s to
*  the current pairing (Pi,Qj)
*/

contrib(tx,ty,lx,ly,node,lev)
int tx,ty,lx,ly,lev,*node ; {
int k1,k2,x1,y1 ;
if (node[14] == 1)
  {
   count = bcount = 0.0 ;
   for (k1=0 ;k1< s[lev]+2*tol ;k1++)
      for (k2=0 ;k2< s[lev]+2*tol ;k2++)
	 {
	  x1 = tx + k1 - tol ;
	  y1 = ly + k2 - tol ;
	  if (x1 > 0 && x1 < 64 && y1 > 0 && y1 < 64)
	    {
	     bcount = bcount + 1.0 ;
	     if (work[x1][y1] != 0) count = count + 1.0 ;
	    }
	 }
   /* find size of Ph and Qk */
   hsize = s[nblevel] ;
   ksize = s[lev] ;
   if (hsize >= ksize) c = count/bcount ;
   else if (count == 0.0) c = 0.0 ;
   else c = hsize/count ;
/* else c = (count/bcount) * (hsize/ksize) ;    */

  }
if (c > mc) mc = c ;
if (leaf(node)) return ;
else
    {
     contrib(tx,ty,tx+b[lev],ty-b[lev],node[3],lev+1) ;
     contrib(tx+b[lev],ty,lx,ty-b[lev],node[4],lev+1) ;
     contrib(tx+b[lev],ty-b[lev],lx,ly,node[5],lev+1) ;
     contrib(tx,ty-b[lev],tx+b[lev],ly,node[6],lev+1) ;
    }
}


/*
* routine to do the relaxation as defined in the file 'doc'
*  under this directory.
*  mlevel   - level of current maximal node
*  sbx      - shift in the x direction
*  sby      - shift in y direction
*  matches  - possible matches for current maximal node in t2
*  nbrs     - possible neighbours for current maximal node in t1
*  pnode    - current maximal node in t1
*  qnode    - current contrib
*  MNODES   - number of maximal nodes in t1
*  MATCHES  - all possible matches in t2 (note difference from 'matches')
*/

relax() {
int k1,k2,k3,k4 ;

for (k1= 0; k1 < MNODES ;k1++)          /* for all maximal nodes        */
   {
    mlevel = sbx = sby = 0 ;            /* get next maximal node        */
    getmax(0,64,64,0,t1,0,k1+1);        /* mlevel,mbx,mby set up        */
/*  printf("mnode: %d mlevel: %d mbx,mby: %d,%d\n",(pnode-const1),mlevel,
						mbx,mby) ;
*/
    matches = 1 ;
    markm(t2) ;                         /* mark possible matches in t2  */
/*  printf("MNODE %d has %d matches (%d)\n",k1,matches-1,mlevel) ;      */
    for (k2=0 ;k2 <matches-1;k2++)      /* for all matches              */
       {
	match(0,64,64,0,t2,0,k2+1) ;    /* make this match & find shift */
	nbrs = 1 ;                      /* init neighbours count        */
	markn(t1,k2+1) ;                /* mark possible nbrs of mnode  */
/*      printf("MNODE %d has %d nbrs\n",k1,nbrs-1) ;    */
	pscore = csum = 0.0 ;           /* support for (Pi,Qj) = 0.0    */
/*      printf("   shift for match %d is %d %d\n",k2,mbx-nbx,mby-nby) ; */
	for (k3=0 ;k3 < nbrs-1 ;k3 ++)  /* for all nbrs,get contrib.    */
	   {
	    getnbr(0,64,64,0,t1,0,k3+1);/* nblevel,nbrx,nbry set        */
	    map() ;                     /* map to work array using shift*/
	    c = mc = 0.0 ;              /* init contribution            */
	    contrib(0,64,64,0,t2,0) ;   /* get maximum contribution     */
	    csum = csum + mc ;          /* and add to total             */
	    pscore =+ csum ;
/*          printf("      csum = %f\n",csum) ;                          */
	   }
	peaks[px][py] = pscore * 10 ;
	rx = getp(pnode) ;              /* rx,ry define element of the  */
	ry = getq(qnode) ;              /* relaxation network to update */
/*      printf("         rx,ry = %d %d\n",rx,ry) ;                      */
	total = nbrs - 1 ;              /* C allowance                  */
	if (total != 0)
	   csum = csum / total ;        /* normalize contribution       */
	if (r[rx][ry] > csum)           /* s(Pi,Qj) =                   */
	  r[rx][ry] = csum ;            /*      min[max(s(Pi,Qj),csum)] */
       }
   }
}

/*
* routine to fill up MNODES address vector
*/
fillp(node)
int *node ;{
if (mptr > MNODES)
  {
   printf("too many maximal nodes to match: %d\n",mptr) ;
   cexit() ;
  }
if (node[15] != 0)
  {
   p[mptr++] = node ;
/* printf("maximal node is %d\n",p[mptr-1] - const1) ;  */
   nptr = 0 ;
   fillq(t2,node[1]) ;
   mtchs[mptr-1] = nptr ;
  }
if (leaf(node)) return ;
else
    {
     fillp(node[3]) ;
     fillp(node[4]) ;
     fillp(node[5]) ;
     fillp(node[6]) ;
   }
}

/*
* routine to fill up MATCHES address vector
*/
fillq(node,lvl)
int *node,lvl ;{
if (nptr > MATCHES)
  {
   printf("too many possible matches %d %d\n",nptr,node[1]) ;
   nptr = 0 ;
  }
if (node[15] != 0)
  q[mptr-1][nptr++] = node ;

if (leaf(node)) return ;
else
    {
     fillq(node[3],lvl) ;
     fillq(node[4],lvl) ;
     fillq(node[5],lvl) ;
     fillq(node[6],lvl) ;
   }
}

/*
* routine to number the maximal nodes
*/
mnumb(node)
int *node ;{
if (node[15] == 1)
  {
   node[15] = mcnt ++ ;
   node[14] = 1 ;
  }
else if (leaf(node)) return ;
else
    {
     mnumb(node[3]) ;
     mnumb(node[4]) ;
     mnumb(node[5]) ;
     mnumb(node[6]) ;
    }
}

/*
* initialization routine
* this routine will fill vectors for all MNODES & MATCHES
* addresses. Vectors are used because for each possible
* pairing (MNODE,MATCH) a coordinate (x,y) can be found
* in the relaxation array r[MNODES][MATCHES]. See TR-702
* for further details.
* MLEVEL is the highest level on which a maximal node occurs in t1
* LLEVEL is the lowest level on which a maximal occurs in t1
*/

init() {
int k1,k2 ;
mptr = nptr = 0 ;
const1 = &t1[0] - 500 ;
const2 = &t2[0] - 500 ;
fillp(t1) ;  /* set up MNODES address vector */
for (k1=0 ;k1<MNODES ;k1++)
   {
    if (p[k1] != 0)
      {
       printf("\n%d : ",p[k1] - const1) ;
       for (k2=0 ;k2<MATCHES ;k2++)
	  printf("%d ",q[k1][k2] - const2) ;
      }
   }
/* now initialize the 'probabilities' */
for (k1=0 ;k1<MNODES ;k1++)
   for (k2=0 ;k2<MATCHES ;k2++)
      r[k1][k2] = 1.0 ;
/* now number maximal nodes */
mcnt = 1 ;
mnumb(t1) ;
mcnt = 1 ;
mnumb(t2) ;

}

results(){
int k1,k2,k3,k4,val ;
score = 0 ;

for (k1=0 ;k1<MNODES ;k1++)
   {
    printf("MNODE: %d : ",(p[k1] - const1)) ;
    k4 = 0 ;
	for (k3=0 ;k3< 10 ;k3++)
	   if (k4 < mtchs[k1])
	     printf("%f ",r[k1][k4++]) ;
	   else printf("........ ") ;
    putchar('\n') ;
   }

putchar('\n') ;
printf("\n    DISPLACEMENT SCORES\n\n") ;
for (k1=PX-1;k1 > -1; k1--)
   {
    putchar('\n') ;
    for (k2=0 ;k2<PY ;k2++)
       {
	val = peaks[k1][k2] ;
	printf("%3d",val) ;
	if (val > score)
	  {
	   shftx = k1 - (PX-1)/2 ;
	   shfty = k2 - (PX-1)/2 ;
	   score = val ;
	  }
       }
   }




}


check(node,numb)
int *node,numb ;{

if (node[15] != 0) profile[node[1]] ++ ;
if (leaf(node)) return ;
else
    {
     check(node[3]) ;
     check(node[4]) ;
     check(node[5]) ;
     check(node[6]) ;
    }
}



/* control section         */

main(argc,options)
int argc ;
char **options ; {
int k1 ;
if (argc < 3)
  {
   printf("usage : qr tree1 tree2 <tf>\n") ;
   cexit() ;
  }
if ((file[0] = open(options[1],2)) == -1)
  {
   printf("error in opening file : %s\n",options[1]) ;
   cexit() ;
  }
readq(t1,head1,0) ;
if ((file[1] = open(options[2],2)) == -1)
  {
   printf("error in opening file : %s\n",options[2]) ;
   cexit() ;
  }
readq(t2,head2,1) ;
if (argc == 4) tol = atoi(options[3]) ;
/* trees read in, now print out heading */

printf("\nMATCHING %s TO %s WITH TF =  %d\n\n",options[1],options[2],tol) ;
printf("MNODE       MATCHES\n") ;
init() ;                                /* initialize   */
putchar('\n') ;                         /* profile the  */
check(t1) ;                             /* tree & print */
printf("\nprofile: ") ;                 /* it           */
for (k1=0 ;k1< 8 ;k1++)
   printf("%d ",profile[k1]) ;
printf("\n\n") ;

relax() ;                               /* relax        */
results() ;                             /* print out res*/
printf("\nhorizontal min & max shifts : %d %d\n",minx,maxx) ;
printf("\nvertical min & max shifts : %d %d\n",miny,maxy) ;
printf("\nestimated shift is (%d,%d)  score: %d\n",shfty,shftx,score) ;

cexit() ;
}
