#ifndef lint
static	char sccsid[] = "@(#)vt100tool.c 1.3 86/04/15 Copyr 1985 MITRE Corp.";
#endif


/*
 * MITRE Corp. Bedford, MA
 */

/*
 * 	Overview:	panelvttool: A shell subwindow emulating
 *                        a vt100 terminal
 *
 */

/*
 *      Author: Edward L. Lafferty
 */

#include <suntool/tool_hs.h>

#include <suntool/ttysw.h>
#include <stdio.h>
#include <suntool/panel.h>
#include "vconfig.h"
#include <suntool/ttysw_impl.h>
static short ic_image[258] = {
#include "vshelltool.icon"
};

#include <suntool/ttytlsw.h>
#include <sunwindow/window_hs.h>
#include <pixrect/pixrect_hs.h>
#include <sundev/kbio.h>
#include <sundev/kbd.h>


extern int winheightp, winwidthp;

mpr_static(shellic_mpr, 64, 64, 1, ic_image);

static	struct icon icon = {64, 64, (struct pixrect *)NULL, 0, 0, 64, 64,
	    &shellic_mpr, 0, 0, 0, 0, (char *)NULL, (struct pixfont *)NULL,
	    ICON_BKGRDGRY};


	    static	sigwinchcatcher(), sigchldcatcher(),
	    	sigtermcatcher(),setup_proc(), keyboard_proc(), button_proc();

static	struct tool *tool;
struct toolsw *ttysw;

static  struct toolsw *setupsw, *arrowsw, *keypadsw, *normkeysw;
static struct singlecolor scr_foreground = {100,100,100};
static struct singlecolor scr_background = {0, 0, 0};
static  caddr_t setup_button, keyboard_button;

static  caddr_t  local_choice, scroll_choice, screen_choice,
                    cursor_choice,xon_choice,ansi_choice,wrap_choice,
		    newline_choice, size_choice, answerback_text, baud_rate,
		    label_choice;
static  caddr_t  PF1_button, PF2_button, PF3_button, PF4_button, R1_button,
		 R2_button,R3_button,R4_button,R5_button,R6_button,
		 R7_button,R8_button,R9_button,R0_button,enter_button,
		 dot_button,comma_button,up_button,down_button,
		 left_button,right_button, minus_button;


static  local_proc(), scroll_proc(), screen_proc(),
		    cursor_proc(),xon_proc(),ansi_proc(),wrap_proc(),
		    newline_proc(), size_proc(), baud_rate_proc(),
		    answer_proc(), label_proc(), norm_proc();

static	char *normalname = "VT100 Shell 2.0";
static  Panel setup, arrow, keypad, normkey;
static struct pixfont *font;

/* so we can set locally
   and still inform the emulator*/
extern int newline, wrap,cursor,vt52mode,big_screen,rev_screen, local;
extern char  answer_message[];

/* defined in the emulator */
extern char* translate_key();
extern char main_font_directory[];

