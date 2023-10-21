#include "tr.incl.h"
/*
prnt_in_box(k,xcenter,ycenter,del_x)
rectangle(xcenter,ycenter,horiz_side_length,vert_side_length)
arrow2(x0,y0,xf,yf,xmid,ymid,label)
init()
text(x0,y0,titles)
check(st)
setbutton(c,buttons)
clearbutton(buttons)
closest(position)
edge(position)
circle(xcenter,ycenter,radius)
arrow(x0,y0,xf,yf,xmid,ymid,label)
int read_port_data(askacg)
int read_node_data(askacg)
int read_limits()
*/
/* ************************************************************ */
prnt_in_box(k,xcenter,ycenter,del_x)
int k;
short xcenter,ycenter,del_x;
{
char str[128],strip[128];
short xpt,ypt,xspacing;
int nchars,j,j0,line,nlines,l,i,iflg;
float frac_nlines,length;
/* print name */
		sprintf(str,"%s",nodes[k].name);
		length = strlen(str) - 1;
                i = 0;
                while(i< length && ispunct(str[i]) == 0) i++; 
/*                while(i< length && strncmp(str[i],"[",1) != 0) i++; */
		nchars = nodes[k].wide/char_width_text; 
                frac_nlines = i/nchars;
                nlines = frac_nlines;
                if (nlines < frac_nlines)
                   nlines++;
                if(i < length) nlines++;
                iflg = 0;
		line = 0;
		j0 = 0;
/*	while(j0 < length){ */
	while(j0 < i){
		for(l=0;l<nchars+1;l++) strip[l] = '\0';
/*		if(j0+nchars > length) nchars = length - j0; */
		if(j0+nchars > i) nchars = i - j0;
		for(j=j0;j<j0+nchars; j++)
			strip[j-j0] = str[j];
                xpt = xcenter - (char_width_text*strlen(strip) - 2)/2;    
                if(iflg == 1) line++;
		if (nodes[k].node < 100)
                 ypt = ycenter + char_height_text/2 - ((nlines-1)*char_height_text)/2 + line*char_height_text; 
                else
                 ypt = ycenter + line*char_height_text;
		if(xpt < 0) xpt = 20;
		if(xpt > MAXX_WINDOW[1])
			printf("xpt = %ld is out of window\nCannot print: %s\n",(long) xpt,str);
		else {
		      gpr_$move(xpt,ypt,st);check(st);
		      gpr_$text(strip,(short) nchars,st);check(st);
			}
		line++;
		j0 += nchars;
                if(j0 >= i){ 
			nchars = length - i;
			i = length;
                        iflg = 1;
                        }
		} /* while */
}
/* ************************************************************ */
rectangle(xcenter,ycenter,horiz_side_length,vert_side_length)
short xcenter, ycenter, horiz_side_length,vert_side_length;
{
short lenx, leny, xxx[10],yyy[10];
	lenx = horiz_side_length;
	leny = vert_side_length;

	xxx[3] = xxx[4] = xxx[0] = xcenter - lenx/2;
	yyy[4] = yyy[1] = yyy[0] = ycenter - leny/2;
	xxx[2] = xxx[1] = xcenter + lenx/2;
	yyy[3] = yyy[2] = ycenter + leny/2;

	gpr_$move(xxx[0],yyy[0],st); check(st);
	gpr_$polyline(xxx,yyy,(short)5,st); check(st);
}

