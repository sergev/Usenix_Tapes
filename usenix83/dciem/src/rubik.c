#
/* An attempt at searching the space of the Rubik's cube puzzle from Hungary.
 * Author: Martin Tuori, June 1980.
 * Uses a frame store for displaying the state of the cube --
 * no motion, just diddle the colour map to 'move' faces around.
 * Here is the way I have numbered my cube; yours may be different,
 * so you may have to modify the program or your cube to get things
 * straight.
 *
 * The strategies I have tried so far:
 * 0: balanced scoring for edges (ex 8-20) and corners (ex 9-21-28)
 *	which are in the correct position.
 * 1: a different balanced strategy.
 * 2: adjacent edge-corner pairs score if they are true neighbours,
 *	and score extra if they are on the right face.
 * 3: do all edges first, then do corners.
 * 4: do all corners first.
 * 5: get one corner right, and spread out from there.
                ________________
           BLUE |    |    |    |
                | 1  | 2  | 3  |
                |    |    |    |
                ________________
                |    |    |    |
                | 4  | 5  | 6  |
                |    |    |    |
                ________________
                |    |    |    |
                | 7  | 8  | 9  |
                |    |    |    |
__ORANGE___________________________YELLOW____________RED________
|    |    |    ||    |    |    ||    |    |    ||    |    |    |
| 10 | 11 | 12 || 19 | 20 | 21 || 28 | 29 | 30 || 37 | 38 | 39 |
|    |    |    ||    |    |    ||    |    |    ||    |    |    |
________________________________________________________________
|    |    |    ||    WHITE     ||    |    |    ||    |    |    |
| 13 | 14 | 15 || 22 | 23 | 24 || 31 | 32 | 33 || 40 | 41 | 42 |
|    |    |    ||    |    |    ||    |    |    ||    |    |    |
________________________________________________________________
|    |    |    ||    |    |    ||    |    |    ||    |    |    |
| 16 | 17 | 18 || 25 | 26 | 27 || 34 | 35 | 36 || 43 | 44 | 45 |
|    |    |    ||    |    |    ||    |    |    ||    |    |    |
________________________________________________________________
                |    |    |    |
                | 46 | 47 | 48 |
                |    |    |    |
                ________________
                |    |    |    |
                | 49 | 50 | 51 |
                |    |    |    |
                ________________
                |    |    |    |
                | 52 | 53 | 54 |
          GREEN |    |    |    |
                ________________
 */
/*% dcc -O rubik.c */
/*% cc -O rubik.c */
/* rand() is a better rand number generator than in vanilla UNIX */
/*#define FRAME_BUFFER to enable local display of cube */

#ifdef FRAME_BUFFER
#include "rds.h"
#endif
#include "stdio.h"

#define WHITE 0377
#define YELLOW 0374
#define ORANGE 0354
#define RED 0311
#define BLUE 03
#define GREEN 030

#define MAX_RECURSE 30

int best_score;
int best_cube[55];
char best_sequence[MAX_RECURSE];
int depth;

int corr_score;
int correct[55] = {
	0,
	BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,
	ORANGE,ORANGE,ORANGE,ORANGE,ORANGE,ORANGE,ORANGE,ORANGE,ORANGE,
	WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,
	YELLOW,YELLOW,YELLOW,YELLOW,YELLOW,YELLOW,YELLOW,YELLOW,YELLOW,
	RED,RED,RED,RED,RED,RED,RED,RED,RED,
	GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,GREEN,
};
int redmap[512] = {0};
int greenmap[512] = {0};
int bluemap[512] = {0};
int grey[256] = {0};
int docolours[40] = {0};

