/* quad tree display (1)
   program to make a quad tree from a given input picture (64 x 64)
   args:   a1 = quad tree file (produced by 'quad' ..etc)
	   a2 = base (x) of Grinnell window
	   a3 = base (y) of Grinell window
	   a4 = 0 if unroped , 1 if roped

   header : w1 = blocks reqd to store tree
	    w2 = white gray level
	    w3 = black gray level
	    w4 = cell size of node
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,file1,file2,cr,ofile ;
int ipts,jpts,nodes,offset,gx,gy,error,size,words ;
double dsum,rsum,dtemp,white,black ;
char option ;

int area[16] ;                  /* Grinnell area        */
char buf[64] ;                  /* input buffer         */
int ibuf[64] ;                  /* row output buffer    */
int  head[8] ;                  /* file header buffer   */
int *tree ;                     /* the root             */
int *fptr ;                     /* pointer to new node  */
int space[9800] ;               /* space for many nodes */
int bound 9800 ;                /* available space      */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */
int cellsize 16 ;               /* size of node         */
char phead [] {0,0,64,0,0,0,64,0,0,0,1,0} ; /* pic header   */
readq(){
int base,blocks ;
base = &space[0] ;
if ((temp = read(file1,head,10)) == -1)
  {
   printf("error in reading tree file\n") ;
   cexit() ;
  }
blocks = head[0] ;
white = head[1] ;
black = head[2] ;
cellsize = head[3] ;

if (bound < blocks*128)
  {
   printf("tree too large\n") ;
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

show(items)                      /* dumps space in 14s */
int items ; {
printf("show %d\n",items) ;
k = 0 ;
for (i=0 ;i<items ; i++)
   {
    for (j=0 ;j<16 ;j++)
       printf("%d ",space[k++]) ;
    printf("\n") ;
   }
}

disp(tx,ty,lx,ly,node,node1,type,lev)
int tx,ty,lx,ly,type,lev ;
int *node,*node1 ; {
int pt ;
if (bleaf(node)) return ;               /* white leaf           */
else if (wleaf(node))
       {
	if (cr >=ly && cr <= ty)        /* current row ok       */
	  for (pt=tx ;pt<lx ;pt++)
	      ibuf[pt] = white ;
       }
else
    {
     disp(tx,ty,tx+b[lev],ty-b[lev],node[3],node,1,lev+1) ;
     disp(tx+b[lev],ty,lx,ty-b[lev],node[4],node,2,lev+1) ;
     disp(tx+b[lev],ty-b[lev],lx,ly,node[5],node,3,lev+1) ;
     disp(tx,ty-b[lev],tx+b[lev],ly,node[6],node,4,lev+1) ;
    }
}

wleaf(node)
int *node; {
return(node[0] == white) ;
}

bleaf(node)
int *node ; {
return(node[0] == black) ;
}


/* control section         */

main(argc,options)
int argc ;
char **options ; {
if (argc < 3)
  {
   printf("usage : qdispr qtree file [m]\n") ;
   cexit() ;
  }
if ((file1 = open(options[1],2)) == -1)
  {
   printf("error in opening file : %s\n",options[1]) ;
   cexit() ;
  }
if ((ofile = creat(options[2],0777)) == -1)
  {
   printf("couldn't create %s\n",options[2]) ;
   cexit() ;
  }
if ((write(ofile,phead,12)) == -1)
  {
   printf("couldn't write header to %s\n",options[2]) ;
   cexit() ;
  }
readq() ;
for (cr=0 ;cr<64 ; cr++)
   {
    for (i=0 ;i<64 ;i++)
       ibuf[i] = 0 ;
    disp(0,64,64,0,tree,tree,0,0) ;
    for (i=0 ;i<64 ;i++)
       buf[i] = ibuf[i] << 6 ;
    if ((write(ofile,buf,64)) == -1)
      {
       printf("couldn't write row %d to %s\n",cr,options[2]) ;
       cexit() ;
      }
   }
cexit() ;
}
