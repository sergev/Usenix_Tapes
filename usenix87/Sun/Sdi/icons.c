/********************************  icons.c  ****************************/
#include "sdi.h"
#include <sunwindow/notify.h>

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * This file contains the code and data to keep the icon busy.
 */

#define ICON_TYPE Icon *

/*
 * The standard macro forces background pixel writing, 
 * which causes icon flicker
 */
#define	MARKS_OWN_DEFINE_ICON_FROM_IMAGE(name, image)				\
	static struct mpr_data CAT(name,_data) = {			\
	    mpr_linebytes(ICON_DEFAULT_WIDTH,1), (short *)(image),	\
	    {0, 0}, 0, 0};						\
	extern struct pixrectops mem_ops;				\
	static struct pixrect CAT(name,_mpr) = {			\
	    &mem_ops, ICON_DEFAULT_WIDTH, ICON_DEFAULT_HEIGHT, 1,	\
	    (caddr_t)&CAT(name,_data)};					\
	static struct icon name = {					\
	    ICON_DEFAULT_WIDTH, ICON_DEFAULT_HEIGHT,			\
	    (struct pixrect *)0,					\
	    0, 0, ICON_DEFAULT_WIDTH, ICON_DEFAULT_HEIGHT,		\
	    &CAT(name,_mpr),						\
	    0, 0, ICON_DEFAULT_WIDTH, ICON_DEFAULT_HEIGHT,		\
	    NULL, (struct pixfont *)0, 0};