int trans[12][55] = {
/* the n'th entry of a transform indicates that the spot which previously
 * held position n on the cube should be moved to new position given.
 */

	0,
	3,6,9,2,5,8,1,4,7,
	37,38,39,13,14,15,16,17,18,
	10,11,12,22,23,24,25,26,27,
	19,20,21,31,32,33,34,35,36,
	28,29,30,40,41,42,43,44,45,
	46,47,48,49,50,51,52,53,54,

	0,
	19,2,3,22,5,6,25,8,9,
	12,15,18,11,14,17,10,13,16,
	46,20,21,49,23,24,52,26,27,
	28,29,30,31,32,33,34,35,36,
	37,38,7,40,41,4,43,44,1,
	45,47,48,42,50,51,39,53,54,

	
	0,
	1,2,3,4,5,6,28,31,34,
	10,11,9,13,14,8,16,17,7,
	21,24,27,20,23,26,19,22,25,
	48,29,30,47,32,33,46,35,36,
	37,38,39,40,41,42,43,44,45,
	12,15,18,49,50,51,52,53,54,

	0,
	1,2,43,4,5,40,7,8,37,
	10,11,12,13,14,15,16,17,18,
	19,20,3,22,23,6,25,26,9,
	30,33,36,29,32,35,28,31,34,
	54,38,39,51,41,42,48,44,45,
	46,47,21,49,50,24,52,53,27,

	0,
	16,13,10,4,5,6,7,8,9,
	52,11,12,53,14,15,54,17,18,
	19,20,21,22,23,24,25,26,27,
	28,29,1,31,32,2,34,35,3,
	39,42,45,38,41,44,37,40,43,
	46,47,48,49,50,51,36,33,30,

	0,
	1,2,3,4,5,6,7,8,9,
	10,11,12,13,14,15,25,26,27,
	19,20,21,22,23,24,34,35,36,
	28,29,30,31,32,33,43,44,45,
	37,38,39,40,41,42,16,17,18,
	48,51,54,47,50,53,46,49,52,

	0,
	7,4,1,8,5,2,9,6,3,
	19,20,21,13,14,15,16,17,18,
	28,29,30,22,23,24,25,26,27,
	37,38,39,31,32,33,34,35,36,
	10,11,12,40,41,42,43,44,45,
	46,47,48,49,50,51,52,53,54,

	0,
	45,2,3,42,5,6,39,8,9,
	16,13,10,17,14,11,18,15,12,
	1,20,21,4,23,24,7,26,27,
	28,29,30,31,32,33,34,35,36,
	37,38,52,40,41,49,43,44,46,
	19,47,48,22,50,51,25,53,54,

	0,
	1,2,3,4,5,6,18,15,12,
	10,11,46,13,14,47,16,17,48,
	25,22,19,26,23,20,27,24,21,
	7,29,30,8,32,33,9,35,36,
	37,38,39,40,41,42,43,44,45,
	34,31,28,49,50,51,52,53,54,

	0,
	1,2,21,4,5,24,7,8,27,
	10,11,12,13,14,15,16,17,18,
	19,20,48,22,23,51,25,26,54,
	34,31,28,35,32,29,36,33,30,
	9,38,39,6,41,42,3,44,45,
	46,47,43,49,50,40,52,53,37,

	0,
	30,33,36,4,5,6,7,8,9,
	3,11,12,2,14,15,1,17,18,
	19,20,21,22,23,24,25,26,27,
	28,29,54,31,32,53,34,35,52,
	43,40,37,44,41,38,45,42,39,
	46,47,48,49,50,51,10,13,16,

	0,
	1,2,3,4,5,6,7,8,9,
	10,11,12,13,14,15,43,44,45,
	19,20,21,22,23,24,16,17,18,
	28,29,30,31,32,33,25,26,27,
	37,38,39,40,41,42,34,35,36,
	52,49,46,53,50,47,54,51,48,

};

main(argc,argv)
	int argc; char **argv;
{
	register int i,j;
	char typedin[80];
	int initial[55];
	char sequence[MAX_RECURSE];

	for(i=0;i<55;i++) initial[i]= correct[i];
	printf("Rubik's cube begins\nCommand:");
	init_display();
#ifdef FRAME_BUFFER
	show(initial);
#endif
	while(gets(typedin) != NULL){
		switch(typedin[0]){
		default:
		case 'h':
			printf("The following commands are valid: htris\n");
			printf("to mean: help,transform,run,input,show\n");
			break;
		case 't':
			i= atoi(&typedin[1]);
			if(i<1 || i>12){
				printf("transforms 1-12 only\n");
				break;
			}
			transform(initial,i);
			break;
		case 'r':
			depth= atoi(&typedin[1]);
			if(depth>MAX_RECURSE || depth<0){
				printf("recursion from 0 to %d only\n",MAX_RECURSE);
				break;
			}
			for(i=0;i<55;i++) best_cube[i]= initial[i];
			for(i=0;i<MAX_RECURSE;i++)
				sequence[i]= best_sequence[i]= '\0';
			best_score= score(best_cube);
			corr_score= score(correct);
			search(initial,depth,sequence);
			for(i=0;i<55;i++) initial[i]= best_cube[i];
			printf("best score= %d/%d\n",best_score,corr_score);
			printf("sequence of moves: ");
			for(i=0;i<depth;i++)
				printf("%d  ",best_sequence[i]);
			printf("\n");
			break;
		case 'i':
			input(initial);
			show(initial);
			break;
		case 's':
			show(initial);
			break;
		}
	}
}

