/*
 * Copyright 1985, Todd Brunhoff.
 *
 * This software was written at Tektronix Computer Research Laboratories
 * as partial fulfillment of a Master's degree at the University of Denver.
 * This is not Tektronix proprietary software and should not be
 * confused with any software product sold by Tektronix.  No warranty is
 * expressed or implied on the reliability of this software; the author,
 * the University of Denver, and Tektronix, inc. accept no liability for
 * any damage done directly or indirectly by this software.  This software
 * may be copied, modified or used in any way, without fee, provided this
 * notice remains an unaltered part of the software.
 *
 * $Header: server.h,v 2.0 85/12/07 18:22:12 toddb Rel $
 *
 * $Log:	server.h,v $
 * Revision 2.0  85/12/07  18:22:12  toddb
 * First public release.
 * 
 */
#include	<sys/param.h>
#include	<sys/mbuf.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<remote/remotefs.h>

typedef	unsigned char	boolean;

/*
 * The maximum number of longs in a message that we accept
 */
#define	MAXMSGS		((R_MAXMBUFS*MLEN)/sizeof(long))
/*
 * The name of a host for which we have no record.  And the name of the user
 * to use if we don't recognize the user on the remote host.
 */
#define	BOGUSHOST	"unknown host"
#define	BOGUSUSER	"unknown user"
#define DEFAULTUSER	"guest"

/*
 * The uid number below which we reserve for privilaged users.
 */
#define	UID_TOO_LOW	20

/*
 * This is to make the debug? macro work for the server.
 *
 * The bits and what they turn on are as follows
 *	0x00000001	process switching
 *	0x00000002	system calls
 *	0x00000004	setuid/setgid, umask
 *	0x00000008	file descriptor allocation
 *	0x00000010	connections
 *	0x00000020	server switching
 *	0x00000040	nameserver
 *	0x00000080	directory nuxi
 *	0x00000100	message in and out
 *	0x00000200	don't fork child for gateway (good for adb)
 *	0x00000400	local/remote file decisions
 *	0x00000800	don't remove log file on exit (except exit on error)
 *	0x00001000	exec information
 *	0x00002000	auto debug for 0x20 (server switching)
 *	0x00004000	parsing messages to gateway
 */
#define	rmt_debug	log
#ifndef RFSDEBUG
#define dumphost()
#endif RFSDEBUG

/*
 * The size of the initial allocation for internal io buffers.
 */
#define	BIGBUF		(8*1024)

/*
 * other Manifest constants...
 */
#define	HOSTNAMELEN	255

/*
 * Map a file descriptor from the user's fd to our own internal fd.
 */
#define	MAPFD(fd, proc)			\
	(((unsigned)fd > NOFILE) ? -1 : (proc)->p_fds[ fd ] )
/*
 * requirements for different system calls.
 */
#define	NEED_ZIP	0x000	/* don't need anything special */
#define	NEED_CWD	0x001	/* need the current working directory set */
#define	NEED_PERM	0x002	/* need the user and group ids set */
#define	NEED_FD		0x004	/* need a file descriptor allocated */
#define	NEED_2PATH	0x010	/* uses two paths */
#define	NEED_MYSERVER	0x020	/* must be run by the assigned server */
#define	NEED_2REMOTE	0x040	/* both paths must be remote */

/*
 * Commands to the server sent by external programs (like rmtmnt, to mount
 * a remote system.
 */
#define	CMD_SERVICE	1
#define	CMD_MOUNT	2	/* here is mount information */
#define	CMD_NEEDMOUNT	3	/* give me mount information */
#define	CMD_WHOAMI	4	/* what uid am I on your host */

/*
 * Finally, some commands that are sent to the gateway server by other
 * servers.
 */
#define	S_NEWSERVER	0
#define	S_NEWPROCESS	1
#define	S_ALLDONE	2
#define	S_THIS_IS_YOURS	3
#define	S_PROCEXIT	4
#define	S_EOF		5
#define	S_I_WOULD_BLOCK	6
#define	S_I_AM_READY	7
#define	S_CORRUPTED	8

/*
 * Macros for getting the address of the paths out of the incomming message.
 * Note that path1addr is for system calls that deal with only one path,
 * and twopath1addr() and twopath2addr() are for system calls that have
 * two paths (in the latter cases, path2 appears in the same position
 * as path1 does for single path system calls).
 */
#define	path1addr(msg)		((char *)&(msg)->m_args[R_PATHSTART])
#define	twopath1addr(msg)	((char *)(msg) + (msg)->m_args[R_PATHOFF])
#define	twopath2addr(msg)	((char *)&(msg)->m_args[R_PATHSTART])

/*
 * Macro for preventing getmsg(), and thereby gobble_last_msg() from
 * reading the last message.  This is used when control is being passed
 * from one server to another.
 */
#define	dont_gobble_msg(msg)	(msg)->m_totlen = 0

/*
 * Macro for determining whether a stat structure reflects the root inode
 * of our machine or not.
 */
