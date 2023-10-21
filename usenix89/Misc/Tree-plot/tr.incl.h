#include <stdio.h>
#include <ctype.h>
#include <math.h>
#ifdef APOLLO
#include </sys/ins/base.ins.c>
#include </sys/ins/pad.ins.c>
#include </sys/ins/gpr.ins.c>
#include </sys/ins/kbd.ins.c>
#endif APOLLO

#define MAXNODE 300
#define MAXPORT 600
#define	PI 3.1415926536

#define TOP 1
#define RHS 2
#define BOT 3
#define LHS 4
#define MID 5

#ifdef APOLLO
gpr_$keyset_t buttons = {0,0,0,0};
gpr_$keyset_t btn     = {0,0,0,0};
gpr_$keyset_t keys     = {0,0,0,0};
char keystroke;

gpr_$bitmap_desc_t mem_bm,init_bitmap;
gpr_$event_t event_type;

status_$t st;
#endif APOLLO

char GRAPH_TITLE[128],NODE_FILE[128],PORT_FILE[128];

float mnx,mny,scl;
float xx[1000], yy[1000];
float char_size, char_sz_sml; /* char_size is half character width */

short x[1000],y[1000]; 
short len, font_id, font_id_small;
short delta_x, delta_y; /* distance between nodes */
short char_width;

int node_max, port_max;
int char_height_text,char_width_text;
int MAXX_WINDOW[5], MAXY_WINDOW[5], MINX_WINDOW[5], MINY_WINDOW[5], 
	MIN_Y_SPACE, MIN_X_SPACE,BOXWIDTH,BOXHT,RADIUS,NODE_LABEL;

struct node_data
	{
	int node;
	char name[128];
	int target[5];
	int source[40];
	struct node_data *src[40];
	int cohort[5];
	int lvl;	/* level of node 0=highest at top of graph 
					1=lower 
					... 
					n=lowest at bottom of graph */
	int nth;	/* order 1=first 2=second ... */
	int of;		/* total # on this lvl */
	short x0;	/* x0.y0 - center of node */
	short y0;
	int shape;	/* 0=circle 1=box 2=rect ... */
	short wide;	/* size of shape in x - radius or width */
	short ht;		/* size of shape in y - radius or height */
/*	int inport[20]; */	/* ports into this node */
/*	int outport[20]; */ /* ports leaving this node */
	int pop;        /* parent position */
	int flg; 	/* flag whether to write name or blank it out */
	} nodes[MAXNODE];

struct node_data *nod[MAXNODE],*tmpnod;

struct port_data
	{
	char name[128];
	int from_node;	/* node# of source */
	int to_node;	/* node# of target */
	int nth_from;	/* nodes[nth_from].node = ports[].from_node */
	int nth_to;	/* nodes[nth_to].node = ports[].to_node */
	short x0;	/* x0,y0 - from port location */
	short y0;
	short xf;	/* xf,yf - to port location */
	short yf;
	int side_from;	/* TOP RHS BOT LHS MID */
	int side_to;
	int angle;	/* angle of arrow from side, +angle=cw -angle=ccw */
	} ports[MAXPORT];

