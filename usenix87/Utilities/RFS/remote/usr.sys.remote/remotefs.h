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
 * $Header: remotefs.h,v 2.1 86/01/05 18:17:01 toddb Exp $
 *
 * $Log:	remotefs.h,v $
 * Revision 2.1  86/01/05  18:17:01  toddb
 * Added ifdef'ed constants for pyramids: FREMOTE, SREMOTE and SNOREMOTE.
 * 
 * Revision 2.0  85/12/07  18:17:35  toddb
 * First public release.
 * 
 */
#ifndef RFSDEBUG
#define debug0()
#define debug1()
#define debug2()
#define debug3()
#define debug4()
#define debug5()
#define debug6()
#define debug7()
#define debug8()
#define debug9()
#define debug10()
#define debug11()
#define debug12()
#define debug13()
#define debug14()
#define rmt_showmsg()
#else RFSDEBUG
/*
 * Each of the debugging macros are defined here.  With this scheme we
 * are able to turn on any portion of the debugging with very little overhead
 * This appears to be better than similar schemes that test if
 * remote_debug <= some-level,  because they force the software into having
 * more and more debug software run as the number gets higher.
 */
extern long	remote_debug;

#define debug0	(!(remote_debug&0x00001)) ? 0:rmt_debug /* exec info */
#define debug1	(!(remote_debug&0x00002)) ? 0:rmt_debug /* startup activity */
#define debug2	(!(remote_debug&0x00004)) ? 0:rmt_debug /* rmt_copypath() */
#define debug3	(!(remote_debug&0x00008)) ? 0:rmt_debug /* unused */
#define debug4	(!(remote_debug&0x00010)) ? 0:rmt_debug /* remote_fork */
#define debug5	(!(remote_debug&0x00020)) ? 0:rmt_debug /* remote[on|off] */
#define debug6	(!(remote_debug&0x00040)) ? 0:rmt_debug /* file descriptors */
#define debug7	(!(remote_debug&0x00080)) ? 0:rmt_debug /* isremote() */
#define debug8	(!(remote_debug&0x00100)) ? 0:rmt_debug /* file remoteness */
#define debug9	(!(remote_debug&0x00200)) ? 0:rmt_debug /* connection startup */
#define debug10	(!(remote_debug&0x00400)) ? 0:rmt_debug /* msg setup */
#define debug11	(!(remote_debug&0x00800)) ? 0:rmt_debug /* msg exceptions */
#define debug12	(!(remote_debug&0x01000)) ? 0:rmt_debug /* shutdowns */
#define debug13	(!(remote_debug&0x02000)) ? 0:rmt_debug /* path translation */
#define debug14	(!(remote_debug&0x04000)) ? 0:rmt_debug /* chdir() activity */
#define debug15	(!(remote_debug&0x08000)) ? 0:rmt_debug /* msg content */
#define debug16	(!(remote_debug&0x10000)) ? 0:rmt_debug /* msg data content */
#define debug17	(!(remote_debug&0x20000)) ? 0:rmt_debug /* unused */
#define debug18	(!(remote_debug&0x40000)) ? 0:rmt_debug /* unused */
#define debug19	(!(remote_debug&0x80000)) ? 0:rmt_debug /* unused */

#endif RFSDEBUG

/*
 * Flags for file structures and proc structures.  These should really be
 * in file.h and proc.h, but are here for now.  Note that you must have
 * the fix in ino_close() if DTYPE_REMOTE is defined as 3.
 */
#define		DTYPE_REMOTE		3  /* file.h: remote file descriptor */
#ifdef pyr
#define		SREMOTE		0x00080000 /* proc.h: activity has occured */
#define		SNOREMOTE	0x80000000 /* proc.h: disallow remote access */
#define		FREMOTE		04000	   /* file.h: this is a remote file */
#endif pyr
#if vax || magnolia || P4400
#define		SREMOTE		0x08000000 /* proc.h: activity has occured */
#define		SNOREMOTE	0x10000000 /* proc.h: disallow remote access */
#define		FREMOTE		08000	   /* file.h: this is a remote file */
#endif vax || magnolia || P4400

/*
 * Defines for the name server.
 */
#define		server_alive(p)					\
			((p) 					\
			&& (p)->p_stat != NULL			\
			&& remote_ns.rn_pid == (p)->p_pid)

#define		NM_SERVER	0	/* register as name server */
#define		NM_WHATNAME	1	/* what name does the kernel need? */
#define		NM_NAMEIS	2	/* the name is... */
#define		NM_DEBUG	3	/* turn on debugging */

