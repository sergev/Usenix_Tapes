/*
Mark Papamarcos
EE348 Final Project (for graduate credit)

This program plays the game of Othello.  It employs an N-ply minimax
search with alpha-beta pruning.  It was decided to use C rather than
LISP due to its much more powerful array manipulation capabilities.
At each leaf node a static board evaluator is called, which roughly
sums one player's pieces times a relative value assigned to each
square and subtracts the value obtained for the other player's pieces.
There are also dependencies between certain squares- if a square is
occupied by either player, the weight of some other square might be
affected.  Finally, the weights of squares are dynamic- changing by
a fixed increment in each stage of the game.

Invocation:              
  othello {pPLY} {f} {a}

Where 'pPLY' sets the maximum search depth to PLY (default=3).
'f' forces the computer to move first (default=human first).
'a' sets automatic play mode.

Copyright (c) 1982, 1986 Mark Papamarcos (hplabs!ridge!valid!markp)
Permission is granted to do with this program whatever you damn well
please, although I expect you would have the decency to let me in on
any profits from commercial ventures :-).
*/

#include <stdio.h>

#define NO 0		/* logical values */
#define YES 1

#define PLY 3		/* default ply level */

#define HIS -1		/* board assignments to mark pieces */
#define OPEN 0
#define MINE 1

#define HISDUDE 'O'	/* what each position prints as */
#define NODUDE '.'
#define MYDUDE '*'

#define MAX 1		/* maximizing or minimizing flag */
#define MIN -1

                        /* Weights */
#define VCORNER 400	/* Corner squares are studdly */
#define VADJCOR -16	/* avoid adjacent ones in general */
#define VINCOR -80	/* But adjacent ones are to be avoided */
#define VCENT1 12	/* Very middle is mediocre */
#define VCENT2 20	/* center border is better */
#define VCENT3 40	/* center corner is very nice initially */
#define VEDGE1 28	/* Edges are nice */
#define VEDGE2 60	/* edge spaces 2 from corners really nice */
#define VADJEDGE 12	/* Next to edges are okay */

#define STAGE 5		/* weights change every 5 moves */

#define DVCOR -30	/* corners greatly decrease in value */
#define DVACOR 5	/* next to corners become more important */
#define DVICOR 5	/* same here */
#define DVEDGE1 0	/* middle edges stay same */
#define DVEDGE2 -4	/* good edges lose their advantage */
#define DVAEDGE 1	/* next to edges little better */
#define DVCENT1 0	/* middle stays same */
#define DVCENT2 0	/* center border stays same */
#define DVCENT3 -3	/* center corners lose their prominence */

#define LARGE 5000	/* a very large weight */

#define DEPEDGE 80	/* if corner taken, adjacent edges better */
#define DEPCOR 40	/* inside corner worth a little more */

static int di[8]= { 1, 1, 0,-1,-1,-1, 0, 1};	/* direction deltas */
static int dj[8]= { 0, 1, 1, 1, 0,-1,-1,-1};

/* weights defined for one quarter of the board */
static int wtab[4][4]= {{VCORNER,VADJCOR ,VEDGE2  ,VEDGE1  },
                        {VADJCOR,VINCOR  ,VADJEDGE,VADJEDGE},
			{VEDGE2 ,VADJEDGE,VCENT3  ,VCENT2  },
			{VEDGE1 ,VADJEDGE,VCENT2  ,VCENT1  }};

/* amount by which weights change every STAGE moves */
static int dwtab[4][4]={{DVCOR  ,DVACOR  ,DVEDGE2 ,DVEDGE1 },
			{DVACOR ,DVICOR  ,DVAEDGE ,DVAEDGE },
			{DVEDGE2,DVAEDGE ,DVCENT3 ,DVCENT2 },
			{DVEDGE1,DVAEDGE ,DVCENT2 ,DVCENT1 }};

int weight[8][8];	/* actual weights for 8x8 */
int dweight[8][8];	/* weight increments */

/* dependency factors defined for one quarter of the board */
static int dtab[4][4]= {{0      ,DEPEDGE ,0       ,0       },
			{DEPEDGE,DEPCOR  ,0       ,0       },
			{0      ,0       ,0       ,0       },
			{0      ,0       ,0       ,0       }};

int dependx[8][8];	/* dependency factor */
int dependi[8][8];	/* square that additional weight depends on */
int dependj[8][8];

int pcc[3];		/* piece counts */
			/* [0]=human, [1]=empty, [2]=computer */

int totalmoves;		/* moves considered */

int maxply;		/* search level */
int mptemp;		/* search level towards end */

int autoplay;		/* automatic play flag */

