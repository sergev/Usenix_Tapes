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
 * $Log:	serverdata.c,v $
 * Revision 2.0  85/12/07  18:22:20  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: serverdata.c,v 2.0 85/12/07 18:22:20 toddb Rel $";
#include	"server.h"
#include	<nlist.h>
#include	<signal.h>
#include	<netdb.h>
#include	<sys/stat.h>

/*
 * system calls.
 */
long	access(), chdir(), chmod(), chown(), close(), dup(), execve(),
	fchmod(), fchown(), fcntl(), flock(), fork(), fstat(), fsync(),
	ftruncate(), ioctl(), link(), lseek(), lstat(), mkdir(),
	mknod(), open(), read(), readlink(), rename(), rmdir(),
	stat(), symlink(), truncate(), unlink(), utimes(),
	write(),
/*
 * ...and our own routines to set up for the system calls.
 */
	noop(), s_access(), s_dup(), s_execinfo(), s_execread(), s_exit(),
	s_fcntl(), s_fd1(), s_fd1_plus(), s_fork(), s_ioctl(), s_lseek(),
	s_open(), s_path1(), s_path1_plus(), s_path2(), s_read(),
	s_readlink(), s_stat(), s_utimes(), s_write();

syscallmap smap[] = {
	s_fork,		noop,     NEED_ZIP,		/* RSYS_fork */
	s_read,		read,     NEED_ZIP,		/* RSYS_read */
	s_write,	write,    NEED_ZIP,		/* RSYS_write */
	s_open,		open,	  NEED_CWD
				 |NEED_MYSERVER
				 |NEED_PERM
				 |NEED_FD,		/* RSYS_open */
	s_fd1,		close,    NEED_ZIP,		/* RSYS_close */
	noop,		noop,	  NEED_CWD
				 |NEED_PERM
				 |NEED_FD,		/* RSYS_creat */
	s_path2,	link,	  NEED_CWD
				 |NEED_MYSERVER
				 |NEED_2PATH
				 |NEED_2REMOTE
				 |NEED_PERM,		/* RSYS_link */
	s_path1,	unlink,   NEED_CWD|NEED_PERM,	/* RSYS_unlink */
	s_path1,	chdir,    NEED_MYSERVER
				 |NEED_CWD
				 |NEED_PERM
				 |NEED_MYSERVER,	/* RSYS_chdir */
	s_path1_plus,	mknod,    NEED_CWD|NEED_PERM,	/* RSYS_mknod */
	s_path1_plus,	chmod,    NEED_CWD|NEED_PERM,	/* RSYS_chmod */
	s_path1_plus,	chown,    NEED_CWD|NEED_PERM,	/* RSYS_chown */
	s_stat,		stat,     NEED_CWD|NEED_PERM,	/* RSYS_stat */
	s_lseek,	lseek,    NEED_ZIP,		/* RSYS_lseek */
	s_access,	access,   NEED_CWD|NEED_PERM,	/* RSYS_access */
	s_stat,		lstat,    NEED_CWD|NEED_PERM,	/* RSYS_lstat */
	s_dup,		dup,      NEED_FD,		/* RSYS_dup */
	s_ioctl,	ioctl,    NEED_ZIP,		/* RSYS_ioctl */
	s_path2,	symlink,  NEED_CWD
				 |NEED_2PATH
				 |NEED_PERM,		/* RSYS_symlink */
	s_readlink,	readlink, NEED_CWD|NEED_PERM,	/* RSYS_readlink */
	s_stat,		fstat,    NEED_ZIP,		/* RSYS_fstat */
	s_dup,		dup,      NEED_FD,		/* RSYS_dup2 */
	s_fd1_plus,	fcntl,    NEED_ZIP,		/* RSYS_fcntl */
	s_fd1,		fsync,    NEED_ZIP,		/* RSYS_fsync */
	noop,		noop,     NEED_ZIP,		/* RSYS_readv */
	noop,		noop,     NEED_ZIP,		/* RSYS_writev */
	s_fd1_plus,	fchown,   NEED_PERM,		/* RSYS_fchown */
	s_fd1_plus,	fchmod,   NEED_PERM,		/* RSYS_fchmod */
	s_path2,	rename,   NEED_MYSERVER
				 |NEED_2REMOTE
				 |NEED_CWD
				 |NEED_2PATH
				 |NEED_PERM,		/* RSYS_rename */
	s_path1_plus,	truncate, NEED_CWD|NEED_PERM,	/* RSYS_truncate */
	s_fd1_plus,	ftruncate,NEED_ZIP,		/* RSYS_ftruncate */
	s_fd1_plus,	flock,    NEED_ZIP,		/* RSYS_flock */
	s_path1_plus,	mkdir,    NEED_CWD|NEED_PERM,	/* RSYS_mkdir */
	s_path1,	rmdir,    NEED_CWD|NEED_PERM,	/* RSYS_rmdir */
	s_utimes,	utimes,   NEED_CWD|NEED_PERM,	/* RSYS_utimes */
	s_exit,		noop,	  NEED_ZIP,		/* RSYS_exit */
	s_fork,		noop,     NEED_ZIP,		/* RSYS_Vfork */
	s_execinfo,	noop,	  NEED_MYSERVER
				 |NEED_CWD
				 |NEED_PERM,		/* RSYS_execinfo */
	s_execread,	noop,	  NEED_PERM,		/* RSYS_execread */
	noop,		noop,	  NEED_ZIP,		/* RSYS_execve */
	noop,		noop,	  NEED_ZIP,		/* RSYS_nosys */
	s_lseek,	lseek,    NEED_ZIP,		/* RSYS_qlseek */
};

