# include       "sps.h"

/*
** MKTREE - Sort the needed processes by subtree and at the top by user.
** This procedure takes a list of processes (as returned by needed())
** and returnes a pointer to a sorted list.
*/
struct process  *mktree ( process, plist )

struct process                  *process ;
struct process                  *plist ;

{
	register struct process *p ;
	register struct process *pp ;
	register struct process *lp ;
	struct process          *op ;
	struct process          proot ;

	proot.pr_sibling = (struct process*)0 ;
	for ( p = plist ; p ; p = p->pr_plink )
	{
		if ( p->pr_pptr > &process[1] )
		{
			for ( pp = plist ; pp ; pp = pp->pr_plink )
			{
				if ( pp != p->pr_pptr )
					continue ;
				if ( lp = pp->pr_child )
				{       /* Does process have children ? */
					op = (struct process*)0 ;
					while (lp &&
					lp->pr_p.p_pid < p->pr_p.p_pid )
					{
						op = lp ;
						lp=lp->pr_sibling ;
					}
					if ( op )
					{
						p->pr_sibling = lp ;
						op->pr_sibling = p ;
						break ;
					}
				}       
				p->pr_sibling = lp ;
				pp->pr_child = p ;
				break ;
			}
			if ( pp )
				continue ;
		}
		/* We have a top level process, sort into top level list.
		   The top level is sorted firstly by user-id and then
		   by process-id. */
		lp = &proot ;
		pp = lp->pr_sibling ;
		while ( pp )
		{
			if ( p->pr_p.p_uid < pp->pr_p.p_uid )
				break ;
			if ( p->pr_p.p_uid == pp->pr_p.p_uid
			&& p->pr_p.p_pid < pp->pr_p.p_pid )
				break ;
			lp = pp, pp = pp->pr_sibling ;
		}
		p->pr_sibling = lp->pr_sibling ;
		lp->pr_sibling = p ;
	}
	return ( proot.pr_sibling ) ;
}
