#include <fcntl.h>
#include <tam.h>
#include <ctype.h>
#include "mongo.h"

#define 	fabs(x)		((x < 0) ? -x: x)


	/* draw the axes of the graph: if arguments are given, use them to
		scale the axes, otherwise, figure out appropriate scales */

draw_axes(cmd_buf)
char *cmd_buf;
{
	char dummy[10];
	double dx, dy;
	INT16 p[2], new_style, old_style;
	double get_units ();

	wselect(graph_w);

	/* draw a box around the graph */
	box();

	/* set line style to solid and save old line style */
	new_style = SOLID;
	vql_attributes(dev_crt, p);
	old_style = p[0];
	vsl_type(dev_crt, new_style);

	/* read scale interval from the command line */
	if (sscanf (cmd_buf, "%s %lf %lf", dummy, &dx, &dy) != 3) {
		/* since no intervals are given, determine suitable intervals */
		dx = get_units(maxx, minx, 8);
		dy = get_units(maxy, miny, 8);
	}

	xaxis(dx);
	yaxis(dy);

	/* restore default text justification and restore old line style */
	vst_alignment(dev_crt, 0, 0, &p[0], &p[1]);
	vsl_type(dev_crt, old_style);

#ifdef PRINTER
	if (PRINT) {
		pr_box();
		vsl_type(dev_prt, new_style);
		pr_xaxis(dx);
		pr_yaxis(dy);
		vst_alignment(dev_prt, 0, 0, &p[0], &p[1]);
		vsl_type(dev_prt, old_style);
	}
#endif

}


	/* draw the hash marks on the x axis and label them; also put the
		scaling factor if there is one */

xaxis(dx)
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
	if (fabs(maxx) > fabs(x1))
		f_to_e(maxx, &fmax, &imax);
	else
		f_to_e(x1, &fmax, &imax);

	/* draw hash marks and label them */
	vst_alignment(dev_crt, 1, 2, &p[0], &p[1]);
	p[1] = MINY + 150;
	p[3] = MINY - 150;
	for (x = x1; x <= maxx; x += dx) {
		world_to_ndc(x, miny, &ndcx, &ndcy);
		p[0] = ndcx;
		p[2] = ndcx;
		v_pline(dev_crt, (INT16) 2, p);
		f_to_e(x, &x1, &i);
		if (i < imax)
			for (; i < imax; i++)
				x1 /= 10;
		sprintf(dummy, "%-4.2lf", x1);
		v_gtext(dev_crt, p[0], p[3], dummy);
	}

	/* print scaling factor, if there is one */
	if (imax != 0) {
		/* left and top justify text */
		vst_alignment(dev_crt, 0, 2, &p[0], &p[1]);
		p[0] = MAXX - 8 * T_WIDTH;
		p[3] -= 2 * T_HEIGHT;
		v_gtext(dev_crt, p[0], p[3], "X 10");
		if (imax != 1) {
			sprintf(dummy, "%-5d", imax);
			p[0] += 4 * T_WIDTH;
			p[3] += T_HEIGHT / 2;
			v_gtext(dev_crt, p[0], p[3], dummy);
		}
	}
}


	/* draw hash marks on the y axis and label them; also put the scaling
		factor if there is one */

yaxis(dy)
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
	if (fabs(maxy) > fabs(y1))
		f_to_e(maxy, &fmax, &imax);
	else
		f_to_e(y1, &fmax, &imax);

	/* put hash marks and label them */
	vst_alignment(dev_crt, 2, 1, &p[0], &p[1]);
	p[0] = MINX - 150;
	p[2] = MINX + 150;
	for (y = y1; y <= maxy; y += dy) {
		world_to_ndc(minx, y, &ndcx, &ndcy);
		p[1] = ndcy;
		p[3] = ndcy;
		v_pline(dev_crt, (INT16) 2, p);
		f_to_e(y, &y1, &i);
		if (i < imax)
			for (; i < imax; i++)
				y1 /= 10;
		sprintf(dummy, "%-4.2lf", y1);
		v_gtext(dev_crt, p[0], p[1], dummy);
	}

	/* print scaling factor, if there is one */
	if (imax != 0) {
		/* right and bottom justify text */
		vst_alignment(dev_crt, 2, 0, &p[0], &p[1]);
		sprintf(dummy, "X 10");
		p[0] = MINX - 3 * T_WIDTH;
		p[1] = MAXY;
		v_gtext(dev_crt, p[0], p[1], dummy);
		if (imax != 1) {
			sprintf(dummy, "%-3d", imax);
			p[0] = MINX;
			p[1] += T_WIDTH / 2;
			v_gtext(dev_crt, p[0], p[1], dummy);
		}
	}
}


	/* find suitable intervals for scaling, given the max and min of the
		scale and the number of intervals desired */