init_display(){
	register int i;

#ifdef FRAME_BUFFER
	/* call a subroutine to artificially grow the stack,
	 * before opening the display, which will lock
	 * us in core.
	 */
	growstack();
	init_rds();

	for(i=0;i<256;i++) grey[i]= i*4;
	slowsub(docolours);
	calib(AREDCALIB,grey);
	calib(AGRNCALIB,grey);
	calib(ABLUCALIB,grey);
	sloweof();
	execdf(docolours);

	slowsub(docolours);
	mult(AMODE,BYPASS);
	coltab(AREDCOL,redmap);
	coltab(AGRNCOL,greenmap);
	coltab(ABLUCOL,bluemap);
	mult(AMODE,COLTAB);
	sloweof();
#endif
}

int cube_to_print[] = {
0,
4,5,6,10,11,12,16,17,18,  19,20,21,31,32,33,43,44,45,
22,23,24,34,35,36,46,47,48,   25,26,27,37,38,39,49,50,51,
28,29,30,40,41,42,52,53,54,   58,59,60,64,65,66,70,71,72,
};

show(cube)
	int cube[];
{
#ifdef FRAME_BUFFER
	register int i;
	register int face;

	for(i=1;i<=54;i++){
		face= cube[i];
		redmap[i]= redmap[256+i]= face&0340;
		greenmap[i]= greenmap[256+i]= (face&034)<<3;
		bluemap[i]= bluemap[256+i]= (face&03)<<6;
	}
	execdf(docolours);
#endif
#ifndef FRAME_BUFFER
	char printb[73];
	register int i;

	for(i=0;i<73;i++) printb[i]= ' ';
	for(i=1;i<55;i++){
		switch(cube[i]){
		case WHITE: printb[cube_to_print[i]]= 'W'; break;
		case YELLOW: printb[cube_to_print[i]]= 'Y'; break;
		case ORANGE: printb[cube_to_print[i]]= 'O'; break;
		case RED: printb[cube_to_print[i]]= 'R'; break;
		case BLUE: printb[cube_to_print[i]]= 'B'; break;
		case GREEN: printb[cube_to_print[i]]= 'G'; break;
		}
	}

	printf("\n");
	for(i=1;i<73;i++){
		printf("%c ",printb[i]);
		if(i==6 || i==12 || i==18 || i==30 || i==42 || i==54
			|| i==60 || i==66 || i==72)
			printf("\n");
	}
#endif
}

transform(cube,tnum)
	int cube[];
	register int tnum;
{
	register int i;
	int temp[55];

	tnum--; /* correct for 0 indexing in trans[][] */
	for(i=0;i<55;i++)
		temp[trans[tnum][i]]= cube[i];
	for(i=0;i<55;i++)
		cube[i]= temp[i];
}


search(cube,level,sequence)
	int cube[];
	int level;
	char sequence[];
{
	register int i,j;
	int temp[55];
	int temp_score;

	if(level <= 0 || best_score == corr_score) return;
	for(j=1;j<=12;j++){
		sequence[depth-level]= j;
		/* now perform pruning, by comparing the would be current
		 * sequence with a list of identity (null) transformation
		 * sequences.
		 */
		if(prune(sequence)) continue;
		for(i=0;i<55;i++) temp[i]= cube[i];
		transform(temp,j);
		temp_score= score(temp);
		if(temp_score > best_score ||
			(temp_score == best_score && (rand()&077777)<(32767/12)) ){
			best_score= temp_score;
			for(i=0;i<55;i++) best_cube[i]= temp[i];
			for(i=0;i<MAX_RECURSE;i++)
				best_sequence[i]= sequence[i];
#ifdef FRAME_BUFFER
			show(best_cube);
#endif
		}
		search(temp,level-1,sequence);
	}
	sequence[depth-level]= '\0';
}

score(cube)
	int cube[];
{
	return(p_score(cube));
}

