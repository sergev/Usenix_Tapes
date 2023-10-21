#include "tr.incl.h"
/*
	tidy tree drawing routines expanded from :
Tidy Drawing of Trees by C. Wetherell & A. Shannon IEEE Vol. SE-5 #5
    Sept 79 pp. 514-520
Tidier Drawings of Trees by E. Reingold & J. Tilford IEEE Vol. SE-7 #2
    Mar '81 pp. 223-228

	program written by April Gillam (gillam@aerospace.arpa)
			   Aerospace Corp.
			   2350 El Segundo Blvd. M1-102
			   El Segundo, Ca. 90245
*/
int tidy_tree(),text(),check(),setbutton(),clearbutton(),closest(),edge(),
	circle(),set_node_locs(),prnt_in_box(),
	where_port(),set_port(),rectangle(),arrow2(),init();
int askacg,lvlstart[20];

main(argc,argv)
char *argv[];
int argc;
{
int loop, j;
short position[2];
#ifdef APOLLO
gpr_$window_t src_wind;
gpr_$position_t dest_orig,cursor_pos;
#endif APOLLO

	read_limits();

	init();		 /* initialize graphics */

/*
		askacg == 1 ask user for acgnode.data & acgport.data file names
			< 0 use small boxes & don't print words in box
*/
	if(argc > 1) askacg = atoi(argv[1]);
	else askacg = 0;
	if(askacg < 0){
		BOXWIDTH = 20; BOXHT = 15; RADIUS = 15; MIN_X_SPACE = 10;
		MIN_Y_SPACE = 10;
		printf("enter BOXWIDTH BOXHT: ");
		scanf("%d %d",&BOXWIDTH,&BOXHT);
		}

	node_max = read_node_data(askacg);

	len = node_max;

	tidy_tree(node_max);

	port_max = read_port_data(askacg);

#ifdef APOLLO
	gpr_$set_bitmap(mem_bm,st); check(st);

	set_node_locs();

	src_wind.x_coord = (short)0;
	src_wind.y_coord = (short)0;
	src_wind.x_size = MAXX_WINDOW[0] - MINX_WINDOW[0];
	src_wind.y_size = MAXY_WINDOW[0] - MINY_WINDOW[0];

	dest_orig.x_coord = (short) 0;
	dest_orig.y_coord = (short) 0;

	gpr_$set_bitmap(init_bitmap,st); check(st);
gpr_$acquire_display(st); check(st);
	gpr_$pixel_blt(mem_bm,src_wind,dest_orig,st); check(st); 
	gpr_$set_cursor_active(true,st); check(st);
	for(loop=1;loop;) {
		for(loop=1;loop==1;){
			gpr_$event_wait(event_type,keystroke,position,st);check(st);
			switch(event_type){
			    case  gpr_$locator:  gpr_$set_cursor_position(position,st);break;
			    case  gpr_$keystroke: 
/*			    case  gpr_$buttons: */
			    switch(255&keystroke){
/*			      case 'c': closest(position);loop=2;break;
			      case 'b': edge(position);   loop=2;break;
*/
		case 'q':	loop=0;break;
              case KBD_$L4: 
			src_wind.x_coord = (short) 0;
			cursor_pos.x_coord = 0;
			cursor_pos.y_coord = position[1];
			gpr_$set_cursor_position(cursor_pos,st);check(st);
			break;
              case KBD_$L6: 
			src_wind.x_coord = MAXX_WINDOW[1] - src_wind.x_size;
			cursor_pos.x_coord = src_wind.x_coord;
			cursor_pos.y_coord = position[1];
			gpr_$set_cursor_position(cursor_pos,st);check(st);
			break;
		case KBD_$L7: 
			src_wind.x_coord = src_wind.x_coord - 100;
			if(src_wind.x_coord < 0)
				src_wind.x_coord = 0;
			break;
              case KBD_$L8S:
			src_wind.y_coord = src_wind.y_coord - 10;
			if(src_wind.y_coord < 0)
				src_wind.y_coord = 0;
			break;
              case KBD_$L9:
			src_wind.x_coord = src_wind.x_coord + 100;
			if(src_wind.x_coord > (MAXX_WINDOW[1] - src_wind.x_size))
				src_wind.x_coord = MAXX_WINDOW[1] - src_wind.x_size;
			break;
              case KBD_$LAS:
			src_wind.x_coord = src_wind.x_coord - 10;
			if(src_wind.x_coord < 0)
				src_wind.x_coord = 0;
			break;
              case KBD_$LCS:
			src_wind.x_coord = src_wind.x_coord + 10;
			if(src_wind.x_coord > (MAXX_WINDOW[1] - src_wind.x_size))
				src_wind.x_coord = MAXX_WINDOW[1] - src_wind.x_size;
			break;
              case KBD_$LD:
			src_wind.y_coord = src_wind.y_coord - 100;
			if(src_wind.y_coord < 0)
				src_wind.y_coord = 0;
			break;
              case KBD_$LES:
			src_wind.y_coord = src_wind.y_coord + 10;
			if(src_wind.y_coord > MAXY_WINDOW[1] - src_wind.y_size)
				src_wind.y_coord = MAXY_WINDOW[1] - src_wind.y_size;
			break;
              case KBD_$LF:
			src_wind.y_coord = src_wind.y_coord + 100;
			if(src_wind.y_coord > MAXY_WINDOW[1] - src_wind.y_size)
				src_wind.y_coord = MAXY_WINDOW[1] - src_wind.y_size;
			break;
		default:
			break;
			      } /* switch keystroke */
		gpr_$pixel_blt(mem_bm,src_wind,dest_orig,st); check(st); 
			break;
			    } /* switch event_type */
			} /* loop */
		} /* loop */

gpr_$release_display(st);check(st);
	gpr_$terminate(true,st);check(st);
#endif APOLLO
}
/* ************************************************************ */
/*	nmax = # of nodes
*/
tidy_tree(nmax)
int nmax;
{
int j,k,m,nlvl,lvl,oldlvl,maxlvl,xdelta,ydelta,pop,jth,j0,jf;
int n_per_lvl[20],next_pos,xmin,xmax,xchild,xparent,mostx,cnt,l,jj;

xdelta=120; ydelta = 100;
for(j=0;j<20;j++)
	n_per_lvl[j]=0;
maxlvl = 0;
for(j=0;j<nmax;j++){
	nod[j] = &nodes[j];
	if(nodes[j].lvl > maxlvl) maxlvl = nodes[j].lvl;
	}
/*
	set lvls
*/
/* find top assertion assuming there is only one at the top */
for(j=0;j<nmax;j++){
	if(nodes[j].lvl > 1) nodes[j].lvl = 0;
	nodes[j].pop = 0;
	}
for(lvl = 1; lvl <= maxlvl; lvl++) {
	for(j = 0; j<nmax; j++){
		if(nodes[j].lvl != lvl) continue;
		k = 0;
		while(nodes[j].source[k] != 0){
			for(m=0; m<nmax; m++)
				if(nodes[m].node == nodes[j].source[k]) {
					nodes[m].lvl = lvl + 1;
					break;
					}
			k++;
			}
		}
	}
/* 
	sort by lvl 
*/
for(k=0;k<nmax-1;k++){
for(j=k;j<nmax-1;j++){
	if(nod[j]->lvl > nod[j+1]->lvl){
		tmpnod = nod[j];
		nod[j] = nod[j+1];
		nod[j+1] = tmpnod;
		}
        }
	}
/*
	find where each lvl begins
*/
for(j=1; j<=maxlvl; j++) lvlstart[j] = nmax;
lvl = 1; 
for(j=0; j<nmax; j++){
	lvl = nod[j]->lvl;
	if(j < lvlstart[lvl]) lvlstart[lvl] = j;
	}
while(lvlstart[maxlvl] == nmax) maxlvl--;
/*
	set src ptr to nod source
*/
for(j=nmax-1;j>=0;j--){
	m = 0;
        if(nod[j]->source[0] == 0) nod[j]->src[0] = NULL;
	else while(nod[j]->source[m] != 0){
		for(k=j+1; k<nmax;k++)
			if(nod[k]->node == nod[j]->source[m]){
          			nod[j]->src[m] = nod[k];
				break;
          			}
		m++;
		}
	}
for(j=0;j<nmax;j++){
	lvl = nodes[j].lvl;
	++n_per_lvl[lvl];
	}
/*
	make sure children of same parents are next to each other
	pop only keeps track of one of parents, what if multiple parents???
*/
for(lvl = 1; lvl < maxlvl; lvl++) {
	pop = 0;
	printf("lvl=%d: ",lvl);
	j0 = lvlstart[lvl]; jf = lvlstart[lvl+1];
	for(j=j0; j<jf; j++){
		if(nod[j]->source[0] == 0) continue;
		pop++;
		m = 0;
		while(nod[j]->source[m] != 0){
			(nod[j]->src[m])->pop = pop;
			printf("src[%d]pop(%d) ",nod[j]->source[m],pop);
			m++;
      			}
		}
printf("\n");
/* for each lvl exchange children positions so siblings are together */
	j0 = lvlstart[lvl+1]; jf = lvlstart[lvl+2];
	for(jj=j0; jj<jf-1; jj++){
		if(nod[jj]->pop == 0) continue;
		k = jj;
		while(k+1 < nmax && nod[++k]->lvl == lvl+1){
			if(nod[jj]->pop <= nod[k]->pop) continue;
			tmpnod = nod[jj];
			nod[jj] = nod[k];
			nod[k] = tmpnod;
			}
		}
printf("\t\t");
	for(jj = j0; jj < jf; jj++)
		printf("%d(%d) ",nod[jj]->node,nod[jj]->pop);
	printf("\n");
	}
/* x spacing - initial values */
for(lvl = 1; lvl <= maxlvl; lvl++) {
	printf("lvl %d) ",lvl);
	next_pos = xdelta;
	j0 = lvlstart[lvl]; jf = lvlstart[lvl+1];
	for(j = j0; j<jf; j++){
		nod[j]->x0 = next_pos;
		next_pos = next_pos + xdelta;		
printf("%d<%d> ",nod[j]->node,nod[j]->x0);
		}
printf("\n");
	}
/*
	assign ycoord
*/
	ydelta = (MAXY_WINDOW[0] - MINY_WINDOW[0]) / maxlvl;
	if(ydelta < 3.5*BOXHT)
		ydelta = 3.5*BOXHT;
	if(ydelta > (MAXY_WINDOW[1] - MINY_WINDOW[1]) / maxlvl)
		ydelta = (MAXY_WINDOW[1] - MINY_WINDOW[1]) / maxlvl; 
for(j=0;j<nmax;j++){
	lvl = nodes[j].lvl;
	nodes[j].y0 = lvl * ydelta;
	}
/*
	 algorithm to assign xcoord
*/
		x_space(maxlvl,nmax,xdelta);

printf("node    x0    y0  lvl\n");
for(j=0;j<nmax;j++){
	printf("%d  %5d  %5d  %3d\n",nod[j]->node,nod[j]->x0,nod[j]->y0,nod[j]->lvl);
	}

}
/* ************************************************************ */
x_space(maxlvl,nmax,xdelta)
int maxlvl,nmax,xdelta;
{
int m,oldlvl,next_pos,jj,j,k,imin,xmin,xmax,lvl,new,xshift,j0,jf;
/*
	 algorithm to assign xcoord
*/
next_pos=xdelta;
m=0; oldlvl=maxlvl; 
for(j=nmax-1;j>=0;j--){
	lvl = nod[j]->lvl;	
	if(lvl == oldlvl) m++;
	else{
/*	make sure nodes are not on top of one another */
		next_pos=xdelta;
		j0 = lvlstart[oldlvl]; jf = lvlstart[oldlvl+1];
		for(jj = jf-1; jj>=j0; jj--){
			for(k = jf-1; k >=j0; k--){
				if(k == jj) continue;
				if(abs(nod[jj]->x0 - nod[k]->x0) < xdelta)
					nod[k]->x0 = nod[jj]->x0 + xdelta;
				}
			}
		m=1;
		oldlvl = lvl;
		} /* else */
/* leaf */
	if(nod[j]->source[0] == 0){
		nod[j]->x0 = next_pos;
		same_spot(xdelta,nmax,lvl,nod[j]->x0,nod[j]->node);
		next_pos = next_pos+xdelta;
		}
/* children */
	else{
		k = 0; xmax=0; xmin=4096;
		while(nod[j]->source[k] != 0){
			if(xmin > (nod[j]->src[k])->x0)
				xmin = (nod[j]->src[k])->x0;
			if(xmax < (nod[j]->src[k])->x0)
				xmax = (nod[j]->src[k])->x0;
			k++;
			}
		new = (xmax+xmin)/2;
		if(new < next_pos) new = next_pos;
		nod[j]->x0 = new;
		same_spot(xdelta,nmax,lvl,nod[j]->x0,nod[j]->node);
		next_pos = new + xdelta;
		} /* else */
	} /* for j=nmax-1 */
printf("Phase I\n");
for(lvl = 1; lvl <= maxlvl; lvl++) {
	printf("lvl %d) ",lvl);
	j0 = lvlstart[lvl]; jf = lvlstart[lvl+1];
	for(j = j0; j<jf; j++)
		printf("%d<%d> ",nod[j]->node,nod[j]->x0);
	printf("\n");
	}
/* put children beneath parents:
	one parent
		leaf:	directly beneath
		bros:	equidistant
	multi parents
		leaf:	equidistant
		bros:	???
*/
/* lvl 1 only has 1 node, so don't need to check if one
	node obscures another
*/
for(j=1;j<nmax;j++)
	same_spot(xdelta,nmax,nod[j]->lvl,nod[j]->x0,nod[j]->node);

for(lvl=1; lvl<maxlvl; lvl++){
	j0 = lvlstart[lvl]; jf = lvlstart[lvl+1];
	for(j = jf-1; j>=j0; j--){
		m = 0; k = 0;
		while(nod[j]->source[k++] != 0) m++;
/* one parent leaf */
		if(m == 1){
			xshift = nod[j]->x0 - nod[j]->src[0]->x0;
			if(xshift > xdelta/2) {
				printf("parent %d at %d xshift=%d xdelta=%d\n",nod[j]->node,nod[j]->x0,xshift,xdelta);
				printf("\tchild=%d at %d\n",nod[j]->source[0],nod[j]->src[0]->x0);
				shift_rt(nod[j]->src[0]->lvl,nod[j]->source[0],xshift,nmax,xdelta);
				}
	        } /* m = 1 */
/* one parent >1 child */
	else if(m > 1){
		k = 0; xmax=0; xmin=4096;
		while(nod[j]->source[k] != 0){
			if(xmin > (nod[j]->src[k])->x0) {
				xmin = (nod[j]->src[k])->x0; imin = k; }
			if(xmax < (nod[j]->src[k])->x0)
				xmax = (nod[j]->src[k])->x0;
			k++;
			}
		if((nod[j]->x0 - xmin) - (xmax - nod[j]->x0) <= xdelta) continue;
		xshift = nod[j]->x0 - (xmax+xmin)/2;
		if(xshift > xdelta/2) {
			printf("parent %d at %d xshift=%d xdelta=%d\n",nod[j]->node,nod[j]->x0,xshift,xdelta);
			for(jj=0;jj<m;jj++)
			printf("\tchildren=%d at %d\n",nod[j]->source[jj],nod[j]->src[jj]->x0);
			shift_rt(nod[j]->src[0]->lvl,nod[j]->source[imin],xshift,nmax,xdelta);
			}
		} /* else m>1 */
		} /* for j=jf-1 ... */

	} /* lvl */
}
/* ************************************************************ */
/* shift lvl nodes, starting with nbro, to the right by xshift
*/
shift_rt(lvl,nbro,xshift,nmax,xdelta)
int lvl,nbro,xshift,nmax,xdelta;
{
int i,flag,old_x,j0,jf,xdif;
	flag = 0;
	printf("lvl=%d\n",lvl);
	j0 = lvlstart[lvl]; jf = lvlstart[lvl+1];
	for(i=j0; i<jf; i++) 
		if(nod[i]->node == nbro) old_x = nod[i]->x0;

	for(i=j0; i<jf; i++){
		printf("\t(%d)x=%d->",nod[i]->node,nod[i]->x0);
		if(nod[i]->x0 >= old_x) nod[i]->x0 += xshift;
		printf("%d\n",nod[i]->x0);
		}
	old_x += xshift;
	for(i=j0; i<jf; i++){
		if(nod[i]->node == nbro) continue;
		if(abs(xdif = nod[i]->x0 - old_x) < xdelta) {
			printf("\t**** %d is at %d oops! (%d at %d)\n",nod[i]->node,nod[i]->x0,nbro,old_x);
			
			}
		}
}
/* ************************************************************ */
/*	
		check that node=nmbr at xspot is the only one there
*/
same_spot(xdelta,nmax,lvl,xspot,nmbr)
int xdelta,nmax,lvl,xspot,nmbr;
{
int jj,k,xshift,j0,jf,j;
	j0 = lvlstart[lvl]; jf = lvlstart[lvl+1];
	for(j=j0; j<jf; j++){
		if(nod[j]->node == nmbr) continue;
		if(abs(nod[j]->x0 - xspot) >= xdelta) continue;

		xshift = nod[j]->x0 - xspot;

		if(xshift < xdelta && xshift > 0)
			shift_rt(lvl,nod[j]->node,xdelta-xshift,nmax,xdelta);

		else if(abs((double)xshift) < xdelta && xshift < 0)
			shift_rt(lvl,nod[k]->node,xdelta+xshift,nmax);
		}
}
/* 
************************* node locations ********************************
*/
set_node_locs()
{
int nlvl, i, j, k, n_per_lvl[20], label_node;
char str[128];
short del_x,del_y,xcenter, ycenter;
short xpt, ypt,xxx[5],yyy[5],xmid,ymid;
short box_w,box_h,box_w_fr,box_w_to;
	/* read from acglimits.data file */
short MIN_BOX_W,MIN_BOX_H,MAX_BOX_W,MAX_BOX_H; 

FILE *datafile, *fopen();
int nn,nx,ny;
	MIN_BOX_W = 20;
	MIN_BOX_H = 20;
	MAX_BOX_W = 120;
	MAX_BOX_H = 120;

	for(j = 0; j < 20; j++) n_per_lvl[j] = 0;
	nlvl = 0;
	for(i = 0; i < node_max; i++){
		j = nodes[i].lvl;
		if(j > nlvl) nlvl = j;
		n_per_lvl[j]++;
		}
	nlvl++;
	label_node = NODE_LABEL;
/* y spacing */
	del_y = (MAXY_WINDOW[0] - MINY_WINDOW[0]) / nlvl;
/* assumes have a box & not a circle -- fix so handles circles too */
	if( del_y < 2*BOXHT) 
		del_y = (MAXY_WINDOW[1] - MINY_WINDOW[1]) / nlvl;
/* title for graph */
/* fix so center of memory_bm ??? */
/*	xpt = (MAXX_WINDOW[0] - MINX_WINDOW[0]) / 2 - char_size*strlen(GRAPH_TITLE); */
/* assuming only one node at the top */
	xpt = nodes[0].x0 - char_size*strlen(GRAPH_TITLE);
	ypt = del_y/3;
	text(xpt,ypt,GRAPH_TITLE);
/* x spacing */
	for(i = 1; i < nlvl; i++){	 /* # of levels */
		for(k = 0; k < node_max; k++){ /* actual node data */
			if(nodes[k].lvl == i) {/* weed out those not on this lvl */

		del_x = (MAXX_WINDOW[0] - MINX_WINDOW[0]) / (n_per_lvl[i] + 1); 
/* assumes have a box & not a circle -- fix so handles circles too */
		if( del_x < 2*BOXWIDTH)
			del_x = (MAXX_WINDOW[1] - MINX_WINDOW[1]) / (n_per_lvl[i] + 1); 
/*		if(RADIUS >= del_x && BOXWIDTH >= del_x){ */
		if(nodes[k].shape == 0) { box_h = 2*RADIUS;
                        box_w = 2*RADIUS; }
		else { box_h = 2*BOXHT;
                        box_w = 2*BOXWIDTH; }
		if(box_h > MAX_BOX_H) box_h = MAX_BOX_H;
		if(box_h < MIN_BOX_H){
			printf("Box height( = %ld) is too small\n\tEither increase window size in acglimits.data\n\tor decrease the # of node levels!\n",(long) box_h);
			exit(0);
			}
		if(box_w > MAX_BOX_W) box_w = MAX_BOX_W;
		if(box_w < MIN_BOX_W){
			printf("Box width( = %ld) is too small\n\tEither increase window size in acgdefs\n\tor decrease the # of node levels!\n",(long) box_w);
			exit(0);
			}
/*		} */ /* comparison to del_x */
/*		else {
			box_w = del_x;
			box_h = 0.5*del_y;
			}
*/
				nodes[k].wide = box_w;
				nodes[k].ht = box_h;
/* from tidy_tree */
				xcenter = nodes[k].x0;
				ycenter = nodes[k].y0;
/* offset should be set here
	xcenter += xoffset;
*/

/* shape 0 = circle 1 = box */
			if(nodes[k].shape == 0) circle(xcenter,ycenter,RADIUS);
			else rectangle(xcenter,ycenter,box_w,box_h);

/* print node# */
#ifdef APOLLO
	gpr_$set_text_font(font_id,st);check(st);
#endif APOLLO
		sprintf(str,"%d",nodes[k].node);
		xpt = xcenter - char_size*strlen(str);
		ypt = ycenter - nodes[k].ht/2 + 2*char_size;
		text(xpt, ypt,str);

#ifdef APOLLO
      gpr_$set_text_font(font_id_small,st);check(st);
#endif APOLLO
/* print name */
        ypt = ycenter - nodes[k].ht/2 + 4*char_size;
	if(label_node == 1 && askacg >= 0)
		prnt_in_box(k,xcenter,ypt,del_x);

        	} /* nodes.lvl ok */
	} /* for k < node_max */   
	} /* for nlvl */
/* arrows */
	where_port(port_max);
	for(i = 0; i < port_max; i++){
		box_w_fr = nodes[ports[i].nth_from].wide;
		box_w_to = nodes[ports[i].nth_to].wide;
                if(ports[i].side_from == BOT && ports[i].side_to == BOT) {
	                xmid = ports[i].x0 + (ports[i].xf - ports[i].x0)/2;
        	        ymid = ports[i].y0 + (del_y - box_h)/4;
#ifdef APOLLO
		      gpr_$set_text_font(font_id,st);check(st);
			arrow(ports[i].x0,ports[i].y0,ports[i].xf,ports[i].yf,xmid,ymid,ports[i].name);
		      gpr_$set_text_font(font_id_small,st);check(st);
#endif APOLLO
			} 
		else {
#ifdef APOLLO
		      gpr_$set_text_font(font_id,st);check(st);
		      arrow(ports[i].x0,ports[i].y0,ports[i].xf,ports[i].yf,0,0,ports[i].name);
		      gpr_$set_text_font(font_id_small,st);check(st);
#endif APOLLO
                	}

		}
}
/* ************************** set port sides *********************** */
where_port(pmax)
int pmax;
{
int i, j, port_n, n, nthfr, nthto, jfr, jto;
/*

   set from_node & to_node values to the subscript, nth, for nodes[nth]
	this is slow & cumbersome - find a faster way

   set x0,y0 xf,yf 
*/		
	for(i = 0; i < pmax; i++){	/* ports */
		nthfr = -1;
		nthto = -1;
		for(j = 0; j < node_max; j++){	/* nodes */
			if(ports[i].from_node == nodes[j].node)	{
				nthfr = ports[i].nth_from = j;
				}
			if(ports[i].to_node == nodes[j].node) {
				nthto = ports[i].nth_to = j;
				}
			} /* nodes */
if(nthfr < 0 || nthto < 0) printf("read_port_data: %d to %d missing node\n",
	ports[i].from_node,ports[i].to_node);
/*
	check level to decide where to put port 
*/
		if(nodes[nthfr].lvl > nodes[nthto].lvl) {
			jfr = ports[i].side_from = TOP;
			jto = ports[i].side_to = BOT; 
			}
		else if(nodes[nthfr].lvl < nodes[nthto].lvl) {
			jfr = ports[i].side_from = BOT;
			jto = ports[i].side_to = TOP; }
		else { /* same lvl so see if next to each other */
			if(nodes[nthfr].nth == nodes[nthto].nth + 1) {
				jfr = ports[i].side_from = LHS;
				jto = ports[i].side_to = RHS;
				}
			else if(nodes[nthfr].nth == nodes[nthto].nth - 1) {
				jfr = ports[i].side_from = RHS;
				jto = ports[i].side_to = LHS;
				}
			else { /* not next to each other -- oh no! */
/*
	don't know how to deal with if not next to each other - do later
		for now put port on bottom
*/
				jfr = ports[i].side_from = BOT;
				jto = ports[i].side_to = BOT;
				}
			} /* else same lvl */
/*
	now find coordinates of the port 
*/
		set_port(jfr,nthfr,jto,nthto,i);

 /* later decide on angle depending on # of ports & other criteria */
		ports[nthto].angle = 0;
		} /* for i -- ports */
}
/* ************************** set port coordinates *********************** */
/*
	nside1,nthnode1 - from
	     2        2 - to
*/
set_port(nside1,nthnode1,nside2,nthnode2,ithport)
int nside1,nthnode1,nside2,nthnode2,ithport;
{
int i,nside,nthnode;
short xfr,yfr;
/*
	now find coordinates of the port 
*/
	for(i=0; i < 2; i++){ /* i=0:from  i=1:to */
		if(i == 0){
			nside = nside1;
			nthnode = nthnode1;
			}
		else{
			nside = nside2;
                        nthnode = nthnode2;
			}
		if(nside == TOP){
			xfr = 0;
			yfr = -nodes[nthnode].ht/2;
			}
		else if(nside == BOT){
			xfr = 0;
			yfr = nodes[nthnode].ht/2;
			}                
		else if(nside == RHS){
                        xfr = nodes[nthnode].wide/2;
			yfr = 0;
			}
		else {	/* LHS */
                        xfr = -nodes[nthnode].wide/2;
			yfr = 0;
			}
		if(i == 0) {
			ports[ithport].x0 = nodes[nthnode].x0 + xfr;
			ports[ithport].y0 = nodes[nthnode].y0 + yfr;
/* clipping */
/*			if(ports[ithport].x0 < MINX_WINDOW) 
				ports[ithport].x0 = MINX_WINDOW;
			else if(ports[ithport].x0 > MAXX_WINDOW)
				ports[ithport].x0 = MAXX_WINDOW;
			if(ports[ithport].y0 < MINY_WINDOW) 
				ports[ithport].y0 = MINY_WINDOW;
			else if(ports[ithport].y0 > MAXY_WINDOW)
				ports[ithport].y0 = MAXY_WINDOW;
*/
                        }
		else {
			ports[ithport].xf = nodes[nthnode].x0 + xfr;
			ports[ithport].yf = nodes[nthnode].y0 + yfr;
/* clipping */
/*			if(ports[ithport].xf < MINX_WINDOW) 
				ports[ithport].xf = MINX_WINDOW;
			else if(ports[ithport].xf > MAXX_WINDOW)
				ports[ithport].xf = MAXX_WINDOW;
			if(ports[ithport].yf < MINY_WINDOW) 
				ports[ithport].yf = MINY_WINDOW;
			else if(ports[ithport].yf > MAXY_WINDOW)
				ports[ithport].yf = MAXY_WINDOW;
*/
			}
		}
}

