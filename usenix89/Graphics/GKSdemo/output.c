#include <fcntl.h>
#include <tam.h>
#include <ctype.h>
#include "mongo.h"


	/* translate the file_x[] and file_y[] arrays into 
	   x[] and y[] arrays in NDC space, 
	   but don't round to integers in preparation for display
	   yet - do that later so that we don't "wrap around"
	   on the screen due to overflow. */

normalize()
{
	double fx, fy, cx, cy;
	int i;

	fx = (MAXX - MINX) / (maxx - minx);
	fy = (MAXY - MINY) / (maxy - miny);
	cx = MINX - fx * minx;
	cy = MINY - fy * miny;

	for (i = 0; i < num_read; i++) {
		x[i] = file_x[i] * fx + cx;
		y[i] = file_y[i] * fy + cy;
	}
}
	

	/* return 1 if the given point is inside the window, 0 otherwise */

inside(x, y)
double x, y;
{
	if ((x < MINX) || (x > MAXX) || (y < MINY) || (y > MAXY))
		return(0);
	return(1);
}


	/* put a point at each data point */

points()
{
	int i;
	double *px, *py;
	INT16 xy[2];

	wselect(graph_w);
	normalize();
	for (i = low_line - 1, px = x, py = y; 
				(i < num_read) && (i < high_line - 1); i++, px++, py++) {
		if (inside(*px, *py)) {
			xy[0] = *px;
			xy[1] = *py;
			v_pmarker(dev_crt, 1, xy);
		}
	}

#ifdef PRINTER
	if (PRINT)
		pr_points();
#endif

}


	/* connect the dots */

connect()
{
	int i;

	wselect(graph_w);
	normalize();
	for (i = low_line - 1; (i < high_line - 1) && (i < num_read - 1); i++)
		clip_n_draw(x[i], y[i], x[i+1], y[i+1]);

#ifdef PRINTER
	if (PRINT)
		pr_connect();
#endif

}


	/* draw a box around the graphics window area */

box()
{
	INT16 corner[10];
	INT16 old_style, new_style;	

	wselect(graph_w);
	new_style = SOLID;		/* solid line */
	vql_attributes(dev_crt, corner);
	old_style = corner[0];

	corner[0] = MINX;	corner[1] = MINY;
	corner[2] = MAXX;	corner[3] = MINY;
	corner[4] = MAXX;	corner[5] = MAXY;
	corner[6] = MINX;	corner[7] = MAXY;
	corner[8] = MINX;	corner[9] = MINY;
	
	vsl_type(dev_crt, new_style);
	v_pline(dev_crt, (INT16) 5, corner);
	vsl_type(dev_crt, old_style); 

#ifdef PRINTER
	if (PRINT)
		pr_box();
#endif

}


	/* place the given string at the current text position - 
	   if it is off-screen, do nothing and let him figure it
	   out. */

label(cmd_buf)
char *cmd_buf;
{
	double dx, dy;
	INT16 px, py;
	char *p;

	wselect(graph_w);
	/* skip over the word "label" and any blank space to get to the
	   text of the command */
	for (p = cmd_buf + 5; (isspace(*p)); p++)
		;

	/* if no text, just return */
	if (*p == '\0')
		return;

	world_to_ndc(x_text_pos, y_text_pos, &dx, &dy);
	if (inside(dx, dy))
		v_gtext(dev_crt, (px = dx), (py = dy), p);

#ifdef PRINTER
	if (PRINT) {
		pr_world_to_ndc(x_text_pos, y_text_pos, &dx, &dy);
		if (pr_inside(dx, dy))
			v_gtext(dev_prt, (px = dx), (py = dy), p);
	}
#endif

}

code(x, y)
double x, y;
{

	return(((y > MAXY) << 3) |
	       ((y < MINY) << 2) |
	       ((x > MAXX) << 1) |
	       ((x < MINX)));
}
	
#define SWAP  t = x1; u = y1; x1 = x2; y1 = y2; x2 = t; y2 = u; code1 = code2;

clip_n_draw(x1, y1, x2, y2)
double x1, y1, x2, y2;
{
	double t, u;
	INT16 xy[4];
	int code1, code2;

	while (1) {
		code1 = code(x1, y1);
		code2 = code(x2, y2);
		if (!(code1 | code2)) {
			xy[0] = x1; xy[1] = y1; xy[2] = x2; xy[3] = y2;
			v_pline(dev_crt, 2, xy);
			return;
		}
		else if (code1 & code2)
			return;
		else {

			/* first, make sure that point 1 is outside the window */
			if (!code1) {
				SWAP
			}

			if (code1 & 8) {
				x1 += (x2 - x1) * (MAXY - y1) / (y2 - y1);
				y1 = MAXY;
			}
			else if (code1 & 4) {
				x1 += (x2 - x1) * (MINY - y1) / (y2 - y1);
				y1 = MINY;
			}
			else if (code1 & 2) {
				y1 += (y2 - y1) * (MAXX - x1) / (x2 - x1);
				x1 = MAXX;
			}
			else if (code1 & 1) {
				y1 += (y2 - y1) * (MINX - x1) / (x2 - x1);
				x1 = MINX;
			}
		}	
	}
}