main(argc, argv)
	int argc;
	char **argv;
{
	char	*toolname = normalname;
	char *tool_name = argv[0];
	char **tool_attrs = NULL;


	if(tool_parse_all(&argc,argv,&tool_attrs, tool_name) == -1)
	  {
	    tool_usage(tool_name);
	    exit(1);
	  }
	/*
	 * Create tool window
	 */
	tool = tool_make(WIN_NAME_STRIPE,TRUE,
			 WIN_ICON, &icon,
			 WIN_LABEL,normalname,
			 WIN_COLUMNS,83,
			 WIN_WIDTH, 680,
			 WIN_HEIGHT, 600,
			 0);

	tool_free_attribute_list(tool_attrs);
	/*
	 * Create tty subwindow
	 * Libsuntool now has vt100 code instead of ansi
	 * Communicate with emulator via envirionment variable VTFONTS
	 */
	setenv("VTFONTS", MAIN_FONT_DIR); 
	ttysw = ttysw_createtoolsubwindow(tool, "ttysw", TOOL_SWEXTENDTOEDGE,
	    384);
	if (ttysw == (struct toolsw *)NULL)
		exit(1);
	
	setupsw = panel_create(tool, 0);
	setup   = (Panel) setupsw->ts_data;

	strcpy(main_font_directory,MAIN_FONT_DIR);

	/* create the items for setup of vt100 screen */
	setup_button = panel_create_item(setup,PANEL_BUTTON,

					 PANEL_NOTIFY_PROC,setup_proc,

					 PANEL_LABEL_IMAGE,
					 panel_button_image(setup,
							    "Setup",
							    7,
							    NULL ),
					 0);

	/* Set up the mouse driven keyboard */

	keyboard_button = panel_create_item(setup,PANEL_BUTTON,
					    PANEL_NOTIFY_PROC,keyboard_proc,
					    PANEL_LABEL_IMAGE,
					    panel_button_image(setup,
							       "Keyboard",
							    8,
							    NULL ),
					 0);
	label_choice = panel_create_item(setup, PANEL_CHOICE,
					 PANEL_SHOW_ITEM, TRUE,
					 PANEL_LABEL_STRING, "Key Labels",
					 PANEL_CHOICE_STRINGS,"Normal", "IBM",
					 			"MRED","WD11",
					 0,
					 PANEL_NOTIFY_PROC, label_proc,
					 PANEL_DISPLAY_LEVEL, PANEL_ALL,
					 PANEL_FEEDBACK, PANEL_INVERTED,
					 0);


	local_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"Online","Local",0,
		PANEL_NOTIFY_PROC, local_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);

	scroll_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"Smooth","Jump",0,
		PANEL_NOTIFY_PROC, scroll_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);

	screen_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"Dark","Light",0,
		PANEL_NOTIFY_PROC, screen_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);

	cursor_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"Block","Under",0,
		PANEL_NOTIFY_PROC, cursor_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);

	xon_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"Xon","No-xon",0,
		PANEL_NOTIFY_PROC, xon_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);

	wrap_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"No Wrap","Wrap",0,
		PANEL_NOTIFY_PROC, wrap_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);

	newline_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"Newline","No Newline",0,
		PANEL_NOTIFY_PROC, newline_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);

	ansi_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"ANSI","VT-52",0,
		PANEL_NOTIFY_PROC, ansi_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);

	size_choice = panel_create_item(setup,PANEL_CHOICE,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_CHOICE_STRINGS,"80 Col","132 Col",0,
		PANEL_NOTIFY_PROC, size_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
					     0);
	answerback_text = panel_create_item(setup,PANEL_TEXT,
		PANEL_SHOW_ITEM, FALSE,
		PANEL_LABEL_STRING, "Answerback:",
                PANEL_NOTIFY_LEVEL, PANEL_NONE,
		PANEL_VALUE, "",
                PANEL_VALUE_DISPLAY_LENGTH, 20,
		0);
