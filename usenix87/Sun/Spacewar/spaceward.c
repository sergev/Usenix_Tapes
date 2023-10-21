/*
 *	Copyright 1987, Jonathan Biggar
 *
 *	Permission is given to copy and distribute for non-profit purposes.
 *
 */

#ifndef lint
static char *rcs = "$header$ Copyright 1986 Jonathan Biggar";
#endif !lint

/*
 * $log$
 */

#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/uio.h>
#include <netdb.h>
#include <sunwindow/notify.h>

#include "spacewar.h"

#define RANDOM(x)	(random() % x)

/* structure def for dbm package */

typedef struct {
    char	*dptr;
    int		dsize;
} datum;

datum		fetch(), firstkey(), nextkey();

/* internal info about objects not sent to clients */
typedef struct intobject {
    double	x_vel;
    double	y_vel;
    short	fuel;
    short	m_count;
    short	dying;
    short	shield;
} Intobject;

int			sock = 0;

int			client_count = 0;
struct sockaddr_in	clients[MAX_PLAYERS];
Player			players[MAX_PLAYERS];
struct header		player_header = {USERS, 0};
struct iovec		player_vec[2] = {(char *)&player_header,
					 sizeof(player_header),
					 (char *)players, sizeof(players)};
struct msghdr		player_packet = { 0,
					  sizeof(struct sockaddr_in),
					  player_vec, 2, 0, 0};

struct timeval		cycle_time = {0, 0};
unsigned long		cycle = 0;
struct itimerval	die_timer = {0, 0, 10, 0};
struct itimerval	update_timer = {0, UPDATE_FREQ, 0, UPDATE_FREQ};
struct header		update_header = {DISPLAY, 0};
Object			objects[MAX_OBJECTS];
Intobject		intobjects[MAX_OBJECTS];
Controls		controls[MAX_PLAYERS];
unsigned long		control_update[MAX_PLAYERS];
int			next_missile = MISSILE_START;
int			star;
int			gravity_sign;

struct iovec		update_vec[2] = {(char *)&update_header,
					 sizeof(update_header),
					 (char *)objects, sizeof(objects)};
struct msghdr		update_packet = { 0,
					  sizeof(struct sockaddr_in),
					  update_vec, 2, 0, 0};

double			deg_2_rad;
double			sin_tab[360];
double			cos_tab[360];

char			*score_file = SCOREFILE;
int			by_machine = BYMACHINE;

void			die();
Notify_value		update();
Notify_value		read_net();
int			hit();
void			send_names();

long			random();
char			*strcpy(), *index();
struct in_addr		inet_makeaddr();

main(argc, argv)
int argc;
char **argv;
{
    struct hostent	*he;
    struct servent	*se;
    char		hostname[32];
    register int	i;

#ifdef DEBUG
    struct sockaddr_in	my_addr;

    /* if debugging start server listening by hand */
    
    if ((se = getservbyname("spaceward", "udp")) == NULL)
	die();
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = se->s_port;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	die();
    if (bind(sock, &my_addr, sizeof(my_addr)) == -1)
	die();
#endif DEBUG

    /* find the broadcast address */
    
    gethostname(hostname, sizeof(hostname));
    if ((he = gethostbyname(hostname)) == NULL)
	die();
    if ((se = getservbyname("spacewar", "udp")) == NULL)
	die();

    /* fill in the sine and cosine tables for speed */
    
    deg_2_rad = acos(-1.0)/180.0;
    for (i = 0; i < 360; i++) {
	sin_tab[i] = sin(i*deg_2_rad);
	cos_tab[i] = cos(i*deg_2_rad);
    }
    
    /* set up the score file */
    
    if (dbminit(score_file) < 0) {
	char cmd_buf[100];
	
	sprintf(cmd_buf, "touch %s.dir %s.pag", SCOREFILE, SCOREFILE);
	system(cmd_buf);
	if (dbminit(score_file) < 0)
	    die();
    }
    
    /* randomize */
    
    srandom(time(0));
    
    /* start main processing */
    (void) notify_set_input_func(&sock, read_net, sock);
    (void) notify_set_itimer_func(die, die, ITIMER_REAL,
      &die_timer, (struct itimerval *)0);
    notify_start();
}