struct scores0{
	int check;
	int ccolour;
	int dependant;
	int dcolour;
	int points0;
}scores0[] = {
/* when this was the 0th strategy, I had corners worth 5 and edges worth 2 points */
/* when it was strat. 1, corners 2, edges 4 */
/* whne it was strat. 3, corners 1, edges 10 */
/* strat. 4, corners 20, edges 1 */
/* currently strat 5 */
1,BLUE,10,ORANGE,400,
3,BLUE,30,YELLOW,100,
7,BLUE,12,ORANGE,1600,
9,BLUE,28,YELLOW,400,
46,GREEN,18,ORANGE,400,
48,GREEN,34,YELLOW,100,
52,GREEN,16,ORANGE,100,
54,GREEN,36,YELLOW,25,

2,BLUE,38,RED,200,
13,ORANGE,42,RED,200,
53,GREEN,44,RED,50,
33,YELLOW,40,RED,50,
4,BLUE,11,ORANGE,800,
6,BLUE,29,YELLOW,200,
49,GREEN,17,ORANGE,200,
51,GREEN,35,YELLOW,50,
20,WHITE,8,BLUE,800,
22,WHITE,15,ORANGE,800,
24,WHITE,31,YELLOW,200,
26,WHITE,47,GREEN,200,

0,0,0,0,0,
};

score0(cube)
	int cube[];
{
	register struct scores0 *sp;
	register int tally;

	tally=0;
	sp= scores0;
	while(sp->check){
		if(cube[sp->check]==sp->ccolour &&
			cube[sp->dependant]==sp->dcolour) tally+= sp->points0;
		sp++;
	}
	return(tally);
}

growstack(){
	int biggie[20*512];

	biggie[20*512 -1]= 0;
}
struct scores1{
	int pair1;
	int check1;
	int pair2;
	int check2;
	int points1;
}scores1[] = {
1,2,38,39,5,
2,3,37,38,5,
1,4,10,11,5,
3,6,29,30,5,
4,7,11,12,5,
6,9,28,29,5,
7,8,19,20,5,
8,9,20,21,5,

10,13,39,42,5,
13,16,42,45,5,
12,15,19,22,5,
15,18,22,25,5,
21,24,28,31,5,
24,27,31,34,5,
30,33,37,40,5,
33,36,40,43,5,

46,47,25,26,5,
47,48,26,27,5,
46,49,17,18,5,
48,51,34,35,5,
49,52,16,17,5,
51,54,35,36,5,
52,53,44,45,5,
53,54,43,44,5,

0,0,0,0,0,
};

struct aux1{
	int corner1;
	int centre1;
	int aux1_pts;
} aux1[] = {
1,5,1,		3,5,1,		7,5,1,		9,5,1,
10,14,1,	12,14,1,	16,14,1,	18,14,1,
19,23,1,	21,23,1,	25,23,1,	27,23,1,
28,32,1,	30,32,1,	34,32,1,	36,32,1,
37,41,1,	39,41,1,	43,41,1,	45,41,1,
46,50,1,	48,50,1,	52,50,1,	54,50,1,

0,0,0,
};

score1(cube)
	int cube[];
{
	register struct scores1 *sp;
	register int tally;
	register struct aux1 *sp1;

	tally=0;
	sp= scores1;
	while(sp->pair1){
		if(cube[sp->pair1] == cube[sp->check1] &&
			cube[sp->pair2] == cube[sp->check2])
			tally += sp->points1;
		sp++;
	}
	sp1= aux1;
	while(sp1->corner1){
		if(cube[sp1->corner1] == cube[sp1->centre1])
			tally += sp1->aux1_pts;
		sp1++;
	}
	return(tally);
}

input(cube)
	int cube[];
{
	register int i,j;

	/* read in characters indicating the sequence of colours */
	for(i=1;i<55;){
		switch(getchar()){
		default:
			printf("bad char in input\n");
			continue;
		case '\n':
		case '\t':
		case ' ':
			continue;
		case 'b':
			cube[i]= BLUE;
			break;
		case 'o':
			cube[i]= ORANGE;
			break;
		case 'w':
			cube[i]= WHITE;
			break;
		case 'y':
			cube[i]= YELLOW;
			break;
		case 'r':
			cube[i]= RED;
			break;
		case 'g':
			cube[i]= GREEN;
			break;
		}
		i++;
	}
	getchar(); /* clear the trailing newline character */

	/* Perform a consistency check on the centres. */
	if(cube[5]!=BLUE || cube[14]!=ORANGE ||
	   cube[23]!=WHITE || cube[32]!=YELLOW ||
	   cube[41]!=RED || cube[50]!=GREEN)
		printf("The centres aren't right\n");
}

/* the following transformation sequences are redundant,
 * and their subtrees can be pruned.
 */