/* ************************************************************ */
/* 
	arrow2 double lined arrow from x0,y0 to xf,yf 
		angle is half-angle of arrowhead

*/
arrow2(x0,y0,xf,yf,xmid,ymid,label)
short x0,y0,xf,yf,xmid,ymid;
char label[128];
{
int quadrant, isignx, isigny, flgmidpt;
float angle,phi,r,tmp,tmpx,tmpy,rtip;
short xxx[100], yyy[100], xpt, ypt, npt, savex0, savey0;
int length,i,j;
char strip[40];

	angle = PI/12.0; /* head of arrow = 2 * angle = 30 degrees */
/*		phi = angle betw line & x-axis */
	flgmidpt = 0; /* flag = 1 if there's a midpoint */
/* if there's a midpoint arrows has direction of xmid,ymid to xf,yf
		instead of x0,y0 to xf,yf
*/
	if(xmid > 0 && ymid > 0){
		flgmidpt = 1;
		savex0 = x0;
		savey0 = y0;
		x0 = xmid;
		y0 = ymid;
		}
	tmpy = yf-y0;
	tmpx = xf-x0;
	if(fabs(tmpx) > 0.001) phi = atan(fabs(tmpy/tmpx));
	else phi = PI/2.0;
	quadrant = 1; /* xf < x0 & yf > y0 */
	if(xf < x0 && yf < y0)  quadrant = 2;
	else if(xf > x0 && yf < y0)  quadrant = 3;
	else if(xf > x0 && yf > y0)  quadrant = 4;
	else if(xf > x0 && y0 == yf) quadrant = 3;
	else if(xf < x0 && y0 == yf) quadrant = 1;
	else if(xf == x0 && y0 < yf) quadrant = 1;
	else if(xf == x0 && y0 > yf) quadrant = 3;

	r = 10;
	rtip = 14;
	if(quadrant == 1) phi = - phi;
	else if(quadrant == 3){
		phi = -phi;
		r = -r; rtip = -rtip;
		}
	else if(quadrant == 4){
		 r = -r; rtip = -rtip; }

	if(flgmidpt == 1){
		x0 = savex0;
		y0 = savey0;
		}
if(flgmidpt == 0){
	xxx[2] = xf + r*cos(phi+angle);
	yyy[2] = yf + r*sin(phi+angle);
	xxx[3] = xf + r*cos(phi-angle);
	yyy[3] = yf + r*sin(phi-angle);
	xxx[4] = xxx[0] = xxx[3] + x0 - xf;
	ypt = yyy[3] + y0 - yf;
if(yf == y0 || (yf < y0 && ypt <= y0) || (yf > y0 && ypt >= y0))
	yyy[4] = yyy[0] = ypt;
/* else if(yf > y0 && ypt >= y0)
	yyy[4] = yyy[0] = ypt;
*/
else yyy[4] = yyy[0] = y0;
	xxx[1] = xxx[2] + x0 - xf;
	ypt = yyy[2] + y0 - yf;
if(yf == y0 || (yf < y0 && ypt <= y0) || (yf > y0 && ypt >= y0))
	yyy[1] = ypt;
/* else if(yf > y0 && ypt >= y0)
	yyy[1] = ypt;
*/
else yyy[1] = y0;
	npt = 5;  
        }
else {
        xxx[2] = xmid + r*cos(phi+angle);
        yyy[2] = ymid + r*sin(phi+angle);
	xxx[3] = xf + r*cos(phi+angle);
	yyy[3] = yf + r*sin(phi+angle);
	xxx[4] = xf + r*cos(phi-angle);
	yyy[4] = yf + r*sin(phi-angle);
	xxx[6] = xxx[0] = xxx[4] + x0 - xf;
	yyy[6] = yyy[0] = yyy[4] + y0 - yf;
	xxx[1] = xxx[3] + x0 - xf;
	yyy[1] = yyy[3] + y0 - yf;
        xxx[5] = xmid + r*cos(phi-angle);
	yyy[5] = ymid + r*sin(phi-angle);
	npt = 7;  
	}
	gpr_$move(xxx[0],yyy[0],st); check(st);
	gpr_$polyline(xxx,yyy,npt,st); check(st);

	angle = PI/4.0; 
/* arrowhead */
	if(flgmidpt == 0){
		xxx[3] = xxx[0] = xf; yyy[3] = yyy[0] = yf;
		xxx[1] = xf + rtip*cos(phi-angle);
		yyy[1] = yf + rtip*sin(phi-angle);
		xxx[2] = xf + rtip*cos(phi+angle);
		yyy[2] = yf + rtip*sin(phi+angle);
		npt = 4;
		}
	else{
	 	xxx[5] = xxx[2] =xf; yyy[5] = yyy[2] = yf;
		xxx[1] = xmid; yyy[1] = ymid;
		xxx[3] = xf + r*cos(phi-angle);
		yyy[3] = yf + r*sin(phi-angle);
		xxx[4] = xf + r*cos(phi+angle);
		yyy[4] = yf + r*sin(phi+angle);
		npt = 6;
		}

	gpr_$move(xxx[0],yyy[0],st); check(st);
	gpr_$polyline(xxx,yyy,npt,st); check(st);
/* 2 lines of port label if there's a [...] phrase */
	if(yf == y0){
		if(xf <= x0){
			xpt = x0 - 2 * (strlen(label) - 1) * char_size;
			ypt = y0 + char_size;
			}
		else {
			xpt = x0 + 3 * char_size;
			ypt = y0 + char_size;
			}
		}
	else {
		ypt = y0 - char_size;   /*  BOXHT/2; */
		xpt = x0 - (strlen(label) - 1) * char_size;
                }
        text(xpt,ypt,label);
/*
			for(j=0;j<i; j++)
			strip                i = 0;
                while(ispunct(label[i]) == 0 && i< length) i++;
		for(j=0;j<length;j++) strip[j] = '\0';
		for(j=0;j<i; j++)
			strip[j] = label[j];
*/
/*
	if(flgmidpt == 0){ 
		xpt = x0 + (xf-x0)/5 - (strlen(strip) - 1) * char_size;
		if(yf > y0) ypt = y0 + 3*char_size; 
                else ypt = y0 - BOXHT/2 - 3*char_size;
                }
	else{
		xpt = xmid - (strlen(strip) -1) * char_size;
		ypt = ymid -3;
		}
	        text(xpt,ypt,strip);
	if(i < length - 1){
		for(j=0;j<length;j++) strip[j] = '\0';
		for(j=i;j<length; j++)
			strip[j-i] = label[j];
	if(flgmidpt == 0){ 
		xpt = x0 + (xf-x0)/5 - (strlen(strip) - 1) * char_size;
		if(yf > y0) ypt = y0 + 6*char_size;
                else ypt = y0 - BOXHT/2;
                }
	else{
		xpt = xmid - (strlen(strip) - 1) * char_size;
		ypt = ymid -3 + 3*char_size;
		}
	        text(xpt,ypt,strip);
                }
*/
/*
	if(flgmidpt == 0){ */ /* no midpoint */
/*		xpt = x0 + (xf-x0)/2 - strlen(label-1) * char_size;
		ypt = y0 + 4*(yf-y0)/5 -3;
		}
	else{
		xpt = xmid - strlen(label) * char_size;
		ypt = ymid -3;
		}
	text(xpt,ypt,label);
*/
}
/* ************************************************************ */
init()
{
 gpr_$offset_t disp_bm_size,mem_size;
/* gpr_$bitmap_desc_t init_bitmap,mem_bm; */
 gpr_$attribute_desc_t attrib_bl_desc;
 short position[2];
 pad_$window_desc_t local_graphic_window_desc;
 stream_$id_t local_stream;

	local_graphic_window_desc.top    = MINY_WINDOW[0];
	local_graphic_window_desc.left   = MINX_WINDOW[0];
	local_graphic_window_desc.width  = MAXX_WINDOW[0];
	local_graphic_window_desc.height = MAXY_WINDOW[0];
	disp_bm_size.x_size = MAXX_WINDOW[0] - MINX_WINDOW[0];
	disp_bm_size.y_size = MAXY_WINDOW[0] - MINY_WINDOW[0];

	scl = 1.0; mnx = MINX_WINDOW[0]; mny = MINY_WINDOW[0];

	pad_$create_window("",0,pad_$transcript,1,
        	local_graphic_window_desc,local_stream,st);
        
        pad_$set_auto_close(local_stream,1,true,st); check(st); 
/* memory bitmap size */
	MINX_WINDOW[1] = 0;
	MINY_WINDOW[1] = 0;
	MAXX_WINDOW[1] = mem_size.x_size = (short) 4096;
	MAXY_WINDOW[1] = mem_size.y_size = (short) 2048;

/*        setbutton('q',btn); 
        setbutton('c',buttons);
        setbutton('b',buttons);
        setbutton('B',buttons); 
*/
        setbutton('q',keys);
        setbutton(KBD_$L4,keys);
        setbutton(KBD_$L6,keys);
        setbutton(KBD_$L7,keys);
        setbutton(KBD_$L8S,keys);
        setbutton(KBD_$L9,keys);
        setbutton(KBD_$LAS,keys);
        setbutton(KBD_$LCS,keys);
        setbutton(KBD_$LD,keys);
        setbutton(KBD_$LES,keys);
        setbutton(KBD_$LF,keys); 

      gpr_$init(gpr_$direct,local_stream,disp_bm_size,0,init_bitmap,st);check(st);
      gpr_$set_obscured_opt(gpr_$block_if_obs,st);
      gpr_$set_auto_refresh(true,st);

      gpr_$enable_input(gpr_$keystroke,keys,st); check(st);
/*      gpr_$enable_input(gpr_$buttons,buttons,st);check(st); */
      gpr_$enable_input(gpr_$locator,0,st);check(st);
      gpr_$set_cursor_active(true,st); check(st);

	gpr_$allocate_attribute_block(attrib_bl_desc,st); check(st);
	gpr_$allocate_bitmap(mem_size,(short) 0,attrib_bl_desc,mem_bm,st); check(st);
	gpr_$set_bitmap(mem_bm,st); check(st);

/* fonts */
      gpr_$load_font_file("/sys/dm/fonts/f5x7",18,font_id_small,st); check(st);
      gpr_$set_text_font(font_id_small,st);check(st);
      gpr_$inq_character_width(font_id_small,"R",char_width,st); check(st);
	char_sz_sml = (long) char_width/2.0;
      gpr_$load_font_file("/sys/dm/fonts/scvc5x10.r.b",26,font_id,st); check(st);
      gpr_$set_text_font(font_id,st);check(st);
      gpr_$inq_character_width(font_id,"u",char_width,st); check(st);
	char_size = (long) char_width/2.0;

      char_width_text = 5 + 2;
      char_height_text = 7 + 1;
}
/* ************************************************************ */
text(x0,y0,titles)
short x0,y0;
gpr_$string_t titles;
{
      gpr_$move(x0,y0,st);check(st);
      gpr_$text(titles,(short) strlen(titles),st);check(st);
}
/* ************************************************************ */
check(st)
status_$t st;
{ if(st.all != status_$ok) error_$print(st);
}
/* ************************************************************ */
setbutton(c,buttons)
char c; int *buttons;
{int j;
 j=c&255; buttons[7-j/32] |= 1<<(j%32);
}
/* ************************************************************ */
clearbutton(buttons)
int *buttons;
{int i; 
 for(i=0;i<4;i++) buttons[i]=0;
};
/* ************************************************************ */
closest(position)
short *position;
{
int i, imin, dmin, d, s, t, namesz;
char str[128];
short where[2];

	dmin=4000000; /*arbitrary large number*/
	for(i=0;i<node_max;i++){
		s=position[0]-x[i];
		t=position[1]-y[i];
		d=s*s+t*t;
		if(dmin>d) {imin=i;dmin=d;};
		}
	sprintf(str,"%s",nodes[imin].name);

/* see if name can fit on one line, else go to 2 or more lines */
	namesz = strlen(nodes[imin].name);
	delta_x = (MAXX_WINDOW[0] - MINX_WINDOW[0])/(nodes[imin].of + 1);	
	if(char_size *namesz <= delta_x){
		s = x[imin] - char_size * namesz;
		if (s < 0) s = 0;                     
		}
	else{	/* split line up */
		s = x[imin] - char_size * namesz;
		if (s < 0) s = 0;                     
		}
	t = y[imin] + 2*RADIUS/3;
	if(t > MAXY_WINDOW[0]) t = y[imin];

	gpr_$set_cursor_active(false,st); check(st);

	if(nodes[imin].flg == 0) {
		gpr_$set_draw_value(gpr_$black,st);
		nodes[imin].flg = 1;
		}
	else {
		gpr_$set_draw_value(gpr_$white,st);
		nodes[imin].flg = 0;
		}
	where[0] = s; where[1] = t;
	gpr_$set_cursor_position(where,st);

	gpr_$move((short) s,(short) t,st);check(st);
	gpr_$text(str,(short) strlen(str),st);check(st);

	gpr_$set_cursor_active(true,st); check(st);

	gpr_$event_wait(event_type,keystroke,position,st);check(st);
}       
/* ************************************************************ */
edge(position)
short position[2];
{int i,ip; float s,t,u,v,dd,d,e,dmin;
 short x1[3],y1[3],l;
 ip= -1;
 dmin=40000000.; /*arbitrary large number*/
 for(i=0;i<len;i++)
 { s=x[i+1]-x[i]; t=y[i+1]-y[i]; dd=sqrt(s*s+t*t);
   if(dd<=0) fprintf(stderr,"distance=0\n");
   s/=dd; t/=dd;
   u=position[0]-x[i]; v=position[1]-y[i];
   d=u*t-v*s; d=d*d;  e=u*s+v*t;
  if((dmin>d)&&(e>0)&&(e<dd)) {dmin=d; ip=i+1;};
 };
 if(ip<0) return;

 len++;
 for(i=len;i>ip;i--)
 {  x[i]=x[i-1];
    y[i]=y[i-1];
 }
/* {  x[i]=x[i-1];  xx[i]=xx[i-1]; 
    y[i]=y[i-1];  yy[i]=yy[i-1];
 }
*/
gpr_$set_cursor_active(false,st); check(st);
i=ip-1; x1[0]=x[i];x1[2]=x[i+2];y1[0]=y[i];y1[2]=y[i+2];
x1[1]=(x1[0]+x1[2])/2;y1[1]=(y1[0]+y1[2])/2;
l=3;
for(;;)
{
 gpr_$set_draw_value(gpr_$black,st);
 gpr_$move(x[i],y[i],st);check(st);
 gpr_$polyline(x1,y1,l,st); 
 x[ip] =x1[1]  =position[0];  /* xx[ip] = (position[0]-10)/scl + mnx; */
 y[ip] =y1[1]  =position[1];  /* yy[ip] = (position[1]-10)/scl + mny; */
 gpr_$set_draw_value(gpr_$white,st);
 gpr_$move(x[i],y[i],st);check(st);
 gpr_$polyline(x1,y1,l,st); 
                
 gpr_$event_wait(event_type,keystroke,position,st);check(st);
    switch(event_type)
    {case  gpr_$locator: break;
     case  gpr_$buttons: 
      if(keystroke=='B'){ gpr_$set_cursor_active(true,st); check(st);
                          gpr_$set_cursor_position(position,st);
                          return;}
    } 
}
}