/* go away because of an error */
void die()
{
    char	buf[80];
    
    sprintf(buf, "echo %d %d / %d >>/usr/games/lib/spacewar.times",
	cycle_time.tv_sec, cycle_time.tv_usec, cycle);
    system(buf);
    exit(0);
}

/* decide on the initial configuration of the universe.  Done whenever
 * no players are active and one wants to join 
 */
void startup()
{
    int		i;

    for (i = MAX_PLAYERS; i < MISSILE_START; i++) {
	objects[i].flags &= ~ALIVE;
	intobjects[i].m_count = RANDOM(ASTEROID_MAX_PER);
    }
    if (star) {
	objects[star].flags &= ~ALIVE;
	star = 0;
    }
    if (RANDOM(2)) {
	star = MAX_PLAYERS;
	objects[star].type = SUN_T;
	objects[star].flags |= ALIVE;
	objects[star].x_pos = X_MAX/2;
	objects[star].y_pos = Y_MAX/2;
	objects[star].rot = 0;
	if (RANDOM(2))
	    gravity_sign = 1;
	else
	    gravity_sign = -1;
    }
    (void) notify_set_itimer_func(update, update, ITIMER_REAL,
      &update_timer, (struct itimerval *)0);
    (void) notify_set_itimer_func(die, die, ITIMER_REAL,
      (struct itimerval *)0, (struct itimerval *)0);
}

/* client's ship has been hit, set up for explosion time */
void die_ship(i)
int i;
{
    if (objects[i].flags & DYING)
	return;
    objects[i].flags |= DYING;
    intobjects[i].dying = DIE_PERIOD;
    intobjects[i].shield = 0;
    objects[i].flags &= ~SHIELD;
    controls[i].thrust = controls[i].fire = controls[i].shield = 0;
}

/* client's ship is dead, get rid of client and start shutdown timer if 
 * last client
 */
void kill_ship(i)
int i;
{
    datum	key, content;
    char	buf[40];
    char	*cp;

    if (by_machine)
	strcpy(buf, &players[i].name[0]);
    else {
	cp = index(&players[i].name[0], '@');
	if (cp != NULL)
	    strncpy(buf, &players[i].name[0],
	      cp-&players[i].name[0]+1);
    else
	strcpy(buf, "unknown");
    }
    key.dptr = buf;
    key.dsize = strlen(buf);
    content.dptr = (char *)&players[i].score;
    content.dsize = sizeof(players[i].score);
    store(key, content);
    objects[i].flags &= ~(ALIVE|DYING);
    players[i].name[0] = '\0';
    send_names();
    if (--client_count == 0) {
	(void) notify_set_itimer_func(update, update, ITIMER_REAL,
	  (struct itimerval *)0, (struct itimerval *)0);
	(void) notify_set_itimer_func(die, die, ITIMER_REAL,
	  &die_timer, (struct itimerval *)0);
    }
    
}

/* calculate the stars gravitational influence on an object */
void sun_gravity(object, intobject)
Object		*object;
Intobject	*intobject;
{
    int			tx = object->x_pos - objects[star].x_pos;
    int			ty = object->y_pos - objects[star].y_pos;
    double		dist = tx*tx+ty*ty;
    
    intobject->x_vel += gravity_sign * 4000*tx/pow(dist, 1.5);
    intobject->y_vel += -gravity_sign * 4000*ty/pow(dist, 1.5);
}

