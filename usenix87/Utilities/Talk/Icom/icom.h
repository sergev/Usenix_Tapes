/*	icom	Control structures, command transfers */

#define	ICOMSTAT	"/usr/src/Utilities/Icom/fifos/icomstat"
#define	SHMKEY	0x49434f4d	/* "ICOM" */
#define	NUMUSERS	4

/*	Main commands	Player to Controller		*/
#define	CLOGIN	0x01	/* login (cast: s_login)	*/
#define	CDCHAR	0x04	/* local display chars */
#define	CLOGOUT	0x06	/* logout (cast: s_pnum) */

struct	s_msg {		/* message template */
	long	mtype;	/* (see command list) */
	int	pnum;
	union {
		char	ta[80];
		int ch;
	} u;
};

struct	s_s1 {
	int	pnum;		/* player # 0-7 */
	int	ppid;		/* (-1 is no player) */
};

struct	s_shar {
	int	cpid;		/* controller pid */
	int	cstatus;	/* controller location */
	struct	s_s1 pp[NUMUSERS];
};

char	*fifolist[] = {
		"/usr/lib/icom/picom0",
		"/usr/lib/icom/picom1",
		"/usr/lib/icom/picom2",
		"/usr/lib/icom/picom3",
		"/usr/lib/icom/picom4",
		"/usr/lib/icom/picom5",
		"/usr/lib/icom/picom6",
		"/usr/lib/icom/picom7" };
