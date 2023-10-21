/* RCS Information: $Header: struct.h,v 1.1 84/08/25 17:24:58 lai Exp $ */

struct state {
	int	s_flag;
	char	s_x;
	char	s_y;
	char	s_dir;
	char	s_hp;
	char	s_incarnation;		/* which incarnation of this user */
	int	s_kills;
};
#define CHANGE 		0x01
#define MOVE 		0x02

struct act {
	int	a_flag;
	char	a_victem;			/* who I shot */
	char	a_killer;			/* who I shot */
	char	a_incarnation;		/* which incarnatin I shot */
};
#define A_SHOT 		0x001
#define A_KILL 		0x002
#define A_QUERY		0x004		/* are you there? */
#define A_DAEMON	0x010		/* daemon has died */

struct user {
	char	u_slot;
	char	u_flag;
	char	u_name[32];
	char	u_hostname[16];
	struct	state u_state;
#define u_x	u_state.s_x
#define u_y	u_state.s_y
#define u_dir	u_state.s_dir
#define u_hp	u_state.s_hp
#define u_kills	u_state.s_kills
	int	u_version;			/* compat check */
	long	u_magic;			/* security */
};

#define U_ALIVE			1		/* alive */
#define U_DEAD			2		/* hey! I'm dead! */
#define U_NEW			3		/* new player */
#define U_BADVERSION		4		/* hey! I'm a moose! */

struct packet {
	char	p_flag;
	char	p_slot;				/* my slot number */
	union {
		struct user	pu_user;
		struct state	pu_state;
		struct act	pu_act;
	} p_data;
};

#define P_USER		0x001		/* this packet contains a user struct */
#define P_STATE		0x002		/* this is a state struct packet */
#define P_ACT	0x004		/* this is an act struct */
