/**********
 *
 *
 *	 @@@   @@@@@  @   @  @@@@@  @      @       @@@
 *	@   @    @    @   @  @      @      @      @   @
 *	@   @    @    @@@@@  @@@    @      @      @   @
 *	@   @    @    @   @  @      @      @      @   @
 *	 @@@     @    @   @  @@@@@  @@@@@  @@@@@   @@@
 *
 *	OTHELLO - graphics front end to othello game
 *
 *
 *
 *	Edward A. Falk
 *	Sun Microsystems
 *
 *	November, 1986
 *
 *
 *
 **********/


/* compilation parameters: */


#define	CELL_WIDTH	40
#define	PIECE_MARGIN	5
#define	PIECE_RAD	(CELL_WIDTH/2 - PIECE_MARGIN)

#ifdef sun3
#define	MOVE_DELTA	5	/* for animation */
#else
#define	MOVE_DELTA	10
#endif



#include <stdio.h>
#include <suntool/sunview.h>
#include <suntool/panel.h>
#include <suntool/canvas.h>
#include <math.h>
#include <strings.h>
 
#include "othello.h"

#define	TOTAL_WIDTH	BOARDSIZE*CELL_WIDTH

/****
 *
 * Constants,  typedefs, externals, globals, statics, macros
 *
 ****/




#ifndef min
#define	min(x,y)	((x)<(y)?(x):(y))
#define max(x,y)	((x)>(y)?(x):(y))
#endif

#define abs(x)		((x)<0?-(x):(x))

#define	eat_line	while(getchar()!='\n')


#define panel_msg(s)	panel_set(panel_mes, PANEL_LABEL_STRING, s, 0)
#define score_msg(s)	panel_set(score_mes, PANEL_LABEL_STRING, s, 0)



	/* main icon */

static short  icon_image[] = {
#include "othello.icon"
};
DEFINE_ICON_FROM_IMAGE (othello_icon, icon_image) ;


static short	hglass_image[] = {
#include <images/hglass.cursor>
} ;
mpr_static(hglass_pr, 16, 16, 1, hglass_image) ;



	/* assorted globals */



static	struct pixfont	*font ;

static	Frame base_frame, help_frame ;

static	Panel	panel ;

static	Canvas	canvas ;

static	Panel_item	quit_but ;
static	Panel_item	suggest_but ;
static	Panel_item	auto_but ;
static	Panel_item	new_game_but ;
static	Panel_item	computer_first_but ;
static	Panel_item	panel_mes ;
static	Panel_item	score_mes ;
static	Panel_item	difficulty_choice ;


static	Cursor	canvas_cursor, hglass_cursor ;


static	Pixwin	*canvas_pw ;



static	enum	{player_start, player_moving, computer_thinking,
		 computer_moving, game_over} canvas_mode ;

static	int	piece_x, piece_y ;	/* current position of moving piece */
static	int	b_dx, b_dy, bres_ctr ;	/* bresenham counters */

static	struct	COORDINATES playermove, computermove;
static	int	cx, cy ;

static	struct BOARD	oldboard ;

static	char	line[40] ;



	/* proc definitions */

static	void	canvas_proc() ;

static	void	quit_proc() ;
static	void	new_game_proc() ;
static	void	computer_first_proc() ;
static	void	suggest_proc() ;
static	void	auto_proc() ;
static	void	difficulty_proc() ;


static	void	init_panel() ;
static	void	canvas_init() ;
extern	void	init_gamelog() ;
extern	void	initlists() ;
extern	void	initboard() ;
extern	void	erase_board() ;



