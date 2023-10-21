/*
*	This file contains printer related functions.
*	Compile this file and other modules with -DPRINTER only if there is
*	a printer.
*/

#include <fcntl.h>
#include <tam.h>
#include <ctype.h>
#include "mongo.h"


INT16 printer[] = {
	1,	/* equal axis ratios */
	1,	/* solid line type */
	1,	/* line color index to use - white */
	3,	/* marker type - star */
	1,	/* marker color index - white */
	1,	/* text font to use - normal */
	1,	/* text color - white */
	0,	/* fill interior style - hollow */
	0,	/* fill style index - plain */
	1,	/* fill color index - white */
	1,	/* send device messages to screen */
	'P', 'R', 'I', 'N', 'T', 'E', 'R', ' '	/* device name */
};



	/* set up printer as graphics output device */

init_print()
{
	INT16 dev_info[66], text_attrib[10], border, err_report;

	/* open printer workstation */
	PRINT = 1;
	err_report = v_opnwk(printer, &dev_prt, dev_info);
	if (err_report != 0) {
		wselect(text_w);
		wprintf(text_w,"error opening printer: %d\n", vq_error());
		return;
	}

	/* set graph borders and text borders */
	vqt_attributes(dev_prt, text_attrib);
	PR_T_HEIGHT = text_attrib[9];
	PR_T_WIDTH = text_attrib[8];
	border = dev_info[51] / 20;
	PR_MAXX = dev_info[51] - border;
	PR_MINX = 2 * border;
	PR_MAXY = dev_info[52] - 2 * border;
	PR_MINY = 3 * border;

	/* set point and line characteristics as the current one for the screen */
	vsm_type(dev_prt, cur_p_type);
	vsl_type(dev_prt, cur_l_type);
}


	/* translate the file_x[] and file_y[] arrays into 
	   pr_x[] and pr_y[] arrays in NDC space for the printer, 
	   but don't round to integers in preparation for display
	   yet - do that later so that we don't "wrap around"
	   on the screen due to overflow. */

pr_normalize()
{
	double fx, fy, cx, cy;
	int i;

	fx = (PR_MAXX - PR_MINX) / (maxx - minx);
	fy = (PR_MAXY - PR_MINY) / (maxy - miny);
	cx = PR_MINX - fx * minx;
	cy = PR_MINY - fy * miny;

	for (i = 0; i < num_read; i++) {
		pr_x[i] = file_x[i] * fx + cx;
		pr_y[i] = file_y[i] * fy + cy;
	}
}
	

	/* return 1 if the given point is inside the graph space, 0 otherwise */

pr_inside(x, y)
double x, y;
{
	if ((x < PR_MINX) || (x > PR_MAXX) || (y < PR_MINY) || (y > PR_MAXY))
		return(0);
	return(1);
}


	/* put a point at each data point */

pr_points()
{
	int i;
	double *px, *py;
	INT16 xy[2];

	pr_normalize();
	for (i = low_line - 1, px = pr_x, py = pr_y; 
				(i < num_read) && (i < high_line - 1); i++, px++, py++) {
		if (pr_inside(*px, *py)) {
			xy[0] = *px;
			xy[1] = *py;
			v_pmarker(dev_prt, 1, xy);
		}
	}
}


	/* connect the dots */

pr_connect()
{
	int i;

	pr_normalize();
	for (i = low_line - 1; (i < high_line - 1) && (i < num_read - 1); i++)
		pr_clip_n_draw(pr_x[i], pr_y[i], pr_x[i+1], pr_y[i+1]);
}


	/* draw a box around the graphics area */

pr_box()
{
	INT16 corner[10];
	INT16 old_style, new_style;	

	new_style = SOLID;		/* solid line */
	vql_attributes(dev_prt, corner);
	old_style = corner[0];

	corner[0] = PR_MINX;	corner[1] = PR_MINY;
	corner[2] = PR_MAXX;	corner[3] = PR_MINY;
	corner[4] = PR_MAXX;	corner[5] = PR_MAXY;
	corner[6] = PR_MINX;	corner[7] = PR_MAXY;
	corner[8] = PR_MINX;	corner[9] = PR_MINY;
	
	vsl_type(dev_prt, new_style);
	v_pline(dev_prt, (INT16) 5, corner);
	vsl_type(dev_prt, old_style); 
}


	/* in the second two arguments, put the NDC coordinates corresponding 
	   to the world coordinates given in the first two arguments */

pr_world_to_ndc(wx, wy, nx, ny)
double wx, wy, *nx, *ny;
{
	double fx, fy, cx, cy;

	fx = (PR_MAXX - PR_MINX) / (maxx - minx);
	fy = (PR_MAXY - PR_MINY) / (maxy - miny);
	cx = PR_MINX - fx * minx;
	cy = PR_MINY - fy * miny;

	*nx = wx * fx + cx;
	*ny = wy * fy + cy;
}


	/* put hash marks on the x axis and label them; also put scaling factor
		if there is one */