/*
 * Some manifest constants.
 */
#define		TRUE		1
#define		FALSE		0
#define		REMOTE_FS_SERVER	"remotefs"
#define		R_MAXSYS	NREMOTE	/* defined in param.h */
#define		R_MNTPATHLEN	30
#define		R_MAXMSG	((MAXPATHLEN * 2) + MLEN)
#define		R_RETRY		(60*5)	/* retry time for connections */

/*
 * State of the data being sent.
 */
#define		R_NOTHINGSENT	1
#define		R_DATANOTSENT	2
#define		R_MSGNOTRED	3
#define		R_DATANOTRED	4

/*
 * internal flags passed to rmt_msgfin() and rmt_datafin().
 */
#define		RFLG_DATAV	0x1	/* data to remotemsg() is a vector */
#define		RFLG_RD		0x2	/* data to remotemsg() to be read */
#define		RFLG_WR		0x4	/* data to remotemsg() to be written */
#define		RFLG_INFO	0x8	/* send message only (don't receive */

/*
 * known indices of data in the message.
 */
#define		R_PATHOFF	3	/* to server (Offset to 2nd path) */
#define		R_PATHSTART	4	/* to server start of actual path */
#define		R_DATA		2	/* to server */
#define		R_RETVAL	0	/* from server */
#define		R_EXECUID	1	/* from server (exec setuid) */
#define		R_EXECGID	2	/* from server (exec setgid) */
#define		R_EXECREADONLY	3	/* from server (exec text read-only) */
#define		R_EXECDATA	4	/* from server (start of exec info) */

/*
 * Maximum size for the message arg data in long words.  NOT LONGWORDS!!  This
 * should be the largest of the following three sizes among all machines
 * that will use the remote file system:
 *
 *	sizeof(struct exec) * sizeof(long) + R_EXECDATA * sizeof(long)
 *	(# of stat struct members + 1) * sizeof(long)
 */
#define		R_MAXARGS	18

/*
 * Maximum and minimum message length in bytes that we will ever need on a
 * receive (big enough for a stat(), lstat() or fstat(), and small enough for
 * a return with no other values.
 */
#define		R_MINRMSG	(sizeof(struct message)-R_MAXARGS*sizeof(long))
#define		R_MAXRMSG	(sizeof(struct message))

/*
 * The maximum number of mbufs that we need to send a message with
 * two paths, each 1024 characters long.  This is used only in the user
 * level implementation.
 */
#define		R_MAXMBUFS	(R_MINRMSG+(MAXPATHLEN*2))/MLEN

/*
 * Here, we describe incomming and outgoing messages.  Note that the number
 * of cells in m_args is only for incomming messages.  An outgoing message
 * may consume much more.
 */
struct message {
	long	m_totlen;	/* length of total message (including data) */
	short	m_uid;		/* uid of client */
	short	m_pid;		/* pid of client */
	short	m_hdlen;	/* length of header (excluding data) */
	short	m_syscall;	/* syscall number */
#define		m_errno m_syscall /* errno (for server) */
	long	m_args[R_MAXARGS];/* remaining arguments or return values */
};

/*
 * This structure describes the kernel information kept for each
 * connection to a remote server.
 */
struct remoteinfo {
	char	r_mntpath[ R_MNTPATHLEN ];	/* path of mount point */
	u_char		r_close:1;	/* True if connection to be closed */
	u_char		r_received:1;	/* True if an incomming msg in r_msg */
	u_char		r_failed:1;	/* connection failed */
	u_char		r_opening:1;	/* connection in process of opening */
	u_char		r_refcnt;	/* a reference count of active use */
	short		r_sender;	/* owner of outgoing data */
	short		r_recver;	/* owner of incomming data */
	u_short		r_users;	/* count of users using this */
	u_short		r_nfile;	/* count of open files */
	u_short		r_nchdir;	/* count of chdir() to this host */
	struct mbuf	*r_name;	/* socket address of remote host */
	struct inode 	*r_mntpt;	/* inode of mount point */
					/* if null, then global */
	struct socket	*r_sock;	/* socket with active connection */
#define	r_age		r_msg.m_totlen	/* used for cacheing name lookups */
#define	r_openerr	r_msg.m_errno	/* used to relay connection errors */
	struct message	r_msg;		/* incomming message */
};

typedef int			(*func)();