main(argc,argv)
int argc;
char *argv[];
{
  int arg;		/* argument count */
  int b[8][8];		/* current position */
  int i,j;		/* counters */
  int ti,tj;		/* temporaries */
  int mvi,mvj;		/* move */
  int endg;		/* endgame flag */
  int movefirst;	/* computer move first flag */
  int hecanmove;	/* human can move */
  int icanmove;		/* computer can move */

  maxply= PLY;		/* set default search level */

  movefirst= NO;
  autoplay= NO;

  /* Take care of arguments to set game parameters */
  arg= 0;
  while(++arg<argc)
    {
      switch(*argv[arg])
	{
	  case 'p':
	    sscanf(++argv[arg],"%d",&maxply);
	    if(maxply<1 || maxply>5)
	      {
		printf("Ply must be between 1 and 5.\n");
		return;
	      }
	    break;
	  case 'f':
	    movefirst= YES;
	    break;
	  case 'a':
	    autoplay= YES;
	    break;
	  default:
	    printf("Usage: othello pn f a\n");
	    return;
	}
    }

  mptemp= maxply;
  for(i=0;i<8;i++)	/* initialize board, weights */
    for(j=0;j<8;j++)
      {
	b[i][j]=OPEN;
	ti= (i>3) ? 7-i : i;
	tj= (j>3) ? 7-j : j;
	weight[i][j]= wtab[ti][tj];
	dweight[i][j]= dwtab[ti][tj];
	dependx[i][j]= dtab[ti][tj];
	if(dependx[i][j]==0)
	  {
	    dependi[i][j]= 0;
	    dependj[i][j]= 0;
	  }
	else
	  {
	    dependi[i][j]= (i<4) ? 0 : 7;
	    dependj[i][j]= (j<4) ? 0 : 7;
	  }
      }
  

  b[3][3]= HIS;		/* starting configuration */
  b[4][4]= HIS;
  b[3][4]= MINE;
  b[4][3]= MINE;

  bookkeep(b);

  printf("\nWelcome to Othello V1.0 (MSP)\n");
  printf("You are playing O.  I am playing *.\n\n");
  if(autoplay)
    printf("Automatic play begins.\n");

  endg= NO;
  while(!endg)
    {
      hecanmove= NO;
      icanmove= NO;
      board(b);
      if(!movefirst)
	{
	  if(!genmove(b,HIS))
	    printf("You have no legal moves.\n");
	  else
	    {
	      hecanmove= YES;
	      if(!autoplay)
		{
		  getmove(b,&mvi,&mvj);
		}
	      else
		{
		  ply(b,HIS,MAX,LARGE,&mvi,&mvj,mptemp,HIS);
		  printf("You take %d%d.\n",mvi,mvj);
		  flip(b,mvi,mvj,HIS,b);
		}
	      bookkeep(b);
	      board(b);
	    }
	}
      else
	{
	  movefirst= NO;
	  bookkeep(b);
	}
      if(!genmove(b,MINE))
	printf("I have no legal moves.\n");
      else
        {
	  icanmove= YES;
	  totalmoves= 0;
	  ply(b,MINE,MAX,LARGE,&mvi,&mvj,mptemp,MINE);
	  printf("I take %d%d.  (moves= %d)\n",mvi,mvj,totalmoves);
	  flip(b,mvi,mvj,MINE,b);
	  bookkeep(b);
	}
      endg= !(pcc[0] && pcc[1] && pcc[2]) || !(hecanmove || icanmove);
    }
  printf("Final Position:\n");
  board(b);
  if(pcc[0]>pcc[2])
    printf("You have won %d to %d.\n",pcc[0],pcc[2]);
  if(pcc[0]==pcc[2])
    printf("We have tied %d to %d.\n",pcc[0],pcc[2]);
  if(pcc[0]<pcc[2])
    printf("I have won %d to %d.\n",pcc[2],pcc[0]);
}

/* bookkeep updates board weights, piece counts, ply level */
bookkeep(bp)
int bp[8][8];
{
  int i,j;

  pcc[0]= 0; pcc[1]= 0; pcc[2]= 0;
  for(i=0;i<8;i++)
    for(j=0;j<8;j++)
      pcc[bp[i][j]+1]++;
  if( (((pcc[0]+pcc[2]-4) % STAGE) ==0) && (pcc[0]+pcc[2] !=4))
    for(i=0;i<8;i++)
      for(j=0;j<8;j++)
	weight[i][j]+= dweight[i][j];
  if(pcc[1]<maxply)	/* near end of game, don't search too far */
    mptemp= pcc[1];
}

/* getmove inputs a move for a board bp, returns 10*row+column */
/* 88 move starts automatic play, 99 suggests a move */
getmove(bp,reti,retj)
int bp[8][8];
int *reti,*retj;
{
  int m,i,j;		/* input move, decomposed row and column */
  int ok;		/* ok flag */

  ok= NO;
  while(!ok)
    {
      printf("\nMove > ");
      scanf("%d",&m);
      switch(m)
	{
	  case 88:
	    printf("Automatic play initiated on next move.\n");
	    autoplay= YES;
	    break;
	  case 99:
	    ply(bp,HIS,MAX,LARGE,&i,&j,mptemp,HIS);
	    printf("Try %d%d.\n",i,j);
	    break;
	  default:
	    i= m/10;
	    j= m%10;
	    if (valid(i,j) && bp[i][j]==OPEN && flip(bp,i,j,HIS,bp))
	      ok= YES;
	    else
	      printf("Illegal move");
	}
    }
  *reti=i;
  *retj=j;
}