/*	baud_rate = panel_create_item(setup, PANEL_CHOICE,
	        PANEL_LABEL_STRING, "Baud RATE",
                PANEL_CHOICE_STRINGS,"75","110","150","300","600","1200",
				      "2400","4800","9600", "19200",0,
                PANEL_NOTIFY_PROC, baud_rate_proc,
		PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
                PANEL_SHOW_ITEM,FALSE,
                0);
*/

	panel_set(setup,
		  PANEL_HEIGHT, PANEL_FIT_ITEMS,
		  PANEL_WIDTH, PANEL_CU(80),
		  0);

	/* Set up keypad */
	/* Client data item is used by key process to determine which
	   key was moused. */

	keypadsw = panel_create(tool,
				PANEL_HEIGHT, PANEL_CU(27),
				PANEL_WIDTH, PANEL_CU(30),   
				PANEL_ITEM_X_GAP,1,
				PANEL_ITEM_Y_GAP,1,
				0);
	keypad = (Panel) keypadsw->ts_data;

	PF1_button = panel_create_item(keypad, PANEL_BUTTON,
				       PANEL_SHOW_ITEM, FALSE,
				       PANEL_CLIENT_DATA, (int)11,
				       PANEL_NOTIFY_PROC, button_proc,
				       PANEL_LABEL_IMAGE,
				       panel_button_image(keypad,
							  "PF1",
							  5,
							  NULL),
				       0);

	PF2_button = panel_create_item(keypad, PANEL_BUTTON,
				       PANEL_SHOW_ITEM, FALSE,
				       PANEL_CLIENT_DATA, (int)12,
				       PANEL_NOTIFY_PROC, button_proc,
				       PANEL_LABEL_IMAGE,
				       panel_button_image(keypad,
							  "PF2",
							  5,
							  NULL),
				       0);
	PF3_button = panel_create_item(keypad, PANEL_BUTTON,
				       PANEL_SHOW_ITEM, FALSE,
				       PANEL_CLIENT_DATA, (int)13,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "PF3",
						       5,
						       NULL),
				    0);

	PF4_button = panel_create_item(keypad, PANEL_BUTTON,
				       PANEL_SHOW_ITEM, FALSE,
				       PANEL_CLIENT_DATA, (int) 14,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "PF4",
						       5,
						       NULL),
				    0);

	R7_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 7,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "7",
						       5,
						       NULL),
				    0);

	R8_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 8,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "8",
						       5,
						       NULL),
				    0);

	R9_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 9,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "9",
						       5,
						       NULL),
				    0);

	minus_button = panel_create_item(keypad, PANEL_BUTTON,
					 PANEL_SHOW_ITEM, FALSE,
					 PANEL_CLIENT_DATA, 15,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "-",
						       5,
						       NULL),
				    0);

	R4_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 4,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "4",
						       5,
						       NULL),
				    0);

	R5_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 5,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "5",
						       5,
						       NULL),
				    0);

	R6_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 6,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "6",
						       5,
						       NULL),
				    0);

	comma_button = panel_create_item(keypad, PANEL_BUTTON,
					 PANEL_SHOW_ITEM, FALSE,
					 PANEL_CLIENT_DATA, 16,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       ",",
						       5,
						       NULL),
				    0);

	R1_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 1,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "1",
						       5,
						       NULL),
				    0);

	R2_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 2,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "2",
						       5,
						       NULL),
				    0);

	R3_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 3,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "3",
						       5,
						       NULL),
				    0);
	enter_button = panel_create_item(keypad, PANEL_BUTTON,
					 PANEL_SHOW_ITEM, FALSE,
					 PANEL_CLIENT_DATA, 17,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "Enter",
						       5,
						       NULL),
				    0);
	R0_button = panel_create_item(keypad, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 10,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       "0",
						       13,
						       NULL),
				    0);

	dot_button = panel_create_item(keypad, PANEL_BUTTON,
				       PANEL_SHOW_ITEM, FALSE,
				       PANEL_CLIENT_DATA, 18,
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(keypad,
						       ".",
						       5,
						       NULL),
				    0);

	arrowsw = panel_create(tool,  0);
	arrow = (Panel) arrowsw->ts_data;

	up_button = panel_create_item(arrow, PANEL_BUTTON,
				      PANEL_SHOW_ITEM, FALSE,
				      PANEL_CLIENT_DATA, 19,
				      PANEL_ITEM_X, PANEL_CU(8),
				      PANEL_ITEM_Y, PANEL_CU(1),
				      PANEL_NOTIFY_PROC, button_proc,
				      PANEL_LABEL_IMAGE,
				      panel_button_image(arrow,
							 "^",
							 5,
							 NULL),
				      0);
	down_button = panel_create_item(arrow, PANEL_BUTTON,
					PANEL_SHOW_ITEM, FALSE,
					PANEL_CLIENT_DATA, 21,
					PANEL_ITEM_X, PANEL_CU(8),
					PANEL_ITEM_Y, PANEL_CU(3),
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(arrow,
						       "v",
						       5,
						       NULL),
				    0);
	left_button = panel_create_item(arrow, PANEL_BUTTON,
					PANEL_SHOW_ITEM, FALSE,
					PANEL_CLIENT_DATA, 22,
					PANEL_ITEM_X, PANEL_CU(2),
					PANEL_ITEM_Y, PANEL_CU(2),
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(arrow,
						       "<--",
						       5,
						       NULL),
				    0);
	right_button = panel_create_item(arrow, PANEL_BUTTON,
					 PANEL_SHOW_ITEM, FALSE,
					 PANEL_CLIENT_DATA, 20,
					 PANEL_ITEM_X, PANEL_CU(15),
					 PANEL_ITEM_Y, PANEL_CU(2),
				    PANEL_NOTIFY_PROC, button_proc,
				    PANEL_LABEL_IMAGE,
				    panel_button_image(arrow,
						       "-->",
						       5,
						       NULL),
				    0);
	panel_fit_height(arrow);



	label_setup();


        /*
	 * Install tool in tree of windows
	 */
	signal(SIGWINCH, sigwinchcatcher);
	tool_install(tool);

	/*
	 * Start tty process
	 */
	signal(SIGCHLD, sigchldcatcher);
	signal(SIGTERM, sigtermcatcher);

	if (ttysw_fork(ttysw->ts_data, ++argv,
	    &ttysw->ts_io.tio_inputmask,
	    &ttysw->ts_io.tio_outputmask,
	    &ttysw->ts_io.tio_exceptmask) == -1) {
		perror("vt100tool");
		exit(1);
	}
	/*
	 * Handle input
	 */
	tool_select(tool, 1 /* means wait for child process to die*/);
	/*
	 * Cleanup
	 */
	tool_destroy(tool);
	exit(0);
}