/* ************************************************************ */
circle(xcenter,ycenter,radius)
short xcenter,ycenter,radius;
{
gpr_$position_t center;

	center.x_coord = xcenter; center.y_coord = ycenter;
	gpr_$circle(center,radius,st);
	check(st);
}
/* ************************************************************ */
/* 
	arrow from x0,y0 to xf,yf ; angle is half-angle of arrowhead
*/
arrow(x0,y0,xf,yf,xmid,ymid,label)
short x0,y0,xf,yf,xmid,ymid;
char label[128];
{
int quadrant, isignx, isigny, flgmidpt;
float angle,phi,r,tmp,tmpx,tmpy;
short xxx[100], yyy[100], xpt, ypt, npt, savex0, savey0;

	angle = PI/12.0; /* head of arrow = 2 * angle = 30 degrees */
/*		phi = angle betw line & x-axis */
	flgmidpt = 0; /* flag = 1 if there's a midpoint */
/* if there's a midpoint arrows has direction of xmid,ymid to xf,yf
		instead of x0,y0 to xf,yf
*/
	if(xmid > 0 && ymid > 0){
		flgmidpt = 1;
		savex0 = x0;
		savey0 = y0;
		x0 = xmid;
		y0 = ymid;
		}
	tmpy = yf-y0;
	tmpx = xf-x0;
	if(fabs(tmpx) > 0.001) phi = atan(fabs(tmpy/tmpx));
	else phi = PI/2.0;
	quadrant = 1; /* xf < x0 & yf > y0 */
	if(xf < x0 && yf < y0)  quadrant = 2;
	else if(xf > x0 && yf < y0)  quadrant = 3;
	else if(xf > x0 && yf > y0)  quadrant = 4;
	else if(xf > x0 && y0 == yf) quadrant = 3;
	else if(xf < x0 && y0 == yf) quadrant = 1;
	else if(xf == x0 && y0 < yf) quadrant = 1;
	else if(xf == x0 && y0 > yf) quadrant = 3;

	r = 10;
	if(quadrant == 1) phi = - phi;
	else if(quadrant == 3){
		phi = -phi;
		r = -r;
		}
	else if(quadrant == 4) r = -r;

	if(flgmidpt == 1){
		x0 = savex0;
		y0 = savey0;
		}

	xxx[0] = x0; yyy[0] = y0;
	if(flgmidpt == 0){
		xxx[4] = xxx[1] =xf; yyy[4] = yyy[1] = yf;
		xxx[2] = xf + r*cos(phi-angle);
		yyy[2] = yf + r*sin(phi-angle);
		xxx[3] = xf + r*cos(phi+angle);
		yyy[3] = yf + r*sin(phi+angle);
		npt = 5;
		}
	else{
		xxx[5] = xxx[2] =xf; yyy[5] = yyy[2] = yf;
		xxx[1] = xmid; yyy[1] = ymid;
		xxx[3] = xf + r*cos(phi-angle);
		yyy[3] = yf + r*sin(phi-angle);
		xxx[4] = xf + r*cos(phi+angle);
		yyy[4] = yf + r*sin(phi+angle);
		npt = 6;
		}

	gpr_$move(xxx[0],yyy[0],st); check(st);
	gpr_$polyline(xxx,yyy,npt,st); check(st);

	if(yf == y0){
		if(xf <= x0){
			xpt = x0 - 2 * (strlen(label) - 1) * char_size;
			ypt = y0 + char_size;
			}
		else {
			xpt = x0 + 3 * char_size;
			ypt = y0 + char_size;
			}
		}
	else {
		ypt = y0 - char_size;   /*  BOXHT/2; */
		xpt = x0 - (strlen(label) - 1) * char_size;
                }
        text(xpt,ypt,label);



/*	if(flgmidpt == 0){ */ /* no midpoint */
/*		xpt = x0 + (xf-x0)/2 - strlen(label) * char_size;
		ypt = y0 + (yf-y0)/2 + 3;
		}
	else{
		xpt = xmid - strlen(label) * char_size;
		ypt = ymid + 3;
		}
*/
/*	printf("\t\txpt=%ld ypt=%ld name=%s\n",(long)xpt,(long)ypt,label); */
	text(xpt,ypt,label);
}
/* 
 ************************* read port data *********************************** 
*/
int read_port_data(askacg)
int askacg;
{
FILE *datafile, *fopen();
char str[128],file[40];
int i, j, port_n, n, pmax, nthfr, nthto, jfr, jto;
/* start with node already in correct order, later do sorting */

	i = 0;
	port_n = 0;
	pmax = 0;
	if(askacg > 0 || askacg == -1){
		printf("enter port file name: ");
		scanf("%s",file);
		if((datafile = fopen(file,"r")) == NULL)
			printf("can't open port file: %s\n",file);
		}
	else if((datafile = fopen(PORT_FILE,"r")) == NULL)
		printf("can't open %s\n",PORT_FILE);
/*	else if((datafile = fopen("acgport.data","r")) == NULL)
		printf("can't open acgport.data\n");
*/
	while ( fscanf(datafile,"%s",str) !=  EOF) {
		if(strncmp(str,".",1) != 0){
			switch(i){
			case 0:	
				strcat(ports[port_n].name,str); 
				strcat(ports[port_n].name," ");
				break;
			case 1:	ports[port_n].from_node = atoi(str);
				break;
			case 2:	ports[port_n].to_node = atoi(str);
				pmax++;
				break;
			default:
	printf("error in reading ports - hit default\n");
			break;
			} /*switch */
		}  /* if strncmp */
		else i++; /* incr i when find a period */
		if(i > 2){
			n = port_n;
			port_n++;
			i = 0;
			}
	}  /* while fscanf */
	fclose(datafile);
return(pmax);
}
/*
 ************************ read node data ****************************
*/
int read_node_data(askacg)
int askacg;
{
FILE *datafile, *fopen();
char str[128],file[40];
int i, j, node_n, n, nmax, l, m;
/* start with node already in correct order, later do sorting */

	i = 0; j = 0; nmax = 0;
	node_n = 0;
	if(askacg > 0 || askacg == -1){
		printf("enter node file name: ");
		scanf("%s",file);
		if((datafile = fopen(file,"r")) == NULL)
			printf("can't open node file: %s\n",file);
		}
	else if((datafile = fopen(NODE_FILE,"r")) == NULL)
		printf("can't open %s\n",NODE_FILE);
/*	else if((datafile = fopen("acgnode.data","r")) == NULL)
		printf("can't open acgnode.data\n");
*/
	while ( fscanf(datafile,"%s",str) !=  EOF) {
		if(strncmp(str,".",1) == 0) {
			j = 0;
			i++;
			}
		else {
			switch(i){
			case 0:	
				for(m=0;m<5;m++)
					nodes[node_n].target[m] = 0;
				nodes[node_n].node = atoi(str);	break;
			case 1:	nodes[node_n].target[j] = atoi(str);
				j++;		break;
			case 2:	nodes[node_n].source[j] = atoi(str);
				j++;	break;
			case 3:	nodes[node_n].cohort[j] = atoi(str);
				j++;	break;
			case 4:	strcat(nodes[node_n].name,str); 
				strcat(nodes[node_n].name," ");
				break;
			case 5: nodes[node_n].lvl = atoi(str);
				break;
			case 6: nodes[node_n].nth = atoi(str);
				break;
			case 7: nodes[node_n].of = atoi(str);
				break;
			case 8: if(strncmp(str,"B",1) == 0) { /* box */
					nodes[node_n].shape = 1;
					nodes[node_n].wide = (short) BOXWIDTH;
					nodes[node_n].ht = (short) BOXHT;
					}
				else {	/* circle */
					nodes[node_n].shape = 0;
					nodes[node_n].wide = (short) RADIUS;
					nodes[node_n].ht = (short) RADIUS;
					}
				nmax++;
				break;
			default:
			nodes[node_n].flg = 0;
			node_n++;
			nodes[node_n].node = atoi(str);
			i = 0; j = 0;
				break;
			} /*switch */
		}  /* else switch */
	}  /* while fscanf */
n = node_n;
	fclose(datafile);
/* set target */
for(j=0;j<nmax;j++){
	i=0;
	while(nodes[j].source[i] != 0){
		m = 0; /* find subscript(m) of node which is the source */
		while(nodes[j].source[i] != nodes[m].node && m < nmax) m++;
/* printf("node=%d source=%d node[%d]=%d\n",nodes[j].node, nodes[j].source[i],m,nodes[m].node); */
		l = 0; /* find first free target */
		while(nodes[m].target[l] != 0) l++;
		nodes[m].target[l] = nodes[j].node;
/*
printf("child nodes[%d]=%d target[%d]=%d parent=%d\n",m,nodes[m].node,l,
	nodes[m].target[l],nodes[j].node);
*/
		i++;
		}
	}
	return(nmax);
}
/* ************************************************************ */
/* 
		read limits from data file acglimits
*/
int read_limits()
{
FILE *datafile, *fopen();
char str[128];
int i,j,j0;
	i = -1;
	if((datafile = fopen("acglimits.data","r")) == NULL)
		printf("can't open acglimits\n");
	while ( fscanf(datafile,"%s",str) !=  EOF) {
		if(ispunct(str[0]) == 0) { /* ==0 false    !=0 true */
/*		if(isalpha(str[0]) == 0) { */
			i++;
			switch(i){
			case 0: 
                                j = 0;
				while((GRAPH_TITLE[j]=str[j]) != '\0')
                                	j++;
                                j0 = j+1;
                                strncat(GRAPH_TITLE," ",1);
				while(fscanf(datafile,"%s",str) != 0 && ispunct(str[0]) == 0) { 
        	                        j = 0;
					while((GRAPH_TITLE[j0+j]=str[j]) != '\0')
        	                        	j++;
        	                        j0 = j0+j+1;
        	                        strncat(GRAPH_TITLE," ",1);
        	                        }
				break;
			case 1: 
                                j = 0;
				while((NODE_FILE[j]=str[j]) != '\0')
                                	j++;
				break;
			case 2: 
                                j = 0;
				while((PORT_FILE[j]=str[j]) != '\0')
                                	j++;
				break;
			case 3:	BOXWIDTH = atoi(str);	
				break;
			case 4:	BOXHT = atoi(str);
				break;
			case 5:	RADIUS = atoi(str);
				break;
			case 6:	NODE_LABEL = atoi(str);
				break;
			case 7: MAXX_WINDOW[0] = atoi(str);
				break;
			case 8: MAXY_WINDOW[0] = atoi(str);
				break;
			case 9: MINX_WINDOW[0] = atoi(str);
				break;
			case 10: MINY_WINDOW[0] = atoi(str);
				break;
			case 11: MIN_X_SPACE = atoi(str);
				break;
			case 12: MIN_Y_SPACE = atoi(str);
				break;
			default:
				printf("read_limits: reached default. bah!\n");
				break;
			} /*switch */
		}  /* if isalpha */
	}  /* while fscanf */

	fclose(datafile);
	return;
}

