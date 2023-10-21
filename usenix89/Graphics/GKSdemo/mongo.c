#include <fcntl.h>
#include <tam.h>
#include <ctype.h>
#include "mongo.h"


INT16 display[] = {
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
	'D', 'I', 'S', 'P', 'L', 'A', 'Y', ' '	/* device name */
};

char *line_type[] = {
	"SOLID",
	"LONG_DASH",
	"DOT",
	"DASH_DOT",
	"MEDIUM_DASH",
	"DASH_2_DOTS"
};

char *point_type[] = {
	"POINT",
	"PLUS",
	"ASTERISK",
	"CIRCLE",
	"CROSS",
	"DIAMOND"
};


main(argc, argv)
int argc; 
char *argv[];
{
	int i, j, k, num1, num2;
	char *gets(), cmd_buf[50], command[10];
	struct nlist *lookup(), *np;

	init();

	while (1) {
		command[0] = '\0';
		wselect (text_w);
		wprintf(text_w, "> ");
		wgets(text_w, cmd_buf);
		sscanf(cmd_buf, "%s", command);
		if ((np = lookup(command)) == NULL) {
			wprintf(text_w, "%s ?\n", command);
			continue;
		}
		(*(np->func))(cmd_buf);
	}
}


	/* set up two windows, one for the graphics, one for text and
	   stuff. select the text one, so that it is on top */

init()
{
	short placex = 1;
	short placey = 0;
	short height, width;
	unsigned short flags;
	struct utdata utd;
	struct nlist *install();
	INT16 dev_info[66], text_attrib[10], err_report;
	int my_clear(), get_file(), get_xc(), get_yc(), get_limits();
	int points(), connect(), box(), quit(), ltype(), ptype();
	int lines(), position(), label(), reverse();
	int draw_axes(), xlabel(), ylabel(), title();
#ifdef PRINTER
	int my_update();
	int init_print(), end_print(), my_print();
#endif

	winit();

	/* create the graphics window */

	height = 24;
	width = 80;
	flags = NBORDER;
	graph_w = wcreate(placex, placey, height, width, flags);
	utd.ut_num = WTXTLABEL;
	strcpy(utd.ut_text, "Mongo Graph");
	ioctl(graph_w, WIOCSETTEXT, &utd);

	/* create the text window */

	height = 4;
	width = 30;
	flags = BORDRESIZE;
	text_w = wcreate(placex, placey, height, width, flags);
	utd.ut_num = WTXTLABEL;
	strcpy(utd.ut_text, "Mongo Commands");
	ioctl(text_w, WIOCSETTEXT, &utd);

	/* open graphics workstation */
	wselect(graph_w);
	err_report = v_opnwk(display, &dev_crt, dev_info);
	if (err_report != 0) {
		wselect(text_w);
		wprintf(text_w,"error opening screen: %d\n", vq_error());
		wexit();
	}

	/* set graph borders and text borders */
	vqt_attributes(dev_crt, text_attrib);
	T_HEIGHT = text_attrib[9];
	T_WIDTH = text_attrib[8];
	MINX = 8 * T_WIDTH;
	MAXX = dev_info[51] - T_WIDTH;
	MINY = 4 * T_HEIGHT;
	MAXY = dev_info[52] - 2 * T_HEIGHT;

	/* set some characteristics of the graphics window */
	cur_p_type = PLUS;
	cur_l_type = DOT;
	vsm_type(dev_crt, cur_p_type);		/* plus signs as points */
	vsl_type(dev_crt, cur_l_type);		/* dotted line */

	/* set a few other things */
	low_line = 1;
	high_line = AR_SIZE;

	/* initialize printer and plotter flags */
	PRINT = 0;
	PLOT = 0;

	/* install commands in the command table */
	table_init();
	install("clear", my_clear);
	install("file", get_file);
	install("xc", get_xc);
	install("yc", get_yc);
	install("lines", lines);
	install("limits", get_limits);
	install("ltype", ltype);
	install("ptype", ptype);
	install("points", points);
	install("connect", connect);
	install("box", box);
	install("position", position);
	install("label", label);
	install("reverse", reverse);
	install("quit", quit);
	install("axes", draw_axes);
	install("xaxis", xlabel);
	install("yaxis", ylabel);
	install("title", title);

#ifdef PRINTER
	install("update", my_update);
	install("dump", my_print);
	install("printer", init_print);
	install("noprinter", end_print);
#endif

}


	/* in the second two arguments, put the NDC coordinates corresponding 
	   to the world coordinates given in the first two arguments */

world_to_ndc(wx, wy, nx, ny)
double wx, wy, *nx, *ny;
{
	double fx, fy, cx, cy;

	fx = (MAXX - MINX) / (maxx - minx);
	fy = (MAXY - MINY) / (maxy - miny);
	cx = MINX - fx * minx;
	cy = MINY - fy * miny;

	*nx = wx * fx + cx;
	*ny = wy * fy + cy;
}


	/* set the type of line used in connect(); with no arguments,
	   prints the current type */

ltype(cmd_buf)
char *cmd_buf;
{
	int i;
	char dummy[7], type[20];

	if (sscanf(cmd_buf, "%s %s", dummy, type) < 2)
		wprintf(text_w, "%s\n", line_type[cur_l_type - 1]);
	else {
		for (i = 0; i <= (MAX_L_TYPE - MIN_L_TYPE); i++)
			if (strcmp(type, line_type[i]) == 0) {
				cur_l_type = i + MIN_L_TYPE;
				vsl_type(dev_crt, cur_l_type);

#ifdef PRINTER
				if (PRINT)
					vsl_type(dev_prt, cur_l_type);
#endif

			}
	}
}


	/* set the type of point used in points(); with no arguments,
	   prints the current type */

ptype(cmd_buf)
char *cmd_buf;
{
	int i;
	char dummy[7], type[20];

	if (sscanf(cmd_buf, "%s %s", dummy, type) < 2)
		wprintf(text_w, "%s\n", point_type[cur_p_type - 1]);
	else {
		for (i = 0; i <= (MAX_P_TYPE - MIN_P_TYPE); i++)
			if (strcmp(type, point_type[i]) == 0) {
				cur_p_type = i + MIN_P_TYPE;
				vsm_type(dev_crt, cur_p_type);

#ifdef PRINTER
				if (PRINT)
					vsm_type(dev_prt, cur_p_type);
#endif

			}
	}
}


	/* clear the graphics window and the printer, if appropriate */

my_clear()
{
	wselect (graph_w);
	v_clrwk(dev_crt);

#ifdef PRINTER
	if (PRINT)
		v_clrwk(dev_prt);
#endif

}


	/* make a hard copy */

#ifdef PRINTER
my_print ()
{
	int err;

	wselect(graph_w);
	err = v_hardcopy(dev_crt);
	if (err != 0) {
		wselect(text_w);
		wprintf(text_w, "error hardcopy\n");
	}
}
#endif


	/* update printer workstation */

#ifdef PRINTER
my_update()
{
	if (PRINT)
		v_updwk(dev_prt);
}
#endif


	/* quit */

quit()
{
	wselect(graph_w);
	v_clrwk(dev_crt);
	v_clswk(dev_crt);

#ifdef PRINTER
	if (PRINT)
		v_clswk(dev_prt);
#endif

	wexit();
}