/*
 * This describes all info associated with each syscall.  Note that while
 * the flag information is available, most syscalls don't reference it
 * because they have the flag hard coded in-line.  It exists for the
 * sake of routines like read and write which cannot hard code the flags
 * very easily.  The follow flag is useful only for system calls that involve
 * pathnames.  The before entry is to tell syscall whether to try to
 * call the remote syscall routine before calling the real system call.
 * The size here make each table entry a
 * power of 2 in size (16 bytes) which the compiler can make faster code for.
 */
struct syscalls {
	func	sys_gen;
	func	sys_spec;
	long	sys_flag;
	short	sys_follow;
	short	sys_before;
};
typedef struct syscalls		syscalls;

/*
 * This structure simply describes the process willing to act as a
 * name server for the remote file system, and the information passing
 * to and from it.
 */
struct nameserver {
	struct proc	*rn_proc;	/* process registered as nameserver */
	struct mbuf	*rn_name;	/* input from name server */
	char		*rn_path;	/* path to translate */
	short		rn_pathlen;	/* length of path to translate */
	short		rn_pid;		/* pid of process (for uniqueness) */
};

/*
 * System calls
 * Note that these have nothing to do with either the vax or magnolia idea
 * of the system calls.  They are simply an index into the systems calls
 * that we are concerned with.
 */
#define	RSYS_fork	0
#define	RSYS_read	1
#define	RSYS_write	2
#define	RSYS_open	3
#define	RSYS_close	4
#define	RSYS_creat	5
#define	RSYS_link	6
#define	RSYS_unlink	7
#define	RSYS_chdir	8
#define	RSYS_mknod	9
#define	RSYS_chmod	10
#define	RSYS_chown	11
#define	RSYS_stat	12
#define	RSYS_lseek	13
#define	RSYS_access	14
#define	RSYS_lstat	15
#define	RSYS_dup	16
#define	RSYS_ioctl	17
#define	RSYS_symlink	18
#define	RSYS_readlink	19
#define	RSYS_fstat	20
#define	RSYS_dup2	21
#define	RSYS_fcntl	22
#define	RSYS_fsync	23
#define	RSYS_readv	24
#define	RSYS_writev	25
#define	RSYS_fchown	26
#define	RSYS_fchmod	27
#define	RSYS_rename	28
#define	RSYS_truncate	29
#define	RSYS_ftruncate	30
#define	RSYS_flock	31
#define	RSYS_mkdir	32
#define	RSYS_rmdir	33
#define	RSYS_utimes	34
#define	RSYS_exit	35
#define	RSYS_vfork	36
#define	RSYS_execinfo	37
#define	RSYS_execread	38
#define RSYS_execve	39
#define	RSYS_nosys	40
#define	RSYS_qlseek	41

/*
 * This macro fills in some of the information needed on every transfer
 * and returns the byte (not longword) offset of the next free byte.
 */
#define introduce(buf, sysnum)						\
				((buf)->m_pid = htons(u.u_procp->p_pid),\
				 (buf)->m_uid = htons(u.u_uid),		\
				 (buf)->m_syscall = htons(sysnum),	\
				 (R_MINRMSG))
#define introduce_1extra(buf, sysnum, x1)				\
				((buf)->m_pid = htons(u.u_procp->p_pid),\
				 (buf)->m_uid = htons(u.u_uid),		\
				 (buf)->m_syscall = htons(sysnum),	\
				 (buf)->m_args[0] = htonl(x1),		\
				 (R_MINRMSG + sizeof(long)))
#define introduce_2extra(buf, sysnum, x1, x2)				\
				((buf)->m_pid = htons(u.u_procp->p_pid),\
				 (buf)->m_uid = htons(u.u_uid),		\
				 (buf)->m_syscall = htons(sysnum),	\
				 (buf)->m_args[0] = htonl(x1),		\
				 (buf)->m_args[1] = htonl(x2),		\
				 (R_MINRMSG + 2*sizeof(long)))
/*
 * This macro defines whether a host is being used or not.
 * The rmtclearhosts() and rmtcopyhosts() are for expansion if someone
 * wants to use more than 32 hosts.
 */
#define	rmthostused(rsys)		(u.u_rmtsys  &  (1<<rsys))
#define	rmtusehost(rsys)		(u.u_rmtsys  |= (1<<rsys))
#define	rmtunusehost(rsys)		(u.u_rmtsys  &= ~(1<<rsys))
#define	rmtclearhosts()			(u.u_rmtsys   =  0)
#define	rmtcopyhosts(dest,hosts)	(dest         =  hosts)

#ifdef pyr
/*
 * Pyramid changed the name on ctob and btoc
 **/
#define	ctob(x)	ptob(x)
#define	btoc(x)	btop(x)
#endif