/* update the universe */
Notify_value update(me, which)
char *me;
int which;
{
    register int	i;
    register int	j;
    int			missile_offset = 0;
    register Object	*object;
    register Intobject	*intobject;
    struct timeval	start, stop;
    int			last_ship;

    gettimeofday(&start, 0);
    cycle++;
    for (i = 0, object = &objects[0], intobject = &intobjects[0];
      i < MAX_PLAYERS; i++, object++, intobject++)
	/* update the positions of each ship */
	if (object->flags & ALIVE) {
	    last_ship = i;
	    if (cycle - control_update[i] > 1000) {
		kill_ship(i);
		continue;
	    }
	    if (intobject->shield > 0) {
		intobject->shield--;
		if (intobject->shield < SHIELD_RECHARGE - SHIELD_TIME)
		    object->flags &= ~SHIELD;
	    }
	    if (controls[i].shield && intobject->shield == 0) {
		intobject->shield = SHIELD_RECHARGE;
		object->flags |= SHIELD;
	    }
	    controls[i].shield = 0;
	    if (controls[i].left) {
		object->rot += ROT_SPEED;
		object->rot %= 360;
	    }
	    if (controls[i].right) {
		object->rot += 360 - ROT_SPEED;
		object->rot %= 360;
	    }
	    /* check for collisions */
	    for (j = i+1; j < MAX_PLAYERS+OTHER_OBJECTS; j++) {
		int	die_bonus;
		if (!(objects[j].flags & ALIVE))
		    continue;
		switch (objects[j].type) {
		case SHIP_T:
		    die_bonus = ((object->flags&DYING)?
		      DIE_PERIOD-intobject->dying:0) +
		      ((objects[j].flags&DYING)?
		      DIE_PERIOD-intobjects[j].dying:0);
		    if (hit(objects[j].x_pos, objects[j].y_pos,
			  object->x_pos, object->y_pos,
			  (int)(object->x_pos + intobject->x_vel),
			  (int)(object->y_pos - intobject->y_vel),
			  KILL_RADIUS+die_bonus*2)) {
			if (!(object->flags & SHIELD))
			    die_ship(i);
			if (!(objects[j].flags & SHIELD))
			    die_ship(j);
		    }
		    break;
		case SUN_T:
		    if (!(object->flags & SHIELD) &&
		      hit(objects[j].x_pos, objects[j].y_pos,
		      object->x_pos, object->y_pos,
		      (int)(object->x_pos+intobject->x_vel),
		      (int)(object->y_pos-intobject->y_vel), 3)) {
			intobject->x_vel = intobject->y_vel = 0;
			if (!(object->flags & DYING) && players[i].score > 0)
			    players[i].score--;
			die_ship(i);
		    }
		    break;
		case ASTEROID_T:
		    if (!(object->flags & SHIELD) &&
		      hit(objects[j].x_pos, objects[j].y_pos,
		      object->x_pos, object->y_pos,
		      (int)(object->x_pos+intobject->x_vel),
		      (int)(object->y_pos-intobject->y_vel), KILL_RADIUS*2)) {
			if (!(object->flags & DYING) && players[i].score > 0)
			    players[i].score--;
			die_ship(i);
		    }
		    break;
		}
	    }
	    /* move the ship */
	    object->x_pos += (int)intobject->x_vel + X_MAX;
	    object->y_pos += -(int)intobject->y_vel + Y_MAX;
	    object->x_pos %= X_MAX;
	    object->y_pos %= Y_MAX;
	    if (star && !(object->flags & SHIELD))
		sun_gravity(object, intobject);
	    /* if ship finished exploding, kill it */
	    if (object->flags & DYING && !--intobject->dying) {
		kill_ship(i);
		continue;
	    }
	    object->flags &= ~THRUSTING;
	    if (controls[i].thrust) {
		register double		hypot1, hypot2;
		register double		tx, ty;
		
		/* limit thrust to a maximum velocity */
		hypot1 = sqrt(intobject->x_vel*intobject->x_vel+
			      intobject->y_vel*intobject->y_vel);
		tx = intobject->x_vel + mycos(object->rot);
		ty = intobject->y_vel + mysin(object->rot);
		hypot2 = sqrt(tx*tx+ty*ty);
		if (hypot2 <= MAX_VELOCITY || hypot2 <= hypot1) {
		    intobject->x_vel = tx;
		    intobject->y_vel = ty;
		} else {
		    intobject->x_vel = hypot1*tx/hypot2;
		    intobject->y_vel = hypot1*ty/hypot2;
		}
		object->flags |= THRUSTING;
	    }
	    if (controls[i].fire && intobject->m_count < MIS_P_PLAYER) {
		objects[next_missile] = *object;
		objects[next_missile].type = MISSILE_T;
		intobjects[next_missile].fuel = Y_MAX/MISSILE_VELOCITY-1;
		intobjects[next_missile].m_count = i;
		intobjects[next_missile].x_vel = intobject->x_vel +
		  MISSILE_VELOCITY * mycos(object->rot);
		intobjects[next_missile].y_vel = intobject->y_vel +
		  MISSILE_VELOCITY * mysin(object->rot);
		next_missile++;
		intobject->m_count++;
	    }
	    controls[i].fire = 0;
	} else {
	    intobject->dying--;
	}
    for (i = MAX_PLAYERS, object = &objects[i], intobject= &intobjects[i];
      i < MISSILE_START; i++, object++, intobject++)
	if (object->type == SUN_T && object->flags & ALIVE) {
	    objects[star].rot += 30;
	    objects[star].rot %= 360;
	} else {
	    if (object->flags & ALIVE) {
		object->x_pos += intobject->x_vel;
		object->y_pos -= intobject->y_vel;
		object->rot += intobject->fuel + 360;
		object->rot %= 360;
		if (object->x_pos < 0 || object->x_pos >= X_MAX ||
		  object->y_pos < 0 || object->y_pos >= Y_MAX) {
		    object->flags &= ~ALIVE;
		    intobject->m_count =
		      RANDOM(ASTEROID_MAX_PER-ASTEROID_MIN_PER) +
		      ASTEROID_MIN_PER;
		    continue;
		}
		if (star)
		    sun_gravity(object, intobject);
	    } else if (--intobject->m_count == 0) {
		object->type = ASTEROID_T;
		object->flags |= ALIVE;
		object->rot = 0;
		intobject->fuel = RANDOM(30) - 15;
		switch (RANDOM(4)) {
		case 0:
		    object->x_pos = 0;
		    object->y_pos = RANDOM(Y_MAX);
		    intobject->x_vel = RANDOM(10)+1;
		    intobject->y_vel = -RANDOM(10)+5;
		    break;
		case 1:
		    object->x_pos = X_MAX-1;
		    object->y_pos = RANDOM(Y_MAX);
		    intobject->x_vel = -RANDOM(10)+1;
		    intobject->y_vel = -RANDOM(10)+5;
		    break;
		case 2:
		    object->y_pos = 0;
		    object->x_pos = RANDOM(X_MAX);
		    intobject->y_vel = -RANDOM(10)+1;
		    intobject->x_vel = RANDOM(10)-5;
		    break;
		case 3:
		    object->y_pos = Y_MAX-1;
		    object->x_pos = RANDOM(X_MAX);
		    intobject->y_vel = RANDOM(10)+1;
		    intobject->x_vel = RANDOM(10)-5;
		    break;
		}
	    }
	}
    for (i = MISSILE_START, object = &objects[i], intobject = &intobjects[i];
      i < MISSILE_START+NUM_MISSILES; i++, object++, intobject++)
        /* update each missile */
	if (object->flags & ALIVE) {
	    if (--intobject->fuel > 0) {
		for (j = 0; j < MAX_PLAYERS+OTHER_OBJECTS; j++)
		    if (objects[j].flags & ALIVE &&
		        (intobject->m_count != j ||
		        intobject->fuel != Y_MAX/MISSILE_VELOCITY - 2) &&
			hit(objects[j].x_pos, objects[j].y_pos,
		            object->x_pos, object->y_pos,
		            (int)(object->x_pos + intobject->x_vel),
		            (int)(object->y_pos - intobject->y_vel),
		            KILL_RADIUS)) {
			object->flags &= ~ALIVE;
			if (objects[j].type != SHIP_T)
			    continue;
			if (intobject->m_count != j &&
			  !(objects[j].flags & (DYING|SHIELD)))
			    if (players[intobject->m_count].score >
			      players[j].score)
				players[intobject->m_count].score++;
			    else
				players[intobject->m_count].score += 
				  (players[j].score-
				   players[intobject->m_count].score)/10+1;
			if (!(objects[j].flags & SHIELD))
			    die_ship(j);
		    }
		/* move the missile */
		object->x_pos += (int)intobject->x_vel + X_MAX;
		object->y_pos += -(int)intobject->y_vel + Y_MAX;
		object->x_pos %= X_MAX;
		object->y_pos %= Y_MAX;
		if (star)
		    sun_gravity(object, intobject);
		/* if the missile hit something, remove it */
		if (!(object->flags & ALIVE)) {
		    intobjects[intobject->m_count].m_count--;
		    missile_offset++;
		} else if (missile_offset > 0) {
		    /* compact the missile list */
		    objects[i-missile_offset] = *object;
		    intobjects[i-missile_offset] = *intobject;
		    object->flags &= ~ALIVE;
		}
	    /* missile ran out of fuel */
	    } else {
	        object->flags &= ~ALIVE;
		intobjects[intobject->m_count].m_count--;
		missile_offset++;
	    }
	/* end of missile list */
	} else {
	    next_missile = i - missile_offset;
	    break;
	}
    update_vec[1].iov_len = (&objects[next_missile]-objects)*sizeof(Object);
    for (i = 0; i <= last_ship; i++)
	if (objects[i].flags & ALIVE || intobjects[i].dying > -300) {
	    update_packet.msg_name = (caddr_t) &clients[i];
	    update_header.ship_num = controls[i].seq;
	    if (sendmsg(sock, &update_packet, 0) == -1)
		die();
	}
    gettimeofday(&stop, 0);
    if (start.tv_sec == stop.tv_sec)
	cycle_time.tv_usec += stop.tv_usec - start.tv_usec;
    else
	cycle_time.tv_usec += 1000000 + stop.tv_usec - start.tv_usec;
    if (cycle_time.tv_usec > 1000000) {
	cycle_time.tv_usec -= 1000000;
	cycle_time.tv_sec += 1;
    }
    return NOTIFY_DONE;
}

