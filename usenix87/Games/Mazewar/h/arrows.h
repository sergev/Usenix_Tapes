/* RCS Information: $Header: arrows.h,v 1.1 84/08/25 17:24:50 chris Exp $ */

#ifdef sun
short	down_arrow[10] = {
	0xFFFF, 0xF3FF, 0xF3FF, 0xF3FF, 0xF3FF, 
	0x807F, 0xC0FF, 0xE1FF, 0xF3FF, 0xFFFF
};
mpr_static(down_pr, 10, 10, 1, down_arrow);
struct pixrect	*px_down = &down_pr;

short	up_arrow[10] = {
	0xFFFF, 0xF3FF, 0xE1FF, 0xC0FF, 0x807F, 
	0xF3FF, 0xF3FF, 0xF3FF, 0xF3FF, 0xFFFF
};
mpr_static(up_pr, 10, 10, 1, up_arrow);
struct pixrect	*px_up = &up_pr;

short	left_arrow[10] = {
	0xFFFF, 0xF7FF, 0xE7FF, 0xC7FF, 0x807F, 
	0x807F, 0xC7FF, 0xE7FF, 0xF7FF, 0xFFFF
};
mpr_static(left_pr, 10, 10, 1, left_arrow);
struct pixrect	*px_left = &left_pr;

short	right_arrow[10] = {
	0xFFFF, 0xFBFF, 0xF9FF, 0xF8FF, 0x807F, 
	0x807F, 0xF8FF, 0xF9FF, 0xFBFF, 0xFFFF
};
mpr_static(right_pr, 10, 10, 1, right_arrow);
struct pixrect	*px_right = &right_pr;
#endif