double
get_units(max, min, n)
double max, min;
int	n;
{
	double dx, x;
	int i, j, xint;
	double e_to_f ();

	dx = (max - min) / n;

	/* convert dx to scientific notation */
	f_to_e (dx, &x, &i);

	/* pick a somewhat neat interval for x so that the scale looks nice */
	switch (xint = x) {
		case 3:
		case 4:
		case 6:
		case 7: {
			x = 5;
			break;
		}
		case 8:
		case 9: {
			x = 10;
			break;
		}
		default: {
			x = xint;
			break;
		}
	}

	/* convert scientific notation to decimal */
	dx = e_to_f (x, i);
	return (dx);
}


	/* label x axis */

xlabel(cmd_buf)
char	*cmd_buf;
{

	INT16 px, py;
	char *p;

	wselect(graph_w);
	/* skip over the word "xaxis" and any blank space to get to the label */
	for (p = cmd_buf + 5; (isspace(*p)); p++)
		;

	/* if no text, just return */
	if (*p == '\0')
		return;

	/* position text and print */
	vst_alignment(dev_crt, 1, 2, &px, &py); 
	px = MINX + (MAXX  - MINX) / 2;
	py = MINY - 2 * T_HEIGHT;
	v_gtext(dev_crt, px, py, p);

	/* set text justification back to default */
	vst_alignment(dev_crt, 0, 0, &px, &py); 

#ifdef PRINTER
	if (PRINT) {
		/* do same to printer */
		vst_alignment(dev_prt, 1, 2, &px, &py); 
		px = PR_MINX + (PR_MAXX  - PR_MINX) / 2;
		py = PR_MINY - 2 * PR_T_HEIGHT;
		v_gtext(dev_prt, px, py, p);
		vst_alignment(dev_prt, 0, 0, &px, &py); 
	}
#endif

}


	/* label y axis */

ylabel(cmd_buf)
char	*cmd_buf;
{

	INT16 px, py;
	char *p;

	wselect(graph_w);
	/* skip over the word "yaxis" and any blank space to get to the label */
	for (p = cmd_buf + 5; (isspace(*p)); p++)
		;

	/* if no text, just return */
	if (*p == '\0')
		return;

	/* rotate the text baseline; set justification to bottom center */
	vst_rotation(dev_crt, 900);
	vst_alignment(dev_crt, 1, 0, &px, &py); 

	/* print text */
	px = MINX - 6 * T_WIDTH;
	py = MINY + (MAXY - MINY) / 2;
	v_gtext(dev_crt, px, py, p);

	/* put text settings back to default */
	vst_rotation(dev_crt, 0);
	vst_alignment(dev_crt, 0, 0, &px, &py); 

#ifdef PRINTER
	if (PRINT) {
		/* do same thing to printer */
		vst_rotation(dev_prt, 900);
		vst_alignment(dev_prt, 1, 0, &px, &py); 
		px = PR_MINX - 6 * PR_T_WIDTH;
		py = PR_MINY + (PR_MAXY - PR_MINY) / 2;
		v_gtext(dev_prt, px, py, p);
		vst_rotation(dev_prt, 0);
		vst_alignment(dev_prt, 0, 0, &px, &py); 
	}
#endif

}


	/* put a title on the graph */

title(cmd_buf)
char	*cmd_buf;
{

	INT16 px, py;
	char *p;

	wselect(graph_w);
	/* skip over the word "title" and any blank space to get to the label */
	for (p = cmd_buf + 5; (isspace(*p)); p++)
		;

	/* if no text, just return */
	if (*p == '\0')
		return;

	/* print text */
	vst_alignment(dev_crt, 1, 2, &px, &py); 
	px = MINX + (MAXX - MINX) / 2;
	py = MAXY + T_HEIGHT;
	v_gtext(dev_crt, px, py, p);

	/* set text justification back to default */
	vst_alignment(dev_crt, 0, 0, &px, &py); 

#ifdef PRINTER
	if (PRINT) {
		/* do same thing to printer */
		vst_alignment(dev_prt, 1, 2, &px, &py); 
		px = PR_MINX + (PR_MAXX - PR_MINX) / 2;
		py = PR_MAXY + PR_T_HEIGHT;
		v_gtext(dev_prt, px, py, p);
		vst_alignment(dev_prt, 0, 0, &px, &py); 
	}
#endif

}