static short icon1_image[] = {
#include "city_icon1.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(icon1, icon1_image);

static short icon2_image[] = {
#include "city_icon2.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(icon2, icon2_image);

static short icon3_image[] = {
#include "city_icon3.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(icon3, icon3_image);

static short icon4_image[] = {
#include "city_icon4.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(icon4, icon4_image);

static short icon5_image[] = {
#include "city_icon5.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(icon5, icon5_image);

static short icon6_image[] = {
#include "city_icon6.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(icon6, icon6_image);

static short icon7_image[] = {
#include "city_icon7.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(icon7, icon7_image);

static short icon8_image[] = {
#include "city_icon8.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(icon8, icon8_image);

static short fancy_icon1_image[] = {
#include "fancy_icon1.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon1, fancy_icon1_image);

static short fancy_icon2_image[] = {
#include "fancy_icon2.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon2, fancy_icon2_image);

static short fancy_icon3_image[] = {
#include "fancy_icon3.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon3, fancy_icon3_image);

static short fancy_icon4_image[] = {
#include "fancy_icon4.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon4, fancy_icon4_image);

static short fancy_icon5_image[] = {
#include "fancy_icon5.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon5, fancy_icon5_image);

static short fancy_icon6_image[] = {
#include "fancy_icon6.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon6, fancy_icon6_image);

static short fancy_icon7_image[] = {
#include "fancy_icon7.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon7, fancy_icon7_image);

static short fancy_icon8_image[] = {
#include "fancy_icon8.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon8, fancy_icon8_image);

static short fancy_icon9_image[] = {
#include "fancy_icon9.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon9, fancy_icon9_image);

static short fancy_icon10_image[] = {
#include "fancy_icon10.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon10, fancy_icon10_image);

static short fancy_icon11_image[] = {
#include "fancy_icon11.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon11, fancy_icon11_image);

static short fancy_icon12_image[] = {
#include "fancy_icon12.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon12, fancy_icon12_image);

static short fancy_icon13_image[] = {
#include "fancy_icon13.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon13, fancy_icon13_image);

static short fancy_icon14_image[] = {
#include "fancy_icon14.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon14, fancy_icon14_image);

static short fancy_icon15_image[] = {
#include "fancy_icon15.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon15, fancy_icon15_image);

static short fancy_icon16_image[] = {
#include "fancy_icon16.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon16, fancy_icon16_image);

static short fancy_icon17_image[] = {
#include "fancy_icon17.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon17, fancy_icon17_image);

static short fancy_icon18_image[] = {
#include "fancy_icon18.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon18, fancy_icon18_image);

static short fancy_icon19_image[] = {
#include "fancy_icon19.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon19, fancy_icon19_image);

static short fancy_icon20_image[] = {
#include "fancy_icon20.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon20, fancy_icon20_image);

static short fancy_icon21_image[] = {
#include "fancy_icon21.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon21, fancy_icon21_image);

static short fancy_icon22_image[] = {
#include "fancy_icon22.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon22, fancy_icon22_image);

static short fancy_icon23_image[] = {
#include "fancy_icon23.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon23, fancy_icon23_image);

static short fancy_icon24_image[] = {
#include "fancy_icon24.h"
};
MARKS_OWN_DEFINE_ICON_FROM_IMAGE(fancy_icon24, fancy_icon24_image);

ICON_TYPE fancy_icon_list[] = {
	(ICON_TYPE)&fancy_icon1,
	(ICON_TYPE)&fancy_icon2,
	(ICON_TYPE)&fancy_icon3,
	(ICON_TYPE)&fancy_icon4,
	(ICON_TYPE)&fancy_icon5,
	(ICON_TYPE)&fancy_icon6,
	(ICON_TYPE)&fancy_icon7,
	(ICON_TYPE)&fancy_icon8,
	(ICON_TYPE)&fancy_icon9,
	(ICON_TYPE)&fancy_icon10,
	(ICON_TYPE)&fancy_icon11,
	(ICON_TYPE)&fancy_icon12,
	(ICON_TYPE)&fancy_icon13,
	(ICON_TYPE)&fancy_icon14,
	(ICON_TYPE)&fancy_icon15,
	(ICON_TYPE)&fancy_icon16,
	(ICON_TYPE)&fancy_icon17,
	(ICON_TYPE)&fancy_icon18,
	(ICON_TYPE)&fancy_icon19,
	(ICON_TYPE)&fancy_icon20,
	(ICON_TYPE)&fancy_icon21,
	(ICON_TYPE)&fancy_icon22,
	(ICON_TYPE)&fancy_icon23,
	(ICON_TYPE)&fancy_icon24,
	};

int MAX_FANCY_ICON = sizeof(fancy_icon_list)/sizeof(ICON_TYPE);


ICON_TYPE icon_list[] = {
	(ICON_TYPE)&icon1,
	(ICON_TYPE)&icon2,
	(ICON_TYPE)&icon3,
	(ICON_TYPE)&icon4,
	(ICON_TYPE)&icon5,
	(ICON_TYPE)&icon6,
	(ICON_TYPE)&icon7,
	(ICON_TYPE)&icon8
	};

int MAX_ICON = sizeof(icon_list)/sizeof(ICON_TYPE);

static int ICON_USECS = 500000;
static int ICON_SECS = 0;
static int icon_update_time = 5;
static int icon_update_type = 1;

static int icon_no = 0;

init_icons()
{
	extern int starting_icon;
	extern int starting_icon_time;
	icon_update_type = starting_icon;
	icon_update_time = starting_icon_time;
}

/*
 * Update the icon through a rotating list of possible pixrects.
 * Started from within the file main.c when  a window close event
 * is detected.
 */
Notify_value
update_icon()
{
	extern int running_icon_pictures;
	switch (icon_update_type) {
		case 0: break;
		case 1: {
			icon_no = (icon_no + 1) % MAX_ICON;
			window_set(controlframe, FRAME_ICON, icon_list[icon_no], 0);
			break;
		}
		case 2: {
			icon_no = (icon_no + 1) % MAX_FANCY_ICON;
			window_set(controlframe, FRAME_ICON, fancy_icon_list[icon_no], 0);
			break;
		}
	}
	if (running_icon_pictures) {
		ICON_SECS = icon_update_time / 10;
		ICON_USECS = (icon_update_time % 10) * 100000;
		do_with_delay(update_icon, ICON_SECS, ICON_USECS);
	}
	return NOTIFY_DONE;
}

static Frame icon_option_frame;

icon_option_done()
{
	window_set(icon_option_frame, FRAME_NO_CONFIRM, TRUE, 0);
	window_destroy(icon_option_frame);
	icon_option_frame = 0;
	resume_proc();
}

icon_update_time_proc(item, value, event)
Panel_item item;
int value;
Event *event;
{
	icon_update_time = value;
}

icon_update_type_proc(item, value, event)
Panel_item item;
int value;
Event *event;
{
	icon_update_type = value;
	if (icon_update_type == 0) {
		window_set(controlframe, FRAME_ICON, icon_list[0], 0);
	}
}

/*
 * Called as a notify proc from the main control panel.
 */
icon_option_proc()
{
	extern int starting_icon;
	extern struct pixfont *buttonfont; /* use 'struct pixfont' for 3.0 compatiblity */
	Panel panel, make_popup_panel();
	if ((panel = make_popup_panel("     SDI Icon Options", NULL)) == NULL) {
		return;
	}
	(void) panel_create_item(panel, PANEL_CYCLE,
		PANEL_LABEL_STRING, " Icon:",
		PANEL_CHOICE_STRINGS, "Plain", "Subtle", "Wild", 0,
		PANEL_VALUE, icon_update_type,
		PANEL_NOTIFY_PROC, icon_update_type_proc,
		0);
	(void) panel_create_item(panel, PANEL_SLIDER,
		PANEL_NOTIFY_LEVEL, PANEL_ALL,
		PANEL_NOTIFY_PROC, icon_update_time_proc,
		PANEL_ITEM_X, ATTR_COL(0),
		PANEL_ITEM_Y, ATTR_ROW(1),
		PANEL_LABEL_STRING, "Icon update (tenths of secs): ", 
		PANEL_NOTIFY_LEVEL, PANEL_ALL,
		PANEL_SLIDER_WIDTH, 100,
		PANEL_MIN_VALUE, 1,
		PANEL_MAX_VALUE, 50,
		PANEL_SHOW_RANGE, FALSE,
		PANEL_SHOW_VALUE, TRUE,
		PANEL_VALUE, icon_update_time,
		PANEL_SHOW_ITEM, TRUE,
		0);
	display_popup_panel(panel);
	}
