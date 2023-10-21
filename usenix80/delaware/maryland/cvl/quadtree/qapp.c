/* quad tree display (1)
   program to make a quad tree from a given input picture (64 x 64)
   input picture (with gxap header) is in /mnt/class/smr/trees/blob1
   gray level < 5 white >15 black , this limits size of tree
   full of inefficient code , needs to be cleaned up
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,tfile,pfile ;
int ipts,jpts,nodes,offset,gx,gy,error,size,words ;
double dsum,rsum,dtemp,white,black ;
char option ;

int area[16] ;                  /* Grinnell area        */
char pic[64][64] ;              /* display picture      */
int phead[6] {0,64,0,64,0,1} ;  /* picture header buf   */
int  head[10] ;                 /* file header buffer   */
int *tree ;                     /* the root             */
int *fptr ;                     /* pointer to new node  */
int space[9800] ;               /* space for many nodes */
int bounds 9800 ;               /* available space      */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */
int cellsize 16 ;               /* size of node         */
int maximal 0 ;                 /* flag for display typ */
int mnodes 0 ;                  /* maximal nodes        */

readq(){
int base,blocks ;
base = &space[0] ;
if ((read(tfile,head,10)) == -1)
  {
   printf("error in reading tree file\n") ;
   cexit() ;
  }
blocks = head[0] ;
white = head[1] ;
black = head[2] ;
cellsize = head[3] ;
temp = 256*bounds ;
if (blocks*128 > bounds)
  {
   printf("tree too large %d\n",temp-bounds) ;
   cexit() ;
  }
for (i=0 ;i< blocks ; i++)
   {
    if ((read(tfile,&space[i*128],256)) == -1)
      {
       printf("error in reading block %d\n",i) ;
       cexit() ;
      }
   }
for (i=0 ;i< 9800 ; i++)
   if (space[i] >= 500) space[i] = space[i] + (base - 500) ;

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


/* this section displays the */
/* tree on the GRINNELL      */
/* inverse of 'grow'         */

wither(topx,topy,botx,boty,gl)
int topx,topy,botx,boty,gl ; {
int gray ;
for (i=topx ;i< botx ; i++)
   for (j=boty ; j<topy ;j++)
      pic[i][j] = gl ;
}

dark(node)
int *node ; {
return(node[0] == black) ;
}


flash(tx,ty,lx,ly,node,node1,type,lev)
int tx,ty,lx,ly,type,lev ;
int *node,*node1 ; {
if (leaf(node))
  {
   if (maximal == 99)
     {
      if (node[15] == 1)
	{
	 mnodes ++ ;
	 wither(tx,ty,lx,ly,0) ;
	}
      else wither(tx,ty,lx,ly,63) ;
     }
   else
     {
      if (dark(node)) wither(tx,ty,lx,ly,0) ;
      else wither(tx,ty,lx,ly,63) ;
     }
  }
else
    {
     flash(tx,ty,tx+b[lev],ty-b[lev],node[3],node,1,lev+1) ;
     flash(tx+b[lev],ty,lx,ty-b[lev],node[4],node,2,lev+1) ;
     flash(tx+b[lev],ty-b[lev],lx,ly,node[5],node,3,lev+1) ;
     flash(tx,ty-b[lev],tx+b[lev],ly,node[6],node,4,lev+1) ;
    }
}


/* control section         */

main(argc,options)
int argc ;
char **options ; {
if (argc < 3)
  {
   printf("usage : qapp qtree qpic < m >\n") ;
   cexit() ;
  }
if ((tfile = open(options[1],0)) == -1)          /* open tree file      */
  {
   printf("couldn't open %s\n",options[1]) ;
   cexit() ;
  }
if ((pfile = creat(options[2],0777)) == -1)      /* open picture file   */
  {
   printf("couldn't create %s\n",options[2]) ;
   cexit() ;
  }
readq() ;                                        /* read in tree        */

if (options[3][0] == 'm') maximal = 99 ;         /* disp maximal only ? */

flash(0,64,64,0,tree,tree,0,0) ;                 /* make the picture    */

if ((write(pfile,phead,12)) == -1)
  {
   printf("couldn't write header %s\n",options[2]) ;
   cexit() ;
  }

if ((write(pfile,pic,4096)) == -1)
  {
   printf("couldn't write picture %s\n",options[2]) ;
   cexit() ;
  }

if (maximal == 99)
  printf("%d maximal nodes found in tree\n",mnodes) ;

cexit() ;
}