char prunetab[][5] = {
1,7,0,0,0,
2,8,0,0,0,
3,9,0,0,0,
4,2,0,0,0,
4,10,0,0,0,
5,3,0,0,0,
5,11,0,0,0,
6,1,0,0,0,
6,12,0,0,0,
7,1,0,0,0,
7,7,0,0,0,
8,2,0,0,0,
8,8,0,0,0,
9,3,0,0,0,
9,9,0,0,0,
10,2,0,0,0,
10,4,0,0,0,
10,8,0,0,0,
10,10,0,0,0,
11,3,0,0,0,
11,5,0,0,0,
11,9,0,0,0,
11,11,0,0,0,
12,1,0,0,0,
12,6,0,0,0,
12,7,0,0,0,
12,12,0,0,0,

1,1,1,0,0,
2,2,2,0,0,
3,3,3,0,0,
4,4,4,0,0,
5,5,5,0,0,
6,6,6,0,0,

1,6,7,0,0,
2,4,8,0,0,
3,5,9,0,0,

0,0,0,0,0,
};

prune(sequence)
	char sequence[];
{
	register int i;
	register int l_seq, l_prune;
	int j;

	l_seq= strlen(sequence);
	for(i=0;prunetab[i][0] != 0;i++){
		l_prune= strlen(prunetab[i]);
		if(l_seq < l_prune) continue;
		if(strcmp(prunetab[i], &sequence[l_seq-l_prune])==0){
/*
printf("pruned: ");
for(j=0;j<depth;j++) printf("%d, ",sequence[j]);
printf("\n");
*/
			return(1);
		}
	}
	return(0);	/* no prune found */
}

/* p_score is a strategy based on the most common approach to solving
the cube, that of starting with one correct face and then proceeding
to the middle band, and finally the opposite face.
 */
struct p_table {
	int p_face1;
	int p_col1;
	int p_face2;
	int p_col2;
	int p_score;
} p_table[] =
{
	19,WHITE, 12,ORANGE, 1000,
	21,WHITE, 28,YELLOW, 1000,
	25,WHITE, 18,ORANGE, 1000,
	27,WHITE, 34,YELLOW, 1000,
	20,WHITE, 8,BLUE, 200,
	22,WHITE, 15,ORANGE, 200,
	26,WHITE, 47,GREEN, 200,
	24,WHITE, 31,YELLOW, 200,
	11,ORANGE, 4,BLUE, 40,
	6,BLUE, 29,YELLOW, 40,
	35,YELLOW, 51,GREEN, 40,
	49,GREEN, 17,ORANGE, 40,
	10,ORANGE, 1,BLUE, 8,
	3,BLUE, 30,YELLOW, 8,
	36,YELLOW, 54,GREEN, 8,
	52,GREEN, 16,ORANGE, 8,
	13,ORANGE, 42,RED, 1,
	2,BLUE, 38,RED, 1,
	33,YELLOW, 40,RED, 1,
	53,GREEN, 44,RED, 1,
	0,0, 0,0, 0,
};

p_score(cube)
	int cube[];
{
	register struct p_table *sp;
	register int tally;

	tally= 0;
	sp= p_table;
	while(sp->p_face1){
		if(cube[sp->p_face1]==sp->p_col1 &&
			cube[sp->p_face2]==sp->p_col2) tally += sp->p_score;
		sp++;
	}
	return(tally);
}

/* rand -- pseudo-rand number generator (cf. Knuth v.2, p464) */
int randvec[] = {
	15637,264,32748,17732,1234,23142,25576,32133,11934,29378,15755,
	22053,1023,31247,13903,16542,14,1264,31444,21099,19465,29888,
	18764,22244,8753,7654,2,30054,12567,22443,30002,14322,16667,989,
	17875,22234,12121,5152,30079,31705,5523,21345,1145,9987,17223,
	11034,5678,2219,17430,0,2705,26244,11214,31345,4
};
int *ptr24 = &randvec[31];
int *ptr55 = randvec;
rand(){
	register newrand;
	newrand = *ptr24+*ptr55;
	*ptr55=newrand;
	ptr55++;
	if(ptr55 == &randvec[55])
		ptr55=randvec;
	ptr24++;
	if(ptr24 == &randvec[55])
		ptr24=randvec;
	return(newrand&077777);
}
srand(seed)
	int seed;
{
	register int i,k;
	int temp;

	seed &= 077777;
	for(i=0;i<55;i++)
		randvec[i]= randvec[i] ^ seed;
	k= (seed%17)+(seed%19)+(seed%21);
	for(i=0;i<55;i++){
		temp= randvec[(i+k)%55];
		randvec[(i+k)%55]= randvec[i];
		randvec[i]= temp;
	}
}