/* this macro calculates the distance between two points, taking into account
 * wrap-around 
 */

#define DISTANCE(x1, x2, y1, y2, r) \
    delx = abs(x1 - x2); \
    dely = abs(y1 - y2); \
    if (delx > X_MAX/2) \
	delx = X_MAX - delx; \
    if (dely > Y_MAX/2) \
	dely = Y_MAX - dely; \
    r = delx * delx + dely * dely;

/* see if a moving object hit another one:
 * stationary object at sx, sy,
 * moving object moving from mx1, my1, to mx2, my2,
 * kill radius is kill
 */
int hit(sx, sy, mx1, my1, mx2, my2, kill)
int sx, sy, mx1, my1, mx2, my2, kill;
{
    register int	a2, b2, c2;
    register int	delx, dely;
    register double	a;
    register double	b;
    double		range1;
    double		range2;
    
    DISTANCE(sx, mx1, sy, my1, a2);
    DISTANCE(mx1, mx2, my1, my2, b2);
    DISTANCE(mx2, sx, my2, sy, c2);
    if (a2 == 0 || c2 == 0)
	return 1;
    a = sqrt((double)a2);
    b = sqrt((double)b2);
    range2 = (double)(a2 + b2 - c2) / (2 * a * b);
    range1 = a*sqrt(1-range2*range2);
    range2 *= a;
    return range1 < kill && range2 > 0 && range2 < b;
}