main(argc,argv)
int	argc ;
char	*argv[] ;
{
	font = pw_pfsysopen() ;


	base_frame = window_create(NULL, FRAME,
	    FRAME_ICON,		&othello_icon,
	    FRAME_LABEL,	"Othello - written by Jonathan Dubman",
	    WIN_ERROR_MSG,	"can't create window.",
	    FRAME_ARGS,		argc, argv,
	    0) ;

	panel = window_create(base_frame, PANEL,
	    WIN_MENU, window_get(base_frame, WIN_MENU),
	    WIN_WIDTH, TOTAL_WIDTH+1,
	    0 ) ;

	init_panel() ;
	window_fit_height(panel) ;

	canvas = window_create(base_frame, CANVAS,
	    WIN_X, 0,
	    WIN_BELOW, panel,
	    WIN_CONSUME_KBD_EVENT, WIN_NO_EVENTS,
	    WIN_EVENT_PROC, canvas_proc,
	    WIN_HEIGHT, TOTAL_WIDTH+1,
	    WIN_WIDTH, TOTAL_WIDTH+1,
	    WIN_IGNORE_PICK_EVENTS, KBD_REQUEST, WIN_ASCII_EVENTS, 0,
	    WIN_MENU, window_get(base_frame, WIN_MENU),
	    CANVAS_AUTO_EXPAND, FALSE,
	    CANVAS_AUTO_SHRINK, FALSE,
	0 ) ;

	window_fit(base_frame) ;

	canvas_pw = (Pixwin *) window_get(canvas, CANVAS_PIXWIN) ;


	canvas_cursor = window_get(canvas, WIN_CURSOR) ;
	hglass_cursor = cursor_create( CURSOR_IMAGE, &hglass_pr, 0 ) ;


	/* game initialization */

	init_gamelog() ;
	initlists() ;
	initboard(&gameboard) ;
	erase_board(&oldboard) ;

	mistakes = 0 ;
	updown = 0 ;  leftright = 0 ;
	computer = FALSE ;
	turns = 0 ;
	firstmove = TRUE ;
	lookahead = 0 ;

	canvas_mode = player_start ;

	canvas_init() ;
	panel_msg("to move, use your left mouse button") ;

	window_main_loop(base_frame) ;
	exit(0) ;
}



static void
init_panel()
{

	quit_but = panel_create_item( panel, PANEL_BUTTON,
	    PANEL_NOTIFY_PROC, quit_proc,
	    PANEL_LABEL_IMAGE, panel_button_image(panel, "quit", 4, 0),
	  0 ) ;

	new_game_but = panel_create_item( panel, PANEL_BUTTON,
	    PANEL_NOTIFY_PROC, new_game_proc,
	    PANEL_LABEL_IMAGE, panel_button_image(panel, "new game", 8, 0),
	  0 ) ;

	computer_first_but = panel_create_item( panel, PANEL_BUTTON,
	    PANEL_NOTIFY_PROC, computer_first_proc,
	    PANEL_LABEL_IMAGE,
		panel_button_image(panel, "Computer starts", 15, 0),
	  0 ) ;

	suggest_but = panel_create_item( panel, PANEL_BUTTON,
	    PANEL_NOTIFY_PROC, suggest_proc,
	    PANEL_LABEL_IMAGE, panel_button_image(panel, "suggest", 7, 0),
	  0 ) ;

	auto_but = panel_create_item( panel, PANEL_BUTTON,
	    PANEL_NOTIFY_PROC, auto_proc,
	    PANEL_LABEL_IMAGE, panel_button_image(panel, "auto", 4, 0),
	  0 ) ;

	difficulty_choice = panel_create_item( panel, PANEL_CHOICE,
	    PANEL_NOTIFY_PROC, difficulty_proc,
	    PANEL_LABEL_BOLD, TRUE,
	    PANEL_LAYOUT, PANEL_HORIZONTAL,
	    PANEL_LABEL_STRING, "Difficulty: ",
	    PANEL_VALUE, 0,
	    PANEL_SHOW_MENU, TRUE,
	    PANEL_CHOICE_STRINGS, "0","1","2","3","4","5","6","7","8","9",0,
	    PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
	  0 ) ;

	panel_mes = panel_create_item( panel, PANEL_MESSAGE,
	    PANEL_LABEL_STRING, "this is a test                ",
	  0 ) ;

	score_mes = panel_create_item( panel, PANEL_MESSAGE,
	    PANEL_LABEL_STRING, "player:  2, computer:  2",
	  0 ) ;
}


static void
quit_proc(item,event)
Panel_item	item ;
Event		*event ;
{
	window_done(base_frame) ;
}




static void
suggest_proc()
{
	struct	COORDINATES suggestion;
	int	x,y ;

	suggestion = scanboard(&gameboard, X, beststrategy, lookahead);
	x = suggestion.xc*CELL_WIDTH - CELL_WIDTH/2 ;
	y = suggestion.yc*CELL_WIDTH - CELL_WIDTH/2 ;
	pw_vector(canvas_pw, x-5,y-5, x+5,y+5, PIX_SRC, 0xff) ;
	pw_vector(canvas_pw, x-5,y+5, x+5,y-5, PIX_SRC, 0xff) ;
}
	



static void
difficulty_proc(item, value, event)
Panel_item	item ;
int		value ;
Event		*event ;
{
	lookahead = value ;
}


#define CIRCLE_VECS	24

static	struct pr_pos	circle_vlist[CIRCLE_VECS] ;

