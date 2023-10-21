/* quad tree display (1)
*
*  program to display
*  black nodes up to a given level  or
*  gray nodes at a given level
*
*/

extern cgoof ;
int i,j,k,l,m,n,temp,pts,tfile,pfile,nlev,type,cellsize,min,p1 ;
double dsum,rsum,dtemp,white,black ;

char pic[64][64] ;              /* input buffer         */
int  head[12] ;                 /* tree header buffer   */
char phead[] {0,0,64,0,0,0,64,0,0,0,1,0} ;
int *tree ;                     /* the root             */
int space[9800] ;               /* space for many nodes */
int bounds 9800 ;               /* available space      */
int b[] {32,16,8,4,2,1,0} ;     /* offset table         */
int oerr[4] ;                   /* error table          */

init() {                        /* init picture         */

for (i=0 ;i< 64 ; i++)
   for (j=0 ;j< 64 ; j++)
      pic[i][j] = 0 ;
}


readq(){                        /* read tree            */
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

for (i=topx ;i< botx ; i++)
   for (j=boty ; j<topy ;j++)
      pic[i][j] ++ ;
}

dark(node)
int *node ; {
return(node[0] == black) ;
}


flash(tx,ty,lx,ly,node,lev)
int tx,ty,lx,ly,lev ;
int *node ; {
if (leaf(node) && node[0] == black && lev <= nlev && type == 0)
  wither(tx,ty,lx,ly,63) ;
else if (leaf(node)) return ;
else
    {
     if (lev == nlev && type == 1) wither(tx,ty,lx,ly,63) ;
     flash(tx,ty,tx+b[lev],ty-b[lev],node[3],lev+1) ;
     flash(tx+b[lev],ty,lx,ty-b[lev],node[4],lev+1) ;
     flash(tx+b[lev],ty-b[lev],lx,ly,node[5],lev+1) ;
     flash(tx,ty-b[lev],tx+b[lev],ly,node[6],lev+1) ;
    }
}


err(num)
int num ;{

for (i=0 ; i< 64 ;i++)
   for (j=0 ; j< 64 ;j++)
      if (num == 0 && pic[i][j] == 1) oerr[0] ++ ;
      else if (num == 1 && pic[i][j] != 0 && nlev != 6) oerr[1] ++ ;
}

writep() {

if ((write(pfile,phead,12)) == -1)
  {
   printf("couldn't write picture header\n") ;
   cexit() ;
  }
if ((write(pfile,pic,4096)) == -1)
  {
   printf("couldn't write picture\n") ;
   cexit() ;
  }
}



/* control section         */

main(argc,argv)
int argc ;
char **argv ; {

if (argc < 3)
  {
   printf("usage : qapx tree1 tree2 [befile] [gefile]\n") ;
   cexit() ;
  }
if (argc > 3)
  if ((pfile = open(argv[3],0)) == -1)
    {
     printf("couldn't open %s\n",argv[3]) ;
     cexit() ;
    }

printf("Match error: %s to %s\n",argv[1],argv[2]) ;
printf("\nlev   min   max\n") ;

for (nlev=1 ;nlev < 7 ; nlev ++)
   {                                                    /* all levels   */
    oerr[0] = oerr[1] = 0 ;
    for (type=0 ;type < 2 ; type ++)
	{                                               /* both errors  */
	 init() ;
	 for (p1=0 ;p1 < 2 ;p1++)
	    {
	     if ((tfile = open(argv[p1+1],0)) == -1)
	       {
		printf("couldn't open %s\n",argv[p1+1]) ;
		cexit() ;
	       }
	     readq() ;
	     close(tfile) ;
	     flash(0,64,64,0,tree,0) ;
/*
	     for (i=0 ;i< 64 ; i++)
		{
		 putchar('\n') ;
		 for (j=0 ;j< 64 ; j++)
		    printf("%d",pic[i][j]) ;
		}
*/
	    }
	 err(type) ;
	}
    oerr[1] = oerr[1] + oerr[0] ;
    if (oerr[0] == 0)
      printf("%3d   no black nodes\n",nlev) ;
    else printf("%3d   %3d   %3d\n",nlev,oerr[0],oerr[1]) ;
   }
cexit() ;
}