char	input_buf[1500];

Notify_value read_net(me, fd)
char	*me;
int	fd;
{
    int			size;
    struct sockaddr_in	from_addr;
    int			from_size = sizeof(from_addr);
    struct header	*head;
    register int	i;
    
    size = recvfrom(sock, input_buf, sizeof(input_buf), 0,
      &from_addr, &from_size);
    if (size < 0)
	die();
    if (size >= 2) {
	head = (struct header *)input_buf;
	switch (head->type) {
	/* someone wants to play */
	case HELLO:
	    for (i = 0; i < MAX_PLAYERS; i++) {
		struct header	you_are;
		datum		key, content;
		char		*cp;
		char		buf[40];
		
		if ((objects[i].flags & ALIVE) &&
		  bcmp(&from_addr, &clients[i], sizeof(from_addr)))
		    continue;
		if (!(objects[i].flags & ALIVE) && ++client_count == 1)
		    startup();
		clients[i] = from_addr;
		you_are.type = YOU_ARE;
		you_are.ship_num = i;
		if (sendto(sock, &you_are, 2, 0,
		  &clients[i], sizeof(clients[i])) == -1)
		    die();
		strcpy(&players[i].name[0], input_buf+sizeof(*head));
		if (by_machine)
		    strcpy(buf, &players[i].name[0]);
		else {
		    cp = index(&players[i].name[0], '@');
		    if (cp != NULL)
			strncpy(buf, &players[i].name[0],
			  cp-&players[i].name[0]+1);
		    else
			strcpy(buf, "unknown");
		}
		key.dptr = buf;
		key.dsize = strlen(buf);
		content = fetch(key);
		if (content.dptr != 0)
		    players[i].score = *(int *)content.dptr;
		else
		    players[i].score = 0;
		send_names();
		if (objects[i].flags & ALIVE)
		    break;
		/* initialize his ship */
		objects[i].flags |= ALIVE;
		objects[i].flags &= ~DYING;
		objects[i].x_pos = RANDOM(X_MAX);
		objects[i].y_pos = RANDOM(Y_MAX);
		objects[i].rot = RANDOM(360/ROT_SPEED)*ROT_SPEED;
		controls[i].left = controls[i].right = controls[i].thrust
		  = controls[i].fire = 0;
		intobjects[i].x_vel = intobjects[i].y_vel = 0;
		control_update[i] = cycle;
		break;
	    }
	    break;
	/* someone is maneuvering */
	case CONTROLS:
	    {
		u_char		fire = controls[head->ship_num].fire;
		u_char		shield = controls[head->ship_num].shield;
		int		seq_dif = ((Controls *)input_buf)->seq -
					  controls[head->ship_num].seq;

		if (seq_dif < 0)
		    seq_dif += 256;
		if (!(objects[head->ship_num].flags & DYING) && seq_dif > 0)
		    controls[head->ship_num] = *(Controls *)input_buf;
		controls[head->ship_num].fire |= fire;
		controls[head->ship_num].shield |= shield;
		control_update[head->ship_num] = cycle;
	    }
	    break;
	/* someone is going away */
	case BYE:
	    objects[head->ship_num].flags |= DYING;
	    intobjects[head->ship_num].dying = 1;
	    break;
	default:
	    break;
	}
    }
    return NOTIFY_DONE;
}

/* update the list of players */
void send_names()
{
    int i;

    for (i = 0; i < MAX_PLAYERS; i++)
	if (objects[i].flags & ALIVE) {
	    player_packet.msg_name = (caddr_t) &clients[i];
	    if (sendmsg(sock, &player_packet, 0) == -1)
        	die();
	}
}
