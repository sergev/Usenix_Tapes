
/*  calctool.c
 *
 *  This program looks and acts just like a simple desktop calculator.
 *  Copyright (c) Rich Burridge - Sun Australia - October 1986.
 *
 *  Version 1.0
 *
 *  No responsibility is taken for any errors or inaccuracies inherent
 *  either to the comments or the code of this program, but if
 *  reported to me then an attempt will be made to fix them.
 */

#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <suntool/sunview.h>
#include <suntool/panel.h>

#define  SPRINTF        (void) sprintf   /* To make lint happy. */
#define  SSCANF         (void) sscanf
#define  STRCPY         (void) strcpy
#define  STRNCAT        (void) strncat

#define  UNDEFINED       0               /* No current arithmetic operation. */
#define  ADDITION        1
#define  SUBTRACTION     2
#define  MULTIPLICATION  3
#define  DIVISION        4
#define  SQUARE_ROOT     5

#define  BUTTON_BORDER   5               /* No of pixels in border. */
#define  BUTTON_COLS     4               /* No of columns of buttons. */
#define  BUTTON_GAP      5               /* No of pixels between buttons. */
#define  BUTTON_HEIGHT   25              /* Number of pixels for height. */
#define  BUTTON_ROWS     5               /* No of rows of buttons. */
#define  BUTTON_WIDTH    40              /* No of pixels for width. */
#define  DISPLAY         30              /* Calculators numerical display. */
#define  DISP_LENGTH     12              /* Length of display in characters. */
#define  MAXLINE         30              /* Length of character strings. */
#define  NOBUTTONS       BUTTON_ROWS * BUTTON_COLS

char *sprintf() ;
void button_press() ;
Frame frame ;
Panel panel1,panel2 ;
Panel_item display_item ;

short icon_image[] = {
#include "calctool.icon"
} ;
DEFINE_ICON_FROM_IMAGE(calctool_icon,icon_image) ;

double old_display ;                /* Previous calculator display value. */
double last_input ;                 /* Previous number input by user. */

int column,row ;
int error ;                         /* Indicates some kind of display error. */
int operator = UNDEFINED ;          /* Current arithmetic operation. */
int pointed ;                       /* Whether a decimal point has been given. */
int started ;                       /* Indicates if number should be displayed. */
int toclear ;                       /* Indicates if display should be cleared. */

char bpress ;                       /* Current button pressed. */
char oldbpress ;                    /* Previous button pressed. */
char button_vals[NOBUTTONS][4] =    /* Calculator button values. */
       {
         "END", " C ", "SRT", "OFF",
         " 7 ", " 8 ", " 9 ", " X ",
         " 4 ", " 5 ", " 6 ", " / ",
         " 1 ", " 2 ", " 3 ", " - ",
         " 0 ", " . ", " = ", " + "
       } ;
char display[MAXLINE] ;             /* Current calculator display. */


main(argc,argv)
int argc ;
char *argv[] ;

{
  frame = window_create(0,FRAME,
                        FRAME_ICON, &calctool_icon,
                        FRAME_SHOW_LABEL, FALSE,
                        FRAME_SUBWINDOWS_ADJUSTABLE, FALSE,
                        FRAME_NO_CONFIRM, TRUE,
                        WIN_TOP_MARGIN, DISPLAY,
                        WIN_ROW_HEIGHT, BUTTON_HEIGHT,
                        WIN_COLUMN_WIDTH, BUTTON_WIDTH,
                        WIN_ROWS, BUTTON_ROWS,
                        WIN_COLUMNS, BUTTON_COLS,
                        FRAME_ARGS, argc,argv,
                        0) ;

  panel1 = window_create(frame,PANEL,WIN_HEIGHT,DISPLAY,0) ;
  panel2 = window_create(frame,PANEL,WIN_BELOW,panel1,0) ;
  display_item = panel_create_item(panel1,PANEL_MESSAGE,
                                   PANEL_PAINT, PANEL_NO_CLEAR,
                                   0) ;
  initialise() ;              /* Initialise certain variables. */
  clear_display() ;
  for (row = 0; row < BUTTON_ROWS; row++)
    for (column = 0; column < BUTTON_COLS; column++)
      panel_create_item(panel2,PANEL_BUTTON,
         PANEL_LABEL_X,column*BUTTON_WIDTH+BUTTON_BORDER+(column*BUTTON_GAP),
         PANEL_LABEL_Y,row*BUTTON_HEIGHT+BUTTON_BORDER+(row*BUTTON_GAP),
         PANEL_LABEL_IMAGE,
           panel_button_image(panel2,button_vals[row*BUTTON_COLS+column],0,0),
         PANEL_NOTIFY_PROC,button_press,0) ;

  window_fit(panel2) ;
  window_fit(frame) ;
  window_main_loop(frame) ;
  exit(0) ;
}


clear_display()

{
  started = 0 ;
  pointed = 0 ;
  toclear = 0 ;
  STRCPY(display,"0.") ;
  make_display() ;
}