static void
init_circle()
{
static	float	ctab[24][2] = {
	{ 0.966, 0.259}, { 0.866, 0.500}, { 0.707, 0.707}, { 0.500, 0.866}, 
	{ 0.259, 0.966}, { 0.000, 1.000}, {-0.259, 0.966}, {-0.500, 0.866}, 
	{-0.707, 0.707}, {-0.866, 0.500}, {-0.966, 0.259}, {-1.000, 0.000}, 
	{-0.966,-0.259}, {-0.866,-0.500}, {-0.707,-0.707}, {-0.500,-0.866}, 
	{-0.259,-0.966}, { 0.000,-1.000}, { 0.259,-0.966}, { 0.500,-0.866}, 
	{ 0.707,-0.707}, { 0.866,-0.500}, { 0.966,-0.259}, { 1.000, 0.000}, } ;

	int	i ;
	float	x1,y1 ;

	for(i=0; i<CIRCLE_VECS;++i)
	{
	  x1 = (PIECE_RAD+.5) * ctab[i][0] + PIECE_RAD;
	  y1 = (PIECE_RAD+.5) * ctab[i][1] + PIECE_RAD;
	  circle_vlist[i].x = x1 ;
	  circle_vlist[i].y = y1 ;
	}
}


static void
draw_cute_circle(x,y,rop)
	int	x,y ;
	int	rop ;
{
	int	i ;
	int	x0,y0, x1,y1 ;

	pw_batch_on(canvas_pw) ;
	x0 = x + circle_vlist[CIRCLE_VECS-1].x ;
	y0 = y + circle_vlist[CIRCLE_VECS-1].y ;
	for(i=0; i<CIRCLE_VECS;i++)
	{
	  x1 = x + circle_vlist[i].x ;
	  y1 = y + circle_vlist[i].y ;
	  pw_vector(canvas_pw, x0,y0, x1,y1, rop, 0xff) ;
	  x0 = x1; y0 = y1 ;
	}
	pw_batch_off(canvas_pw) ;
}




static void
fill_cute_circle(x,y,rop)
	int	x,y ;
	int	rop ;
{
static	int	npts[] = {CIRCLE_VECS} ;

	pw_batch_on(canvas_pw) ;
	pw_polygon_2(canvas_pw,x,y, 1, npts, circle_vlist,
	  PIX_COLOR(0xff)|rop, NULL,0,0) ;
	pw_batch_off(canvas_pw) ;

}



static void
update_board_image()
{
	int	x, y ;
	int	x0, y0 ;

	int	xcount,ocount ;

	for(y=1; y<=BOARDSIZE; y++)
	  for(x=1; x<=BOARDSIZE; x++)
	    if( gameboard.loc[x][y] != oldboard.loc[x][y] )
	    {
	      x0 = (x-1)*CELL_WIDTH + PIECE_MARGIN ;
	      y0 = (y-1)*CELL_WIDTH + PIECE_MARGIN ;
	      switch(oldboard.loc[x][y])
	      {
		case EMPTY:
		case X:
		  switch(gameboard.loc[x][y])
		  {
		    case O:
		      fill_cute_circle(x0, y0, PIX_SRC) ;
		      break ;
		    case X:
		      draw_cute_circle(x0, y0, PIX_SRC) ;
		      break ;
		  }
		  break ;
		case O:
		  switch(gameboard.loc[x][y])
		  {
		    case X:
		      fill_cute_circle(x0, y0, PIX_CLR) ;
		      draw_cute_circle(x0, y0, PIX_SRC) ;
		      break ;
		  }
		  break ;
	      }
	      oldboard.loc[x][y] = gameboard.loc[x][y] ;
	    }


	xcount = count(&gameboard, X);
	ocount = count(&gameboard, O);
	score_msg( sprintf(line,"player: %2d, computer: %2d",xcount,ocount)) ;

}



static void
canvas_init()
{
	int	i, j ;
	int	x0,y0  ;

	init_circle() ;

	pw_writebackground(canvas_pw, 0,0,TOTAL_WIDTH+1,TOTAL_WIDTH+1,PIX_SRC);

	pw_batch_on(canvas_pw) ;
	for(x0=0; x0<=TOTAL_WIDTH; x0+=CELL_WIDTH)
	{
	  pw_vector(canvas_pw, x0,0, x0,TOTAL_WIDTH, PIX_SRC, 0xff) ;
	  pw_vector(canvas_pw, 0,x0, TOTAL_WIDTH,x0, PIX_SRC, 0xff) ;
	}

	pw_batch_off(canvas_pw) ;

	update_board_image() ;
}



