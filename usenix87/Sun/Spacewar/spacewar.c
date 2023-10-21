/*
 *	Copyright 1987, Jonathan Biggar
 *
 *	Permission is given to copy and distribute for non-profit purposes.
 *
 */

#ifndef lint
static char *rcs = "$header$ Copyright 1987 Jonathan Biggar";
#endif !lint

/*
 * $log$
 */

#include <stdio.h>
#include <sys/types.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <suntool/panel.h>
#include <sunwindow/notify.h>
#include <math.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "spacewar.h"

#define	ITIMER_NULL	((struct itimerval *)0)

void			die();

char			shieldkey = '\r';

Frame			main_frame;
static short		icon_image[] = {
#include "spacewar.icon"
};
DEFINE_ICON_FROM_IMAGE(icon, icon_image);

Canvas			canvas;
Pixwin			*canvas_pw;
Panel			panel;
Panel_item		play_message;
Panel_item		play_yes;
Panel_item		play_no;
Panel_item		cp1;
Panel_item		cp2;
void			play_yes_no();
Panel_item		player_names[MAX_PLAYERS];
Panel_item		player_scores[MAX_PLAYERS];

struct singlecolor	foreground = {255, 255, 255};
struct singlecolor	background = {0, 0, 0};

double			deg_2_rad;
double			sin_tab[360];
double			cos_tab[360];

/* xy_point_list contains the vectors needed to draw a ship and a star */

double			xy_point_list[][2] = {	/*0*/	10.0,	0.0,
						/*1*/	-10.0,	5.0,
						/*2*/	-10.0,	-5.0,
						/*3*/	-10.0,	0.0,
						/*4*/	-15.0,	0.0,
						/*5*/	10.0,	0.0,
						/*6*/	-10.0,	0.0,
						/*7*/	0.0,	10.0,
						/*8*/	0.0,	-10.0,
						/*9*/	8.66,	5.0,
						/*10*/	-8.66,	-5.0,
						/*11*/	5.0,	8.66,
						/*12*/	-5.0,	-8.66,
						/*13*/	-5.0,	8.66,
						/*14*/	5.0,	-8.66,
						/*15*/	-8.66,	5.0,
						/*16*/	8.66,	-5.0,
						/*17*/	20.0,	0.0,
						/*18*/	-15.0,	10.0,
						/*19*/	-15.0,	-10.0,
						/*20*/	10.0, 	0,
						/*21*/	7.0,	-5.0,
						/*22*/	2.0,	-7.0,
						/*23*/	0,	-10.0,
						/*24*/	-10.0,	-7.0,
						/*25*/	-9.0,	6.0,
						/*26*/	2.0,	10.0};
#define PL_SIZE		(sizeof(xy_point_list)/sizeof(double)/2)
double			rt_point_list[PL_SIZE][3];

Controls		controls = { {CONTROLS, 0}, 0, 0, 0, 0, 0, 0};
struct itimerval	control_timer = {0, UPDATE_FREQ, 0, UPDATE_FREQ};
int			control_count = 0;
int			last_ack = 0;
Notify_value		control_send();

int			ship_num = -1;
int			num_objects = 0;
Object			objects[MAX_OBJECTS];
int			die_count[MAX_PLAYERS];

int			sock;
struct sockaddr_in	my_addr;
struct sockaddr_in	serv_addr;

struct {
    struct header	head;
    char		name[NAME_SIZE];
}			hello_head = { HELLO, -1};
struct itimerval	hello_timer = {1, 0, 1, 0};
int			hello_count = 0;
Notify_value		hello();

void			dead();
void			event_proc();
Notify_value		net_input();

char			*getlogin();
struct in_addr		inet_makeaddr();