do_calculation()      /* Perform arithmetic calculation and display result. */

{
  if (bpress == 'R')
    {
      SSCANF(display,"%lf",&last_input) ;
      operator = SQUARE_ROOT ;
    }
  else if (bpress != '=')
    {
      if (oldbpress == '+' || oldbpress == '-' || oldbpress == 'X' ||
          oldbpress == '/' || oldbpress == '=')
        {
          operator = bpress ;
          return ;
        }
      SSCANF(display,"%lf",&last_input) ;
    }
  else if (oldbpress != '=' && oldbpress != 'R') SSCANF(display,"%lf",&last_input) ;

  switch (operator)
    {
      case UNDEFINED      : break ;
      case ADDITION       : SPRINTF(display,"%.10f",old_display + last_input) ;
                            break ;
      case SUBTRACTION    : SPRINTF(display,"%.10f",old_display - last_input) ;
                            break ;
      case MULTIPLICATION : SPRINTF(display,"%.10f",old_display * last_input) ;
                            break ;
      case DIVISION       : SPRINTF(display,"%.10f",old_display / last_input) ;
                            break ;
      case SQUARE_ROOT    : SPRINTF(display,"%.10f",sqrt(last_input)) ;
    }
  make_result() ;
  toclear = 1 ;
  SSCANF(display,"%lf",&old_display) ;
}


initialise()

{
  error = 0 ;                 /* Currently no display error. */
  oldbpress = '\0' ;          /* No previous button pressed. */
  operator = UNDEFINED ;      /* No arithmetic operator defined yet. */
  old_display = 0.0 ;         /* No previous value defined yet. */
}


make_display()      /* Build up the calculators display - right justified. */

{
  char output[MAXLINE] ;

  SPRINTF(output,"%20s",display) ;
  panel_set(display_item,PANEL_LABEL_STRING,output,0) ;
}


make_result()       /* Display result of calculation. */

{
  char output[MAXLINE] ;
  int pnt ;

  pnt = strlen(display)-1 ;
  while (display[pnt] != '.' && display[pnt] == '0') display[pnt--] = '\0' ;

  for (pnt = 0; pnt < strlen(display); pnt++)
    if (display[pnt] == '.') break ;

  if (pnt > DISP_LENGTH)
    {
      display[DISP_LENGTH-2] = '\0' ;
      SPRINTF(output,"E%18s.",display) ;
      error = 1 ;
    }
  else if (!strcmp(display,"Infinity"))
    {
      SPRINTF(output,"E%19s","0.") ;
      error = 1 ;
    }
  else if (display[0] == '-') SPRINTF(output,"-%19s",&display[1]) ;
  else SPRINTF(output,"%20s",display) ;
  panel_set(display_item,PANEL_LABEL_STRING,output,0) ;
}


/*ARGSUSED*/
static void button_press(item,event)
Panel_item item ;
Event *event ;

{
  int column,row ;

  column = (event_x(event) - BUTTON_BORDER) / (BUTTON_WIDTH + BUTTON_GAP) ;
  row = (event_y(event) - BUTTON_BORDER) / (BUTTON_HEIGHT + BUTTON_GAP) ;
  bpress = button_vals[row*BUTTON_COLS+column][1] ;

  if (error && bpress != 'C') return ;   /* Only C can clear an error condition. */

  switch (bpress)
    {
      case 'N' : window_destroy(frame) ;              /* Terminate program. */
                 break ;
      case 'F' : window_set(frame,FRAME_CLOSED,TRUE,0) ;  /* Turn window iconic. */
      case 'C' : clear_display() ;
                 operator = UNDEFINED ;
                 old_display = 0.0 ;
                 error = 0 ;
                 break ;
      case 'R' : do_calculation() ;                       /* Square root. */
                 operator = SQUARE_ROOT ;
                 break ;
      case '1' :
      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' : started = 1 ;
      case '0' : if (toclear)
                   {
                     clear_display() ;
                     if (bpress != '0') started = 1 ;
                   }
                 if (strlen(display) < DISP_LENGTH)
                   {
                     if (pointed)
                       {
                         STRNCAT(display,&bpress,1) ;
                         make_display() ;
                       }
                     else if (started)
                       {
                         if (!strcmp(display,"0.")) display[0] = '\0' ;
                         display[strlen(display)-1] = '\0' ;
                         STRNCAT(display,&bpress,1) ;
                         STRNCAT(display,".",1) ;
                         make_display() ;
                       }
                   }
                 break ;

      case '+' : do_calculation() ;
                 operator = ADDITION ;
                 break ;
      case '-' : do_calculation() ;
                 operator = SUBTRACTION ;
                 break ;
      case 'X' : do_calculation() ;
                 operator = MULTIPLICATION ;
                 break ;
      case '/' : do_calculation() ;
                 operator = DIVISION ;
                 break ;

      case '.' : if (strlen(display) < DISP_LENGTH)
                   if (!pointed) pointed = 1 ;
                 break ;

      case '=' : do_calculation() ;
                 break ;
    }
  oldbpress = bpress ;
}
