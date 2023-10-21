#

/*
*/

#include "../xlr.h"
#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../userx.h"
#include "../proc.h"
#include "../procx.h"
#include "../reg.h"
#include "../inode.h"
#include "../inodex.h"
#include "../file.h"
#include "../filex.h"
#include "../seg.h"

/*
 * routines to implement file locking at the incore inode level
 * the system has two file locking modes, subscriber and system-wide.
 * subscriber locking is on a voluntary basis, where as system-wide
 * is not.  To avoid deadly embraces waiting for processes to free
 * files, the following scheme is employed:  if the process wishing
 * to lock a given file has no other files, he sleeps on the lock.
 * Otherwise he gets an error return and must do his own sleep; this
 * provides a safe way out when A has B and wants C while D has C and 
 * wants B.
 */

/*
 * entry for xlock and xrel routines
 */

xlocke()
{
	xlr(ASSIGNF);
}

xrel()
{
	xlr(XRELS);
}
/*
 * common code for xlock and xrel.  There are two levels of lock.
 * If suser calls xlock with -1, a system wide lock mode is set up.
 * If suser calls xrel with -1, either the system wide lock is released
 * or all users are unlocked.
 */

xlr(mode)
{
	register *fp;
	register struct inode *ip;


	fp = getf(u.u_ar0[R0]);
	if (fp == NULL)
		if(u.u_ar0[R0] < 0 && suser())	{
			if(mode & ASSIGNF)	{
				filemode = XLOCKED;
/*
				printf("System lock mode.\n");
*/
				return;
			} else	{
				if (filemode & XLOCKED)	{
					filemode = 0;
/*
					printf("Release system lock.\n");
*/
					return;
				} else	{
					xlrfree(XALL,0,0);
/*
					printf("Free all users.\n");
*/
					return;
				}
			}
		} else	{
			u.u_error = EIFA;
			return;
		}

	ip = fp->f_inode;

	if (ip->i_flag & IOWNED)	{
		if (mode & ASSIGNF)	{
			if (u.u_procp->p_inodes != NULL)	{
				u.u_error = ETMF;
				return;
			} else	{
				while(ip->i_flag & IOWNED)
					sleep(&ip->i_nlink,PWAIT);
				goto setown;
			}
		} else
			if (ip->i_owner == u.u_procp->p_pid)
				xlrfree(XSINGLE,u.u_procp,ip);
			else
				goto error;
	} else	{
		if (mode & ASSIGNF)
			goto setown;
		else
			goto error;
	}
	return;

setown:
	ip->i_flag =| IOWNED;
	ip->i_nxtown = u.u_procp->p_inodes;
	u.u_procp->p_inodes = ip;
	ip->i_owner = u.u_procp->p_pid;
/*
	printf("inodep %o is owned by %o.\n",ip,ip->i_owner);
*/
	return;

error:
	u.u_error = EIFA;
}


xlrfree(mode,procp,inodep)
struct proc *procp;
struct inode *inodep;
{
	register struct inode *ip,*tp;
	register struct proc *pp;

	switch(mode)	{

		case XSINGLE:
			pp = procp;
			ip = pp->p_inodes;
			tp = pp;
			while(ip != inodep && ip != NULL)	{
				tp = ip;
				ip = ip->i_nxtown;
			}
			if(ip == NULL)
				printf("Lost inode lock, ip = %o\n",ip);
			if (pp == tp)
				pp->p_inodes = pp->p_inodes->i_nxtown;
			else
				tp->i_nxtown = ip->i_nxtown;
/*
			printf("inodep %o is released from %o.\n",ip,ip->i_owner);
*/
			ip->i_owner = -1;
			ip->i_nxtown = NULL;
			ip->i_flag =& ~IOWNED;
			wakeup(&ip->i_nlink);
			break;

		case XUSER:
			pp = procp;
			for(;;)	{
				if (pp->p_inodes == NULL)
					break;
				xlrfree(XSINGLE,pp,pp->p_inodes);
			}
			break;

		case XALL:
			for ( pp = &proc[0]; pp < &proc[NPROC]; pp++)
				if (pp->p_inodes != NULL)
					xlrfree(XUSER,pp,0);

	}
}