main(argc, argv)
int argc;
char **argv;
{
    struct servent	*se;
    struct hostent	*he;
    char 		hostname[32];
    register int	i;
    Cursor		cursor;
    char		*name;
    int			rows;
    int			cols;
    struct pixfont	*panel_font;
    struct pixfont	*default_font;

    /* parse arguments */
    for(argc--, argv++; argc > 0; argc--, argv++)
	if (argv[0][0] = '-')
	    switch (argv[0][1]) {
	    case 's':
		if (argv[0][2] == '\\')
		    switch(argv[0][3]) {
		    case 'r':
			shieldkey = '\r';
			break;
		    case 'n':
			shieldkey = '\n';
			break;
		    case 'b':
			shieldkey = '\b';
			break;
		    }
		else
		    shieldkey = argv[0][2];
		break;
	    default:
		die("Usage: spacewar [-s{key}]\n", 0);
		break;
	    }
	else
	    die("Usage: spacewar [-s{key}]\n", 0);
	    
    /* fill in sine and cosine tables for future reference and speed */
    
    deg_2_rad = acos(-1.0)/180.0;
    for (i = 0; i < 360; i++) {
	sin_tab[i] = sin(i*deg_2_rad);
	cos_tab[i] = cos(i*deg_2_rad);
    }
    
    /* pre-calculate angles for ship rotations */
    
    for (i = 0; i < PL_SIZE; i++) {
	
	rt_point_list[i][0] =
	  sqrt((xy_point_list[i][0] * xy_point_list[i][0] +
	  xy_point_list[i][1] * xy_point_list[i][1]));
	rt_point_list[i][1] = xy_point_list[i][1]/rt_point_list[i][0];
	rt_point_list[i][2] = xy_point_list[i][0]/rt_point_list[i][0];
    }

    /* setup my address and server address */
    
    if ((se = getservbyname("spacewar", "udp")) == NULL)
	die("Can't find spacewar service\n", 0);
    gethostname(hostname, sizeof(hostname));
    if ((he = gethostbyname(hostname)) == NULL)
	die("Can't find hostname?\n", 0);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = se->s_port;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	die("Can't create socket:", 1);
    if(bind(sock, &my_addr, sizeof(my_addr)) == -1)
	die("Can't bind socket address:", 1);
    if((se = getservbyname("spaceward", "udp")) == NULL)
	die("Can't find spaceward service\n", 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = se->s_port;
    bcopy(he->h_addr, &serv_addr.sin_addr, he->h_length);
    serv_addr.sin_addr =
      inet_makeaddr(inet_netof(serv_addr.sin_addr),INADDR_ANY);
      
    /* set up my name */
    
    if ((name = getlogin()) != NULL)
	strcpy(hello_head.name, name);
    else
	strcpy(hello_head.name, "unknown");
    strcat(hello_head.name, "@");
    strcat(hello_head.name, hostname);

    /* invert the icon because we are reverse video */
    for (i = 0; i < sizeof(icon_image)/sizeof(short); i++)
	icon_image[i] ^= 0xFFFF;

    /* create main frame */
   
    main_frame = window_create(NULL, FRAME,
			       FRAME_SHOW_LABEL, FALSE,
			       WIN_HEIGHT, S_Y_MAX,
			       WIN_WIDTH, S_X_MAX,
			       WIN_X, 0,
			       WIN_Y, 0,
			       FRAME_FOREGROUND_COLOR, &foreground,
			       FRAME_BACKGROUND_COLOR, &background,
			       FRAME_INHERIT_COLORS, TRUE,
			       FRAME_SUBWINDOWS_ADJUSTABLE, FALSE,
			       FRAME_ICON, &icon,
			       0);

    /* set up canvas for the graphics */
    
    canvas = window_create(main_frame, CANVAS,
			   WIN_HEIGHT, Y_MAX,
			   WIN_WIDTH, X_MAX,
			   CANVAS_RETAINED, FALSE,
			   WIN_EVENT_PROC, event_proc,
			   WIN_CONSUME_KBD_EVENTS,
			       WIN_ASCII_EVENTS,
			       0,
			   WIN_CONSUME_PICK_EVENTS,
			       WIN_MOUSE_BUTTONS,
			       0,
			   0);
    win_set_kbd_focus((int)window_get(canvas, WIN_FD),
      (int)window_get(canvas, WIN_DEVICE_NUMBER));
    canvas_pw = canvas_pixwin(canvas);

    /* set up the panel on the right hand side */
    
    default_font = pf_default();
    panel_font = pf_open("/usr/lib/fonts/fixedwidthfonts/screen.r.7");
    panel = window_create(main_frame, PANEL,
			  PANEL_ACCEPT_KEYSTROKE, TRUE,
			  WIN_NOTIFY_EVENT_PROC, event_proc,
			  WIN_HEIGHT, P_Y_MAX,
			  WIN_WIDTH, P_X_MAX,
			  WIN_RIGHT_OF, canvas,
			  WIN_FONT, panel_font,
			  0);
    rows = (int)window_get(panel, WIN_ROWS);
    /* work around sunview bug */
    cols = (int)window_get(panel, WIN_WIDTH)/
      ((int)window_get(panel, WIN_COLUMN_WIDTH) +
      (int)window_get(panel, WIN_COLUMN_GAP));
    cp1 = panel_create_item(panel, PANEL_MESSAGE,
			     PANEL_LABEL_STRING, "Copyright 1987",
			     PANEL_ITEM_Y, ATTR_ROW(2),
			     PANEL_ITEM_X, ATTR_COL(cols/2-10),
			     PANEL_LABEL_FONT, default_font,
			     0);
    cp2 = panel_create_item(panel, PANEL_MESSAGE,
			     PANEL_LABEL_STRING, "Jonathan Biggar",
			     PANEL_ITEM_Y, ATTR_ROW(4),
			     PANEL_ITEM_X, ATTR_COL(cols/2-11),
			     PANEL_LABEL_FONT, default_font,
			     0);
    play_message = panel_create_item(panel, PANEL_MESSAGE,
				     PANEL_LABEL_STRING, "Play again?",
				     PANEL_ITEM_Y, ATTR_ROW(8),
				     PANEL_ITEM_X, ATTR_COL(cols/2-8),
				     PANEL_SHOW_ITEM, FALSE,
				     PANEL_LABEL_FONT, default_font,
				     0);
    play_yes = panel_create_item(panel, PANEL_BUTTON,
				 PANEL_LABEL_IMAGE,
				   panel_button_image(panel, "YES", 3,
				     default_font),
				 PANEL_ITEM_Y, ATTR_ROW(10),
				 PANEL_ITEM_X, ATTR_COL(cols/2-5),
				 PANEL_NOTIFY_PROC, play_yes_no,
				 PANEL_SHOW_ITEM, FALSE,
				 0);
    play_no = panel_create_item(panel, PANEL_BUTTON,
				PANEL_LABEL_IMAGE,
				  panel_button_image(panel, "NO", 3,
				    default_font),
				PANEL_ITEM_Y, ATTR_ROW(12),
				PANEL_ITEM_X, ATTR_COL(cols/2-5),
				PANEL_NOTIFY_PROC, play_yes_no,
				PANEL_SHOW_ITEM, FALSE,
				0);

    /* set up panel item for each player's name */
    
    for (i = 0; i < MAX_PLAYERS; i++) {
	player_scores[i] = panel_create_item(panel, PANEL_MESSAGE,
					     PANEL_LABEL_STRING, "",
					     PANEL_ITEM_X, ATTR_COL(1),
					     PANEL_ITEM_Y,
					       ATTR_ROW(rows-MAX_PLAYERS+i),
					     0);
	player_names[i] =
	  panel_create_item(panel, PANEL_MESSAGE,
				   PANEL_LABEL_STRING, "",
				   PANEL_ITEM_X, ATTR_COL(6),
				   PANEL_ITEM_Y,
				     ATTR_ROW(rows-MAX_PLAYERS+i),
				   0);
    }

    /* make cursor visible when in panel */
    
    cursor = window_get(panel, WIN_CURSOR);
    cursor_set(cursor, CURSOR_OP, PIX_SRC^PIX_DST, 0);
    window_set(panel, WIN_CURSOR, cursor, 0);

    /* start main processing */
    		  
    (void) notify_set_input_func(&sock, net_input, sock);
    (void) notify_set_itimer_func(&ship_num, hello, ITIMER_REAL,
	&hello_timer, ITIMER_NULL);
    window_main_loop(main_frame);
    exit(0);
}

void draw_vector(ship, p1, p2, on, boom_ang, boom_x, boom_y)
Object		*ship;
register int	p1;
register int	p2;
int		on;
int		boom_ang;
int		boom_x;
int		boom_y;
{
    register double	tsin;
    register double	tcos;
    
    if (ship->rot + boom_ang >= 0) {
	tsin = mysin((ship->rot + boom_ang) % 360);
	tcos = mycos((ship->rot + boom_ang) % 360);
    } else {
	tsin = mysin((ship->rot + boom_ang) % 360 + 360);
	tcos = mycos((ship->rot + boom_ang) % 360 + 360);
    }
    pw_vector(canvas_pw,
      ship->x_pos + boom_x + (int)(rt_point_list[p1][0]*
        (tcos*rt_point_list[p1][2]-tsin*rt_point_list[p1][1])),
      ship->y_pos - boom_y - (int)(rt_point_list[p1][0]*
        (tcos*rt_point_list[p1][1]+tsin*rt_point_list[p1][2])),
      ship->x_pos + boom_x + (int)(rt_point_list[p2][0]*
        (tcos*rt_point_list[p2][2]-tsin*rt_point_list[p2][1])),
      ship->y_pos - boom_y - (int)(rt_point_list[p2][0]*
        (tcos*rt_point_list[p2][1]+tsin*rt_point_list[p2][2])),
      PIX_SRC, on);
}

void display_object(object, on, high, boom_count)
Object	*object;
int	on;
int	high;
int	boom_count;
{
    Rect	lock_rect;
    /* boom_size contains the angles to rotate the ship vectors by
     * to get a nice explosion
     */

    switch (object->type) {
    case SHIP_T:
	lock_rect.r_left = object->x_pos - 15;
	lock_rect.r_top = object->x_pos - 15;
	lock_rect.r_width = 30;
	lock_rect.r_height = 30;
	pw_lock(canvas_pw, &lock_rect);
	draw_vector(object, 0, 1, on, boom_count*15, 2*boom_count, 2*boom_count);
	draw_vector(object, 1, 2, on, boom_count*(-15), -2*boom_count, 0);
	draw_vector(object, 2, 0, on, boom_count*15, 2*boom_count, -2*boom_count);
	if (high)
	    draw_vector(object, 0, 3, on, boom_count*(-15), 0, 0);
	if (object->flags & THRUSTING)
	    draw_vector(object, 3, 4, on, 0, 0, 0);
	if (object->flags & SHIELD) {
	    draw_vector(object, 17, 18, on, 0, 0, 0);
	    draw_vector(object, 18, 19, on, 0, 0, 0);
	    draw_vector(object, 19, 17, on, 0, 0, 0);
	}
	pw_unlock(canvas_pw);
	break;
    case MISSILE_T:
	pw_rop(canvas_pw, object->x_pos-1, object->y_pos-1, 3, 3,
	       PIX_SRC|PIX_COLOR(on), (struct pixrect *)0, 0, 0);
	break;
    case SUN_T:
	lock_rect.r_left = object->x_pos - 15;
	lock_rect.r_top = object->x_pos - 15;
	lock_rect.r_width = 30;
	lock_rect.r_height = 30;
	pw_lock(canvas_pw, &lock_rect);
	draw_vector(object, 5, 6, on, 0, 0, 0);
	draw_vector(object, 7, 8, on, 0, 0, 0);
	draw_vector(object, 9, 10, on, 0, 0, 0);
	draw_vector(object, 11, 12, on, 0, 0, 0);
	draw_vector(object, 13, 14, on, 0, 0, 0);
	draw_vector(object, 15, 16, on, 0, 0, 0);
	pw_unlock(canvas_pw);
	break;
    case ASTEROID_T:
	lock_rect.r_left = object->x_pos - 15;
	lock_rect.r_top = object->x_pos - 15;
	lock_rect.r_width = 30;
	lock_rect.r_height = 30;
	pw_lock(canvas_pw, &lock_rect);
	draw_vector(object, 20, 21, on, 0, 0, 0);
	draw_vector(object, 21, 22, on, 0, 0, 0);
	draw_vector(object, 22, 23, on, 0, 0, 0);
	draw_vector(object, 23, 24, on, 0, 0, 0);
	draw_vector(object, 24, 25, on, 0, 0, 0);
	draw_vector(object, 25, 26, on, 0, 0, 0);
	draw_vector(object, 26, 20, on, 0, 0, 0);
	pw_unlock(canvas_pw);
	break;
    }
}

void event_proc(win, event, arg)
Window	win;
Event	*event;
caddr_t	arg;
{
    struct header	head;
    
    if (win == (Window)panel)
	window_default_event_proc(win, event, arg);
    if (event_is_ascii(event)) {
	if (event_id(event) == '\033') {
	    head.type = BYE;
	    head.ship_num = ship_num;
	    if (sendto(sock, &head, sizeof(head), 0,
	      &serv_addr, sizeof(serv_addr)) == -1)
		die("Can't send:", 1);
	} else if (event_id(event) == shieldkey)
	    controls.shield = 1;
	else
            controls.fire = 1;
    } else if (event_is_button(event))
	switch(event_id(event)) {
	case MS_LEFT:
	    controls.left = !event_is_up(event);
	    break;
	case MS_MIDDLE:
	    controls.thrust = !event_is_up(event);
	    break;
	case MS_RIGHT:
	    controls.right = !event_is_up(event);
	    break;
	}
    else
	return;
    if (ship_num == -1 || objects[ship_num].flags & DYING)
	return;
    controls.seq++;
    if (sendto(sock, &controls, sizeof(controls), 0,
      &serv_addr, sizeof(serv_addr)) == -1)
	die("Can't send:", 1);
    (void) notify_set_itimer_func(&controls, control_send, ITIMER_REAL,
      &control_timer, ITIMER_NULL);
}

Notify_value control_send(me, which)
char *me;
int which;
{
    struct header	head;

    if (ship_num == -1 || objects[ship_num].flags & DYING)
	return;
    if (control_count++ > 20) {
	die("server not responding?\n", 0);
	head.type = BYE;
	head.ship_num = ship_num;
	if (sendto(sock, &head, sizeof(head), 0,
	  &serv_addr, sizeof(serv_addr)) == -1)
	    die("Can't send:", 1);
    }
    if (sendto(sock, &controls, sizeof(controls), 0,
      &serv_addr, sizeof(serv_addr)) == -1)
	die("Can't send:", 1);
}

Notify_value hello(me, which)
char *me;
int which;
{
    /* kludge to work around apparent Sunview bug */
    panel_paint(cp1, PANEL_CLEAR);
    panel_paint(cp2, PANEL_CLEAR);

    if (hello_count++ > 10)
	die("server not running?\n", 0);
    if (sendto(sock, &hello_head, sizeof(hello_head), 0,
      &serv_addr, sizeof(serv_addr)) == -1)
	die("Can't send:", 1);
    return NOTIFY_DONE;
}

char		input_buf[1500];

Notify_value net_input(me, fd)
char	*me;
int	fd;
{
    int			size;
    struct sockaddr_in	from_addr;
    int			from_len = sizeof(from_addr);
    struct header	*head;
    register int	i;
    register Object	*object;
    int			seq_diff;
    int			new_num_objects;

    size = recvfrom(sock, input_buf, sizeof(input_buf), 0,
      &from_addr, &from_len);
    if (size < 0)
	die("Can't read:", 1);
    if (size >= 2) {
	head = (struct header *)input_buf;
	switch (head->type) {
	/* tell me which ship i am */
	case YOU_ARE:
	    ship_num = head->ship_num;
	    controls.head.ship_num = ship_num;
	    controls.seq = 0;
	    last_ack = 0;
	    control_count = 0;
	    (void) notify_set_itimer_func(&controls, control_send,
	      ITIMER_REAL, ITIMER_NULL, ITIMER_NULL);
	    (void) notify_set_itimer_func(&ship_num, hello, ITIMER_REAL,
	      ITIMER_NULL, ITIMER_NULL);
	    serv_addr = from_addr;
	    break;
	/* update the screen */
	case DISPLAY:
	    /* acknowledged control information */
	    seq_diff = head->ship_num - last_ack;
	    if (seq_diff < 0)
		seq_diff += 256;
	    if (head->ship_num == controls.seq) {
		last_ack = head->ship_num;
		control_count = 0;
		(void) notify_set_itimer_func(&controls, control_send,
		  ITIMER_REAL, ITIMER_NULL, ITIMER_NULL);
		controls.fire = controls.shield = 0;
	    } else if (seq_diff > 0) {
		last_ack = head->ship_num;
		control_count = 0;
		(void) notify_set_itimer_func(&controls, control_send,
		  ITIMER_REAL, &control_timer, ITIMER_NULL);
	    }
	    new_num_objects = (size-sizeof(*head))/sizeof(Object);
	    /* display ships first */
	    for (i = 0, object = (Object *)(input_buf+sizeof(*head));
	      i < MAX_PLAYERS; i++, object++) {
	        if (objects[i].flags & ALIVE)
		    display_object(&objects[i], 0, i==ship_num, die_count[i]);
		if (object->flags & DYING && die_count[i] <= DIE_PERIOD)
		    die_count[i]++;
		else
		    die_count[i] = 0;
		if (object->flags & ALIVE)
		    display_object(object, 1, i == ship_num, die_count[i]);
	    }
	    /* display any extra objects */
	    for (i = MAX_PLAYERS; i < MAX_PLAYERS+OTHER_OBJECTS;
	      i++, object++) {
		if (objects[i].flags & ALIVE)
		    display_object(&objects[i], 0, 0, 0);
		if (object->flags & ALIVE)
		    display_object(object, 1, 0, 0);
	    }
	    /* undisplay old missile positions */
	    for (i = MISSILE_START; i < MISSILE_START+NUM_MISSILES &&
	      i < num_objects; i++) {
	        if (objects[i].flags & ALIVE)
		    display_object(&objects[i], 0, 0, 0);
		else if (i >= MISSILE_START)
		    break;
	    }
	    /* display new missile positions */
	    for (i = MISSILE_START; i < MISSILE_START+NUM_MISSILES &&
	      i < new_num_objects; i++, object++) {
		if (object->flags & ALIVE)
		    display_object(object, 1, 0, 0);
		else if (i >= MISSILE_START)
		    break;
	    }
	    num_objects = new_num_objects;
	    /* save objects to undisplay next update */
	    bcopy(input_buf+sizeof(*head), objects, num_objects*sizeof(Object));
	    if (ship_num != -1 && !(objects[ship_num].flags & ALIVE))
		dead();
	    break;
	/* list of active players */
	case USERS:
	    for (i = 0; i < MAX_PLAYERS; i++) {
		Player	*player = (Player *)(input_buf+
		  sizeof(*head)+sizeof(Player)*i);
		char	score_buf[10];
		
		panel_set(player_names[i],
		  PANEL_LABEL_STRING, &player->name[0], 0);
		if (player->name[0]) {
		    sprintf(score_buf, "%4d", player->score);
		    panel_set(player_scores[i],
		      PANEL_LABEL_STRING, score_buf, 0);
		} else
		    panel_set(player_scores[i], PANEL_LABEL_STRING, "", 0);
	    }
	    break;
	default:
	    break;
	}
    }
    return NOTIFY_DONE;
}

/* ask if the player wants to play again */
void dead()
{
    control_count = 0;
    (void) notify_set_itimer_func(&controls, control_send,
      ITIMER_REAL, ITIMER_NULL, ITIMER_NULL);
    panel_set(play_message, PANEL_SHOW_ITEM, TRUE, 0);
    panel_set(play_yes, PANEL_SHOW_ITEM, TRUE, 0);
    panel_set(play_no, PANEL_SHOW_ITEM, TRUE, 0);
    ship_num = -1;
}

void play_yes_no(item, event)
Panel_item	item;
Event		*event;
{
    if (item == play_no)
	exit(0);
    /* set up to play again */
    panel_set(play_message, PANEL_SHOW_ITEM, FALSE, 0);
    panel_set(play_yes, PANEL_SHOW_ITEM, FALSE, 0);
    panel_set(play_no, PANEL_SHOW_ITEM, FALSE, 0);
    pw_writebackground(canvas_pw, 0, 0, X_MAX, Y_MAX, PIX_SRC);
    hello_count = 0;
    (void) notify_set_itimer_func(&ship_num, hello, ITIMER_REAL,
	&hello_timer, ITIMER_NULL);
    controls.left = controls.right = controls.thrust =
      controls.fire = controls.shield = 0;
}

/* die because of some UNIX error */
void die(message, pperr)
char *message;
int pperr;
{
    fprintf(stderr, message);
    if (pperr)
	perror("spacewar");
    exit(1);
}