pr_xaxis(dx)
double dx;
{
	char dummy[10];
	double x1, x, ndcx, ndcy;
	int imax, i;
	double fmax;
	INT16 p[4];

	/* find the position of the first marker on the x axis */
	x1 = ((int)(minx / dx)) * dx;
	if (x1 < minx)
		x1 += dx;
	f_to_e(maxx, &fmax, &imax);

	/* put hash marks and label them */
	vst_alignment (dev_prt, 1, 2, &p[0], &p[1]);
	p[1] = PR_MINY + 150;
	p[3] = PR_MINY - 150;
	for (x = x1; x <= maxx; x += dx) {
		pr_world_to_ndc(x, miny, &ndcx, &ndcy);
		p[0] = ndcx;
		p[2] = ndcx;
		v_pline(dev_prt, (INT16) 2, p);
		f_to_e(x, &x1, &i);
		if (i < imax)
			for (; i < imax; i++)
				x1 /= 10;
		sprintf(dummy, "%-4.2lf", x1);
		v_gtext(dev_prt, p[0], p[3], dummy);
	}

	/* print scaling factor, if there is one */
	if (imax != 0) {
		vst_alignment(dev_prt, 0, 2, &p[0], &p[1]);
		p[0] = PR_MAXX - 8 * PR_T_WIDTH;
		p[3] -= 1.5 * PR_T_HEIGHT;
		v_gtext(dev_prt, p[0], p[3], "X 10");
		if (imax != 1) {
			sprintf(dummy, "%-5d", imax);
			p[0] += 4 * PR_T_WIDTH;
			p[3] += PR_T_HEIGHT / 2;
			v_gtext(dev_prt, p[0], p[3], dummy);
		}
	}
}


	/* print hash marks on the y axis and label them; also put scaling
		factor if there is one */

pr_yaxis (dy)
double dy;
{
	char dummy[10];
	double y1, y, ndcx, ndcy;
	int imax, i;
	double fmax;
	INT16 p[4];

	/* find the position of the first marker on the y axis */
	y1 = ((int)(miny / dy)) * dy;
	if (y1 < miny)
		y1 += dy;
	f_to_e(maxy, &fmax, &imax);

	/* put hash marks and label them */
	vst_alignment (dev_prt, 2, 1, &p[0], &p[1]);
	p[0] = PR_MINX - 150;
	p[2] = PR_MINX + 150;
	for (y = y1; y <= maxy; y += dy) {
		pr_world_to_ndc(minx, y, &ndcx, &ndcy);
		p[1] = ndcy;
		p[3] = ndcy;
		v_pline(dev_prt, (INT16) 2, p);
		f_to_e(y, &y1, &i);
		if (i < imax)
			for (; i < imax; i++)
				y1 /= 10;
		sprintf(dummy, "%-4.2lf", y1);
		v_gtext(dev_prt, p[0], p[1], dummy);
	}

	/* print scaling factor, if there is one */
	if (imax != 0) {
		vst_alignment(dev_prt, 2, 0, &p[0], &p[1]);
		sprintf(dummy, "X 10");
		p[0] = PR_MINX - 3 * PR_T_WIDTH;
		p[1] = PR_MAXY;
		v_gtext(dev_prt, p[0], p[1], dummy);
		if (imax != 1) {
			sprintf(dummy, "%-3d", imax);
			p[0] = PR_MINX;
			p[1] += PR_T_WIDTH / 2;
			v_gtext(dev_prt, p[0], p[1], dummy);
		}
	}
}


	/* close printer workstation */

end_print ()
{
	if (PRINT == 0)
		return;
	v_clswk(dev_prt);
	PRINT = 0;
}

pr_code(x, y)
double x, y;
{

	return(((y > PR_MAXY) << 3) |
	       ((y < PR_MINY) << 2) |
	       ((x > PR_MAXX) << 1) |
	       ((x < PR_MINX)));
}
	
#define SWAP  t = x1; u = y1; x1 = x2; y1 = y2; x2 = t; y2 = u; code1 = code2;

pr_clip_n_draw(x1, y1, x2, y2)
double x1, y1, x2, y2;
{
	double t, u;
	INT16 xy[4];
	int code1, code2;

	while (1) {
		code1 = pr_code(x1, y1);
		code2 = pr_code(x2, y2);
		if (!(code1 | code2)) {
			xy[0] = x1; xy[1] = y1; xy[2] = x2; xy[3] = y2;
			v_pline(dev_prt, 2, xy);
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
				x1 += (x2 - x1) * (PR_MAXY - y1) / (y2 - y1);
				y1 = PR_MAXY;
			}
			else if (code1 & 4) {
				x1 += (x2 - x1) * (PR_MINY - y1) / (y2 - y1);
				y1 = PR_MINY;
			}
			else if (code1 & 2) {
				y1 += (y2 - y1) * (PR_MAXX - x1) / (x2 - x1);
				x1 = PR_MAXX;
			}
			else if (code1 & 1) {
				y1 += (y2 - y1) * (PR_MINX - x1) / (x2 - x1);
				x1 = PR_MINX;
			}
		}	
	}
}