static void
animate_move(cx,cy)
	int	cx,cy ;
{
	int	x0,y0, x1,y1 ;
	int	x,y, dx,dy, ctr ;
static	struct rect	r = {0,0,TOTAL_WIDTH+1, TOTAL_WIDTH+1};

	pw_lock(canvas_pw, &r) ;
	x1 = (cx-1)*CELL_WIDTH + PIECE_MARGIN ;
	y1 = (cy-1)*CELL_WIDTH + PIECE_MARGIN ;
	dx = x1 ;
	dy = y1 ;
	if(x1>y1)
	{
	  ctr = dx/2 ;
	  x=0 ;
	  y=0 ;
	  draw_cute_circle(x,y,PIX_NOT(PIX_DST)) ;
	  while(x<x1)
	  {
	    x0 = x ;
	    y0 = y ;
	    x += MOVE_DELTA ;
	    if((ctr -= dy)<0)
	    {
	      ctr += dx ;
	      y += MOVE_DELTA ; 
	    }
	    draw_cute_circle(x,y,PIX_NOT(PIX_DST)) ;
	    draw_cute_circle(x0,y0,PIX_NOT(PIX_DST)) ;
	  }
	  draw_cute_circle(x,y,PIX_NOT(PIX_DST)) ;
	}
	else
	{
	  ctr = dy/2 ;
	  x=0 ;
	  y=0 ;
	  draw_cute_circle(x,y,PIX_NOT(PIX_DST)) ;
	  while(y<y1)
	  {
	    x0 = x ;
	    y0 = y ;
	    y += MOVE_DELTA ;
	    if((ctr -= dx)<0)
	    {
	      ctr += dy ;
	      x += MOVE_DELTA ; 
	    }
	    draw_cute_circle(x,y,PIX_NOT(PIX_DST)) ;
	    draw_cute_circle(x0,y0,PIX_NOT(PIX_DST)) ;
	  }
	  draw_cute_circle(x,y,PIX_NOT(PIX_DST)) ;
	}
	pw_unlock(canvas_pw) ;
}




static void
who_wins()
{
	int	cs, ps ;

	ps = count(&gameboard, X);
	cs = count(&gameboard, O);
	if(ps>cs)
	  score_msg(sprintf(line,"You win %d-%d",ps,cs)) ;
	else if(ps == cs)
	  score_msg(sprintf(line,"A tie %d-%d",ps,cs)) ;
	else
	  score_msg(sprintf(line,"Computer wins %d-%d",cs,ps));
}


static void
player_moved()
{
	playermove.xc = cx ;
	playermove.yc = cy ;
	formfliplist(&gameboard, cx, cy, X);
	gameboard.loc[cx][cy] = X;
	do_flip(&gameboard, listcount);
	zap_list();
	update_gamelog(X, cx, cy);
	update_board_image() ;
	if (gamelog[turns].take == 1)
	  panel_msg("you took 1 piece") ;
	else
	  panel_msg(sprintf(line, "you took %d pieces",gamelog[turns].take));
}



static void
computer_think()
{
	window_set(canvas, WIN_CURSOR, hglass_cursor, 0) ;
	computermove = scanboard(&gameboard, O, beststrategy, lookahead);
	cx = computermove.xc;
	cy = computermove.yc;
	if (validmove(&gameboard, cx, cy, O) == FALSE)
	{
	  panel_msg("Computer is forced to pass.") ;
	  update_gamelog(O, PASS);
	  who_wins() ;
	  canvas_mode = game_over ;
	}
	else
	{
	  formfliplist(&gameboard, cx, cy, O);
	  gameboard.loc[cx][cy] = O;
	  do_flip(&gameboard, listcount);
	  zap_list();
	  update_gamelog(O, cx, cy);
	  animate_move(cx,cy) ;
	  update_board_image() ;
	  if (gamelog[turns].take == 1)
	    panel_msg("Computer took 1 piece") ;
	  else
	    panel_msg(sprintf
	      (line, "Computer took %d pieces",gamelog[turns].take));
	  canvas_mode = player_start ;
	}
	window_set(canvas, WIN_CURSOR, canvas_cursor, 0 ) ;
}



