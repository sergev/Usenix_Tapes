#include <sys/window.h>

extern short text_w, graph_w;


reverse()
{
	struct urdata ur;
	struct uwdata uw;
	int i, j, k;
	unsigned short pat[16];

	ioctl(graph_w, WIOCGETD, &uw);
	ur.ur_height = uw.uw_height;
	ur.ur_width = uw.uw_width;

	for (i = 0; i < 16; i++)
		pat[i] = 65535;
	ur.ur_srcbase = 0;	
	ur.ur_srcwidth = 0;
	ur.ur_dstbase = 0;	
	ur.ur_dstwidth = 0;
	ur.ur_srcx = 0;
	ur.ur_srcy = 0;
	ur.ur_dstx = 0;	
	ur.ur_dsty = 0;	
	ur.ur_srcop = SRCXOR;
	ur.ur_dstop = DSTSRC;
	ur.ur_pattern = pat;

	ioctl(graph_w, WIOCRASTOP, &ur);
}