char	*syscallnames[] = {
	"fork",
	"read",
	"write",
	"open",
	"close",
	"creat",
	"link",
	"unlink",
	"chdir",
	"mknod",
	"chmod",
	"chown",
	"stat",
	"lseek",
	"access",
	"lstat",
	"dup",
	"ioctl",
	"symlink",
	"readlink",
	"fstat",
	"dup2",
	"fcntl",
	"fsync",
	"readv",
	"writev",
	"fchown",
	"fchmod",
	"rename",
	"truncate",
	"ftruncate",
	"flock",
	"mkdir",
	"rmdir",
	"utimes",
	"exit",
	"vfork",
	"execinfo",
	"execread",
	"execve",
	"nosys",
	"quick lseek"
};

char	hostname[ HOSTNAMELEN ];/* our host name */
char	mntpt[ MAXPATHLEN ];	/* mount point for client */
char	*program;		/* name of this program */
char	*last_argaddr;		/* last address that we can scribble on */
char	*service = REMOTE_FS_SERVER; /* name of alternate internet service */
char	*stdlogfile = "/usr/tmp/rfs_log"; /* log file  for server */
char	*logfile;
long	serviceport;		/* port number for service */
long	remote_debug;		/* level of debug output */
short	current_uid;		/* whatever uid we are, right now */
short	current_pid;		/* whatever pid we are, right now */
short	current_ppid;		/* our parent server */
short	current_umask;		/* whatever umask we have, right now */
short	current_server;		/* server that has control right now */
short	gateway_server;		/* pid of our gateway */
short	last_sentry;		/* previous sentry server (if non-zero) */
long	fds_in_use;		/* number of total file descriptors open */
long	to_gateway;		/* file descriptor for messages to gateway */
long	so_listen;		/* socket for listening for connections */
long	from_servers;		/* file descriptor for messages from servers */
long	blocking_servers;	/* number of servers waiting for I/O */
hosts	*hostlist;		/* all the hosts we know of */
hosts	*host;			/* the current host that we talk to */
hosts	*thishost;		/* host pointer for this machine */
users	*userlist;		/* all the users on this host we know of */
users	*default_user;		/* default user to map unknown clients to */
process *wildcard;		/* wildcard process for easy requests */
boolean	i_am_gateway = TRUE;	/* whether we are the gateway server */
boolean	i_have_control = TRUE;	/* whether the gateway server has control of */
				/* the command socket */
boolean	i_am_asleep;		/* whether we are sleeping or not */
boolean	gateway_needs_control;	/* True if gateway wants control back */
boolean	watch_for_lock;		/* True if we need to watch for lock on fd 2 */
boolean	route_to_gateway;	/* True if we should route to gateway */
boolean	in_root_directory = TRUE;/* whether we are at root directory or not */
struct stat	filetypes[ NOFILE ];	/* file types for open files */

char	byteorder[4] = { BYTEORDER };
long	catch(),
	nameserver(),
	wakeup_call(),
	alarmsig();

struct sigvec sig_continue = {
	wakeup_call,
	1<<(SIGIO -1),
	0
};

struct sigvec sig_ignore = {
	(int (*)())SIG_IGN,
	1<<(SIGHUP -1),
	0
};

struct sigvec sig_alarm = {
	alarmsig,
	1<<(SIGALRM -1),
	0
};

struct sigvec sig_name = {
	nameserver,
	1<<(SIGURG -1),
	0
};

struct sigvec sig_vec = {
	catch,
	 (1<<(SIGINT -1))
	|(1<<(SIGQUIT-1))
	|(1<<(SIGBUS-1))
	|(1<<(SIGILL-1))
	|(1<<(SIGSEGV-1))
	|(1<<(SIGPIPE-1))
	|(1<<(SIGSYS-1))
	|(1<<(SIGTERM-1))
	|(1<<(SIGTTIN-1))
	|(1<<(SIGTTOU-1))
	|(1<<(SIGXCPU-1))
	|(1<<(SIGXFSZ-1))
	|(1<<(SIGVTALRM-1)),
	0
};

#ifdef RFSDEBUG

long	newdebug();

struct sigvec sig_debug = {
	newdebug,
	(1<<(SIGTRAP -1)),
	0
};
#endif RFSDEBUG

struct stat	root;			/* stat info for root directory */