static
sigchldcatcher()
{
	tool_sigchld(tool);
}

static
sigwinchcatcher()
{
	tool_sigwinch(tool);
}

static
sigtermcatcher()
{
	ttysw_done(ttysw->ts_data);
	exit(0);
}
static int insetup = 0;
static int inkeyboard = 0;

static
setup_proc(item,event)
     Panel_item item;
     struct inputevent *event;
{
  if(insetup == 0)
    {
      panel_set(local_choice,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(scroll_choice,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(screen_choice,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(cursor_choice,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(xon_choice,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(ansi_choice,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(wrap_choice,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(newline_choice,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(answerback_text,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(baud_rate,PANEL_SHOW_ITEM,TRUE,0);
      insetup = 1;
    }
  else
    {
      panel_set(local_choice,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(scroll_choice,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(screen_choice,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(cursor_choice,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(xon_choice,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(ansi_choice,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(wrap_choice,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(newline_choice,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(answerback_text,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(baud_rate,PANEL_SHOW_ITEM,FALSE,0);
      strcpy(answer_message,(char*) panel_get_value(answerback_text));
      insetup = 0;
    }
}

/* Toggle the appearance of the panel keyboard */

static
keyboard_proc(item,event)
     Panel_item item;
     struct inputevent *event;
{
  if(inkeyboard == 0)
    {
      panel_set(PF1_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(PF2_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(PF3_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set(PF4_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R7_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R8_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R9_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( minus_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R4_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R5_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R6_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( comma_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R1_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R2_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R3_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( R0_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( dot_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( enter_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( up_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( down_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( right_button,PANEL_SHOW_ITEM,TRUE,0);
      panel_set( left_button,PANEL_SHOW_ITEM,TRUE,0);
      inkeyboard = 1;
    }
  else
    {
      panel_set(PF1_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(PF2_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(PF3_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set(PF4_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R7_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R8_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R9_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( minus_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R4_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R5_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R6_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( comma_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R1_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R2_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R3_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( R0_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( dot_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( enter_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( up_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( down_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( right_button,PANEL_SHOW_ITEM,FALSE,0);
      panel_set( left_button,PANEL_SHOW_ITEM,FALSE,0);
      inkeyboard = 0;
    }
}

/* What to do when a button is clicked */
/* Since the meaning of the keys is a function of variables set in
   the vt100 environment, this routine uses the translator which
   is used to interpret real sun-2 keyboard actions: translate_key()
*/

static
button_proc(item, event)
     Panel_item item;
     struct inputevent event;
{
  int button_pushed = 0;
  extern  struct ttysubwindow  *_ttysw;
  char * keystr;
  button_pushed = (int) panel_get(item, PANEL_CLIENT_DATA);
  switch (button_pushed)
    {
    case 11:
      keystr = translate_key(KEY_RIGHT(1));
      break;
    case 12:
      keystr = translate_key(KEY_RIGHT(2));
      break;
    case 13:
      keystr = translate_key(KEY_RIGHT(3));
      break;
    case 7:
      keystr = translate_key(KEY_RIGHT(4));
      break;
    case 8:
      keystr = translate_key(KEY_RIGHT(5));
      break;
    case 9:
      keystr = translate_key(KEY_RIGHT(6));
      break;
    case 4:
      keystr = translate_key(KEY_RIGHT(7));
      break;
    case 5:
      keystr = translate_key(KEY_RIGHT(8));
      break;
    case 6:
      keystr = translate_key(KEY_RIGHT(9));
      break;
    case 1:
      keystr = translate_key(KEY_RIGHT(10));
      break;
    case 2:
      keystr = translate_key(KEY_RIGHT(11));
      break;
    case 3:
      keystr = translate_key(KEY_RIGHT(12));
      break;
    case 10:
      keystr = translate_key(KEY_RIGHT(13));
      break;
    case 18:
      keystr = translate_key(KEY_RIGHT(14));
      break;
    case 17:
      keystr = translate_key(KEY_RIGHT(15));
      break;
    case 19:
      keystr = translate_key(KEY_TOP(3));
      break;
    case 21:
      keystr = translate_key(KEY_TOP(4));
      break;
    case 22:
      keystr = translate_key(KEY_TOP(5));
      break;
    case 20:
      keystr = translate_key(KEY_TOP(6));
      break;
    case 14:
      keystr = translate_key(KEY_TOP(7));
      break;
    case 15:
      keystr = translate_key(KEY_TOP(8));
      break;
    case 16:
      keystr = translate_key(KEY_TOP(9));
      break;
    default:
      keystr = "";
   }
  if(local)
    ttysw_output(_ttysw,keystr,strlen(keystr));
  else
    {
      write(_ttysw->ttysw_pty,keystr, strlen(keystr));
      /* Sorry about this hack */
      /* We have to fake out the ttysw_input routine */
      /* since it only queues up the input till the next */
      /* real input event. We will actually write to the */
      /* pty and not put the text into the input queue.*/
    }

}



static
local_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
  local = value;
}

static
scroll_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
}
static
screen_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
/* Never could get this to work right!

  (value == 0) ? pw_whiteonblack(tool->tl_pixwin, 0, 1)
    :pw_blackonwhite(tool->tl_pixwin,0 , 1);
  (value == 0) ? rev_screen = 0:1;
  pw_writebackground(tool->tl_pixwin, 0, 0,
		     winwidthp, winheightp, PIX_NOT(PIX_DST));
*/
}


static
cursor_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
}
static
xon_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
}
static
ansi_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
}
static
wrap_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
  wrap = value;
}
static
newline_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
  newline = value;
}
static
size_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
  big_screen = value;
}

static
baud_rate_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
}

static
label_setup()
{
  keys[(int)panel_get(PF1_button,PANEL_CLIENT_DATA)].button_handle = PF1_button;
  keys[(int)panel_get(PF2_button,PANEL_CLIENT_DATA)].button_handle = PF2_button;
  keys[(int)panel_get(PF3_button,PANEL_CLIENT_DATA)].button_handle = PF3_button;
  keys[(int)panel_get(PF4_button,PANEL_CLIENT_DATA)].button_handle = PF4_button;
   keys[(int)panel_get(R7_button,PANEL_CLIENT_DATA)].button_handle = R7_button;
   keys[(int)panel_get(R8_button,PANEL_CLIENT_DATA)].button_handle = R8_button;
   keys[(int)panel_get(R9_button,PANEL_CLIENT_DATA)].button_handle = R9_button;
   keys[(int)panel_get(minus_button,PANEL_CLIENT_DATA)].button_handle = minus_button;
   keys[(int)panel_get(R4_button,PANEL_CLIENT_DATA)].button_handle = R4_button;
   keys[(int)panel_get(R5_button,PANEL_CLIENT_DATA)].button_handle = R5_button;
   keys[(int)panel_get(R6_button,PANEL_CLIENT_DATA)].button_handle = R6_button;
   keys[(int)panel_get(comma_button,PANEL_CLIENT_DATA)].button_handle = comma_button;
   keys[(int)panel_get(R1_button,PANEL_CLIENT_DATA)].button_handle = R1_button;
   keys[(int)panel_get(R2_button,PANEL_CLIENT_DATA)].button_handle = R2_button;
   keys[(int)panel_get(R3_button,PANEL_CLIENT_DATA)].button_handle = R3_button;
   keys[(int)panel_get(R0_button,PANEL_CLIENT_DATA)].button_handle = R0_button;
   keys[(int)panel_get(dot_button,PANEL_CLIENT_DATA)].button_handle = dot_button;
   keys[(int)panel_get(enter_button,PANEL_CLIENT_DATA)].button_handle = enter_button;
   keys[(int)panel_get(up_button,PANEL_CLIENT_DATA)].button_handle = up_button;
   keys[(int)panel_get(down_button,PANEL_CLIENT_DATA)].button_handle = down_button;
   keys[(int)panel_get(right_button,PANEL_CLIENT_DATA)].button_handle = right_button;
   keys[(int)panel_get(left_button,PANEL_CLIENT_DATA)].button_handle = left_button;
}

static
label_proc(item,value,event)
     Panel_item item;
     int value;
     struct inputevent event;
{
  int i;
  switch (value){
  case 0:
    for(i = 1; i <= 22; i++)
      panel_set(keys[i].button_handle,
		PANEL_LABEL_IMAGE, panel_button_image(keypad,
						       keys[i].norm_label,
						       (i == 10)?13:5,
						       NULL),
		0);
    break;
  case 2:
    for(i = 1; i <= 22; i++)
      panel_set(keys[i].button_handle,
		PANEL_LABEL_IMAGE,panel_button_image(keypad,
						      keys[i].mred_label,
						      (i == 10)?13:5,
						      NULL),
		0);
    break;
  case 1:
    for(i = 1; i <= 22; i++)
      panel_set(keys[i].button_handle,
		PANEL_LABEL_IMAGE, panel_button_image(keypad,
						       keys[i].ibm_label,
						       (i ==10)?13 : 5,
						       NULL),
		0);
    break;
  case 3:
    for(i = 1; i <= 22; i++)
      panel_set(keys[i].button_handle,
		PANEL_LABEL_IMAGE, panel_button_image(keypad,
						       keys[i].wd11_label,
						       (i ==10)?13 : 5,
						       NULL),
		0);
    break;
  case 4:
    for(i = 1; i <= 22; i++)
      panel_set(keys[i].button_handle,
		PANEL_LABEL_IMAGE, panel_button_image(keypad,
						       keys[i].emacs_label,
						       (i ==10)?13 : 5,
						       NULL),
		0);
    break;
    
  }
}

static
norm_proc(item, value,event)
     Panel_item item;
     int value;
     struct inputevent event;

{}