/* flip checks if move i,j legal for bp, returns YES or NO. */
/* Result of flip returned in bp2 */
flip(bp,i,j,self,bp2)
int bp[8][8];
int i,j,self;
int bp2[8][8];
{
  int ok;		/* okay flag */
  int dc,pc,nix;	/* counters, abort flag */
  int it,jt,pct;	/* temporaries */
  int ix,jx;		/* more temporary counters */
  int enemy;		/* value of enemy */

  enemy= -self;
  ok= NO;
  for(dc=0;dc<8;dc++)	/* for each possible direction */
    {
      pct= 0;
      nix= NO;		/* don't discount this direction yet */
      it=i;jt=j;	/* start searching from move location */
      while(pct++,!nix && valid(it+= di[dc],jt+= dj[dc]))
	{
	  if (bp[it][jt]!=enemy)
	    nix= YES;
	  if ((bp[it][jt]==self)&&(pct>1))
	    {
	      if(!ok)		/* automatically initialize result board */
		{
		  ok= YES;
		  for(ix=0;ix<8;ix++)
		    for(jx=0;jx<8;jx++)
		      bp2[ix][jx]= bp[ix][jx];
		}
	      for(pc=0;pc<pct;pc++)	/* do the flip */
	        bp2[i+pc*di[dc]][j+pc*dj[dc]]= self;
	    }
	}
    }
  return(ok);
}

/* genmove checks if a legal move exists for self from bp */
genmove(bp,self)
int bp[8][8];
int self;
{
  int i,j;		/* counters */
  int tbp[8][8];	/* temporary board */

  for(i=0;i<8;i++)
    for(j=0;j<8;j++)
      if(bp[i][j]==OPEN && flip(bp,i,j,self,tbp))
	return(YES);
  return(NO);
}

/* ply is the workhorse function.  It evaluates all possible moves */
/* on bp for self, given a previous max/min of bound.  It returns */
/* the value of the best move and points to it in pmvi and pmvj. */
/* doer is the one who will execute this move. */
ply(bp,self,maxmin,bound,pmvi,pmvj,depth,doer)
int bp[8][8];
int self,maxmin,bound;
int *pmvi,*pmvj;
int depth;
int doer;
{
  int i,j;		/* counters */
  int nbp[8][8];	/* new position after move */
  int best;		/* extreme min or max found so far */
  int prune;		/* prune flag */
  int valmove;		/* what move under consideration is worth */
  int besti,bestj;	/* best move found here */

  totalmoves++;		/* keep statistics of this */
  if(depth==0)		/* leaf node- return board evaluation */
    return(sbe(bp,doer));
  best= (maxmin==MAX) ? -LARGE : LARGE;		/* best move= nil */
  prune= NO;
  for(i=0;!prune && i<8;i++)
    for(j=0;!prune && j<8;j++)
      {
	if(bp[i][j]==OPEN && flip(bp,i,j,self,nbp))	/* legal */
	  {
	    valmove= ply(nbp,-self,-maxmin,best,pmvi,pmvj,depth-1,doer);
	    if(maxmin==MAX)
	      {
		if(valmove > best)	/* best so far, maximizing */
		  {
		    besti= i;
		    bestj= j;
		    best= valmove;
		    prune= (best>bound) ? YES : NO;	/* alpha/beta */
		  }
	      }
	    else
	      if(valmove < best)	/* best so far, minimizing */
		{
		  besti= i;
		  bestj= j;
		  best= valmove;
		  prune= (best < bound) ? YES : NO;	/* alpha/beta */
		}
	  }
      }
  *pmvi= besti;		/* return best move */
  *pmvj= bestj;
  if(( (best>=0) ? best : -best)==LARGE)	/* none found */
    return(bound);
  else
    return(best);
}

/* valid determines whether i,j is on the board */
valid(i,j)
int i,j;
{
  return((i>=0 && i<=7 && j>=0 && j<=7) ? YES : NO);
}

/* sbe is the static board evaluator, returning bp's value for self */
sbe(bp,self)
int bp[8][8];
int self;
{
  int i,j;		/* counters */
  int goodness;		/* running total */

  goodness= 0;
  for(i=0;i<8;i++)
    for(j=0;j<8;j++)
      goodness+= weight[i][j]*bp[i][j] + 
	         dependx[i][j]*abs(bp[dependi[i][j]][dependj[i][j]]);
  goodness*= self;
  return(goodness);
}

/* board prints the board bp */
board(bp)
int bp[8][8];
{
  int i,j;
  char equiv[3];
  equiv[0]= HISDUDE;
  equiv[1]= NODUDE;
  equiv[2]= MYDUDE;
  printf("\n  ");
  for(j=0;j<8;j++)
    printf(" %d",j);
  printf("\n  ----------------\n");
  for(i=0;i<8;i++)
    {
      printf("%d|",i);
      for(j=0;j<8;j++)
	printf(" %c",equiv[bp[i][j]+1]);
      printf("\n");
    }
}