#define	isroot(p)	(p->st_ino == root.st_ino && p->st_dev == root.st_dev)

/*
 * This structure is one-to-one with each local user where the server runs.
 * Note that the u_next and u_prev pointers must be located
 * in the first and second spots of the structure, respectively so that
 * they can be modified by the linked-list routines.
 */
typedef struct users_type	users;
struct users_type {
	users	*u_next;	/* pointers for linked list, both forward... */
	users	*u_prev;	/* ... and back. */
	char	*u_name;	/* The ascii name of same. */
	char	*u_dir;		/* login directory for same */
	char	*u_rhosts;	/* path of the user's rhost file */
	short	u_local_uid;	/* A user id number on the local host */
	long	u_local_groups[NGROUPS];/* The groups this user belongs to */
	char	u_numgroups;	/* The number of groups in u_local_groups */
};

/*
 * Remote user specification.
 */
typedef struct ruser_type	rusers;
struct ruser_type {
	rusers	*r_next;
	rusers	*r_prev;
	short     r_uid;	/* Uid number on remote host ... */
	char	*r_name;	/* Uid name on remote host ... */
	users	*r_user;	/* corresponding local user */
};

/*
 * This is the important stuff.  There is one of these structures for
 * each pid that we are providing service to.
 */
typedef struct process_type	process;
struct process_type {
	process		*p_next;
	process		*p_prev;
	long		p_returnval;	/* return value from last syscall */
	rusers		*p_ruser;	/* info about the owner of this pid */
	short		p_pid;		/* process id number on remote host */
	short		p_uid;		/* remote uid that was last known */
	short		p_handler;	/* the handler for this process */
	short		p_errno;	/* errno for the system call */
	char		p_execfd;	/* file descriptor of exec file */
	boolean		p_execstarted;	/* whether we have done first read */
	char		p_fds[ NOFILE ];/* fd's assoc. with this pid */
};

/*
 * This structure keeps track of the possible hosts that may make a connection
 * the the remote fs server.  Note that the h_next and hprev pointers must be
 * located in the first and second spots of the structure, respectively,
 * so that they can be modified by the linked-list routines.
 */
typedef struct hosts_type	hosts;
struct hosts_type {
	hosts	*h_next;
	hosts	*h_prev;
	char	**h_names;	/* name (and aliases) of a host */
	rusers	*h_rusers;	/* the user list for this host */
	users	*h_default_user;/* default local user (if defined */
	rusers	*h_default_ruser;/* default when the remote user is unknown */
	long	h_portnum;	/* port number that we connected on */
	char	*h_mntpt;	/* mount point for this machine */
	process	*h_proclist;	/* processes we know about on this host */
	struct in_addr	h_addr;	/* network address */
	union h_bytes {
		long	hu_mounted; /* non-zero if host has been mounted */
		u_char	hu_byteorder[4]; /* byte order for this host */
	} h_bytes;
#define		h_mounted	h_bytes.hu_mounted
#define		h_byteorder	h_bytes.hu_byteorder
	char	h_cmdfd;	/* file descriptor for commands */
	boolean	h_byteorderok;	/* true if byte order same as ours */
	short	h_serverpid;	/* gateway server for this host */
};

/*
 * This structure is the mask structure that the linked list routines use
 * to modify any linked-list type of structure.  Note that l_next and l_prev
 * must be in the first and second spots.
 */
typedef struct l_list_type	l_list;
struct l_list_type {
	l_list	*l_next;
	l_list	*l_prev;
	long	l_data;		/* never used */
};

/*
 * This structure is for convenience: it simply defines an easy way of
 * storing the host/user line found in a .rhost file.
 */
typedef struct rhost_type	rhost;
struct rhost_type {
	char	*rh_host;
	char	*rh_user;
};

/*
 * Each message from the servers to the gateway, is placed into this
 * structure, the "gateway message".
 */
typedef struct gtmsg_type	gtmsgs;
struct gtmsg_type {
	short	g_server;	/* server that sent the message. */
	short	g_pid;		/* pid of whom this message is about */
	short	g_cmd;		/* what this message is about */
};

/*
 * Finally, this is the way we keep database info on the system calls
 * themselves.
 */
typedef struct syscallmap	syscallmap;
struct syscallmap {
	func	s_server;
	func	s_syscall;
	char	s_type;
};

rhost	*getrhostent();
hosts	*tcpaccept();
hosts	*findhostaddr();
hosts	*findhostname();
hosts	*newhost();
l_list	*toplist();
l_list	*bottomlist();
l_list	*addlist();
l_list	*deletelist();
process *newprocess();
process	*findprocess();
process	*change_to_proc();
process	*add_new_process();
users	*finduid();
users	*findusername();
users	*newuser();
rusers	*findremuid();
rusers	*findrusername();
rusers	*newruser();
char	**newname();
char	*copy();
char	*malloc();
char	*realloc();
char	*get_data_buf();
short	*newshortlist();
