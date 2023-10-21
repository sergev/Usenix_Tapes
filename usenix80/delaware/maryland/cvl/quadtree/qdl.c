/* DISPLAY QUAD TREE LEVEL
*  Used in TR-847
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,file1,file2 ;
int ipts,jpts,nodes,offset,gx,gy,error,size,words ;
double dsum,rsum,dtemp,white,black ;
char option ;

int area[16] ;                  /* Grinnell area        */
char buf[64] ;                  /* input buffer         */
int  head[10] ;                 /* file header buffer   */
int *tree ;                     /* the root             */
int *fptr ;                     /* pointer to new node  */
int space[9800] ;               /* space for many nodes */
int bounds 9800 ;               /* available space      */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */
int cellsize 16 ;               /* size of node         */
int maximal 0 ;                 /* flag for display typ */
int mnodes 0 ;                  /* maximal nodes        */
int dlevel 1 ;                  /* level to display     */
int glevel 0 ;                  /* gray level to disp   */

readq(){
int base,blocks ;
base = &space[0] ;
if ((read(file1,head,10)) == -1)
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
    if ((read(file1,&space[i*128],256)) == -1)
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
gray = gl ;
gray = gray << 8 ;
/* printf("wither %d %d %d %d %d\n",topx,topy,botx,boty,gray) ; */
for (i=topx ;i< botx ; i++)
   for (j=boty ; j<topy ;j++)
      gwpnt(area,gray,j,i) ;
}

dark(node)
int *node ; {
return(node[0] == black) ;
}


flash(tx,ty,lx,ly,node,node1,type,lev)
int tx,ty,lx,ly,type,lev ;
int *node,*node1 ; {
if (leaf(node) && node[0] == black && dlevel == node[1])
  {
   mnodes ++ ;
   wither(tx,ty,lx,ly,glevel) ;
  }
else if (leaf(node)) return ;
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
if (argc < 6)
  {
   printf("usage : qdisp qfile gwinx gwiny level gl\n") ;
   cexit() ;
  }
if ((file1 = open(options[1],0)) == -1)
  {
   printf("error in opening file : %s\n",options[1]) ;
   cexit() ;
  }
readq() ;
gx = atoi(options[2]) ;
gy = atoi(options[3]) ;
dlevel = atoi(options[4]) ;
glevel = atoi(options[5]) ;
gopen(area,gx,gy,64,64) ;

flash(0,64,64,0,tree,tree,0,0) ;
printf("%d level %d maximal nodes found & displayed\n",mnodes,dlevel) ;
cexit() ;
}