static void
player_think()
{
	playermove = scanboard(&gameboard, X, mostpieces, 0);
	cx = playermove.xc;
	cy = playermove.yc;
	if (validmove(&gameboard, cx, cy, X) == FALSE)
	{
	  panel_msg("you are forced to pass");
	  who_wins() ;
	  update_gamelog(X, PASS);
	  canvas_mode = game_over ;
	}
	else
	{
	  if( computer == FALSE )
	  {
	    canvas_mode = player_start ;
	  }
	  else
	  {
	    window_set(canvas, WIN_CURSOR, hglass_cursor, 0) ;
	    playermove = scanboard(&gameboard, X, beststrategy, lookahead);
	    cx = playermove.xc;
	    cy = playermove.yc;
	    animate_move(cx,cy) ;
	    player_moved() ;
	    canvas_mode = computer_thinking ;
	    window_set(canvas, WIN_CURSOR, canvas_cursor, 0 ) ;
	  }
	}
}



static void
canvas_proc(win,event,arg)
	Canvas	win ;
	Event	*event ;
	caddr_t	arg ;
{
	int	x0,y0 ;


	switch(canvas_mode)
	{
	  case player_start:
	    switch(event_id(event))
	    {
	      case MS_LEFT:
		if(event_is_down(event))
		{
		  piece_x = event_x(event) - PIECE_RAD ;
		  piece_y = event_y(event) - PIECE_RAD ;
		  draw_cute_circle(piece_x, piece_y, PIX_NOT(PIX_DST)) ;
		  canvas_mode = player_moving ;
		}
		break ;
	    }			/* ignore all others */
	    break ;

	  case player_moving:
	    switch(event_id(event))
	    {
	      case LOC_MOVE:
	      case LOC_DRAG:
	      case LOC_TRAJECTORY:
		draw_cute_circle(piece_x, piece_y,PIX_NOT(PIX_DST)) ;
		piece_x = event_x(event) - PIECE_RAD ;
		piece_y = event_y(event) - PIECE_RAD ;
		draw_cute_circle(piece_x, piece_y,PIX_NOT(PIX_DST)) ;
		break ;

	      case LOC_WINENTER:
	      case LOC_WINEXIT:
	      case LOC_RGNENTER:
	      case LOC_RGNEXIT:
	      case WIN_STOP:
		draw_cute_circle(piece_x, piece_y,PIX_NOT(PIX_DST)) ;
		canvas_mode = player_start ;
		break ;

	      case MS_LEFT:
		if(event_is_down(event))
		{
		  draw_cute_circle(piece_x, piece_y,PIX_NOT(PIX_DST)) ;
		  canvas_mode = player_start ;
		}
		else
		{
		  draw_cute_circle(piece_x, piece_y,PIX_NOT(PIX_DST)) ;

		  /* this replaces the getmove subroutine */
		  cx = (piece_x+PIECE_RAD)/CELL_WIDTH + 1 ;
		  cy = (piece_y+PIECE_RAD)/CELL_WIDTH + 1 ;
		  if(validmove(&gameboard, cx, cy, X) == FALSE)
		  {
		    panel_msg("invalid move");
		    canvas_mode = player_start ;
		  }
		  else
		  {
		    player_moved() ;
		    canvas_mode = computer_thinking ;
		    while(canvas_mode == computer_thinking)
		    {
		      computer_think() ;
		      if(canvas_mode == player_start)
			player_think() ;
		    }
		  }
		}
		break ;
	    }			/* ignore all others */
	    break ;



	  case game_over:
	    switch(event_id(event))
	    {
	      case MS_LEFT:
	      case MS_MIDDLE:
	      case MS_RIGHT:
		if(event_is_down(event))
		{
		  panel_msg("Game over");
		}
		break ;
	    }			/* ignore all others */
	    break ;
	}
}



static void
computer_first_proc()
{
	if(turns == 0)
	{
	  canvas_mode = computer_thinking ;
	  while(canvas_mode == computer_thinking)
	  {
	    computer_think() ;
	    if(canvas_mode == player_start)
	      player_think() ;
	  }
	}
}



static void
auto_proc()
{
	if(canvas_mode == game_over)
	  panel_msg("Game over");
	else
	{
	  computer=TRUE ;
	  canvas_mode = player_start ;
	  while(canvas_mode == player_start)
	  {
	    player_think() ;
	    if(canvas_mode == computer_thinking)
	      computer_think() ;
	  }
	}
}


static void
new_game_proc()
{
	/* game initialization */

	init_gamelog() ;
	initlists() ;
	initboard(&gameboard) ;
	erase_board(&oldboard) ;

	firstmove = TRUE ;
	computer = FALSE ;

	canvas_mode = player_start ;

	canvas_init() ;
	panel_msg("to move, use your left mouse button") ;
}
