#include "opt.h"
#define LIMIT 50
struct node *marked[LIMIT];	/* list of nodes marked by mark()	*/
struct node **markp &marked[0];	/* pointer to location in array		*/
/*
 *	Function:	brasw
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     This function looks for branch statements that need to be changed
 *	to jumps.  It assumes that any branch that branches over plus or
 *	minus 126 bytes of instructions should be changed to a jump.
 *	It tries to find a branch or jump within range of the current branch
 *	and make a bra to it, failing that it turns bra into jump and CBR
 *	into the reverse sense branch over a jump to the location.
 *
 */

brasw()
{
	register struct node *p, *p1;
	register t;
	int nj;
	extern char revbr[];

	t = 0;
    do
	{
	nj = 0;
	for (p=first.forw; p!=0; p = p->forw)
	    if (p->op > 0)
		{
		if (p->op <= BSS)
			p->refc = t;
		else if (p->op <= WAI)
			p->refc = ++t;
		else if (p->op <= CBR)
			p->refc = t = t + 2;
		else if (p->op <= TST)
			p->refc = t = t + 3;
		}
	for (p=first.forw; p!=0; p = p->forw)
		{
		if ((p->op== BRA || p->op==CBR) && ((p->ref != 0
		    && abs(p->ref->refc - p->refc) > 126)
		    || p->ref == 0))
			{
			for (p1 = first.forw; p1 != 0; p1 = p1->forw)
			   if (p1 != p && (p1->op == BRA || p1->op == JMP)
				&& p1->ref == p->ref
				&& abs(p1->refc - p->refc) < 126)
				{
				p->ref = p1 = insertl(p1);
				p->labno = p1->labno;
				p->code = 0;
				goto next;
				}
			if (p->op==BRA)
				{
				p->op = JMP;
				jmpbr--;
				}
			else
				{
				p->subop = revbr[p->subop];
				p1 = alloc(sizeof first);
				p1->combop = JMP;
				p1->labno = p->labno;
				p1->ref = p->ref;
				p1->code = p->code;
				p1->back = p;
				p->forw->back = p1;
				p1->forw = p->forw;
				p->forw = p1;
				p->ref = insertl(p1->forw);
				p->labno = p->ref->labno;
				p->code = 0;
				nskip--;
				}
			nj++;
		   next:;
			}
		}
	}
    while (nj != 0);
}
/*
 *	Function:	redcode()
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     Looks for register operations whose register is "dead" and
 *	removes them.  "dead" registers are never stored before reloadling.
 *	It calls prop to find cases where the value of a or b is
 *	known and the result of a test can be computed now.
 *
 */

int validx;		/* non-zero if x has been ldx index	*/

struct reginfo {
	int	rvalid;		/* 0 if value is not valid, != 0 otherwise */
	int	rvalue;		/* value in reg if rvalid != 0		*/
	struct node *load;	/* the node reg last loaded in		*/
	struct node *store;	/* the node reg last stored into	*/
	} reg[2];		/* reg[0] = areg, reg[1] = breg		*/

redcode()
{
register struct node *cnode;	/* the current node	*/
register int csubop;		/* the subop field	*/
register struct reginfo *creg;	/* the current reg info	*/
char *ccode;
int tval;

reg[0].rvalid = reg[0].load = reg[0].store = 0;
reg[1].rvalid = reg[1].load = reg[1].store = 0;

for(cnode = first.forw; cnode != 0; cnode = cnode->forw)
    {
    csubop = cnode->subop;
    ccode = cnode->code;
    if (csubop == BREG) creg = &reg[1];
    else if (csubop == AREG) creg = &reg[0];
    else creg = 0;

#ifdef DEBUG
printf("node %o, op %d, subop %d,",cnode, cnode->op, csubop);
if (creg != 0)
  printf("valid %d, value %d, store %o, load %o\n", creg->rvalid, creg->rvalue,
creg->store, creg->load);
else printf("\n");
#endif

    if (creg)	/* if reg is used, check for LDA's or CLR's following	*/
	{
	if (cnode->op != STA && cnode->op != PUL && cnode->op != PSH)
	    if (follow(cnode->forw, csubop))
		{
		unmark();
		rmnode(cnode);
		continue;
		}
	    else unmark();
	if (ccode && *ccode == '#' && creg )
	    {
	    if (cnode->op == LDA)
		{
		if ((tval = getcnum(ccode)) != 0)
		    if (creg->rvalid != 0 && creg->rvalue == tval)
			rmnode(cnode);
		    else
			{
			creg->rvalid = 1;
			creg->rvalue = tval;
			}
		else creg->rvalid = 0;
		}
	    }
	}
    switch(cnode->op)
	{
	case CLR:
		if (creg)
			if (creg->rvalid != 0 && creg->rvalue == 0)
			    rmnode(cnode);
			else
			    {
			    creg->rvalid = 1;
			    creg->rvalue = creg->load = creg->store = 0;
			    }
		break;
	case COM:
		if (creg)
			{
			if (creg->rvalid)
	    		    creg->rvalue = ~creg->rvalue;
			creg->load = creg->store = 0;
			}
		break;
	case STA:
		creg->store = cnode;
		break;
	case LDA:
		if (creg->store != 0)
		    if (equstr((creg->store)->code, ccode))
			{
			rmnode(cnode);
			break;
			}
		    else creg->store = 0;
		if (creg->load != 0)
		    if (equstr((creg->load)->code, ccode))
			{
			rmnode(cnode);
			break;
			}
		creg->load = cnode;
		if (*ccode != '#') creg->rvalid = 0;
		break;
	case TBA:
		creg->rvalid = reg[1].rvalid;
		creg->rvalue = reg[1].rvalue;
		creg->load = reg[1].load;
		creg->store = reg[1].store;
		break;
	case TAB:
		creg->rvalid = reg[0].rvalid;
		creg->rvalue = reg[0].rvalue;
		creg->load = reg[0].load;
		creg->store = reg[0].store;
		break;
	case TSX:
	case INX:
	case LDX:
	case DEX:	/* none of these affect current contents	*/
		reg[0].load = reg[0].store = reg[1].load = reg[1].store = 0;
		break;
	case DES:
	case INS:
	case LDS:
	case NOP:
	case PSH:
	case STS:
	case STX:
	case TXS:
	case CBR:
	case CMP: /* none of these change a or b contents or index reg	*/
		break;
	case TST:
		if (creg && creg->rvalid) prop(cnode, csubop, creg);
		break;
	case BRA:
		if (reg[0].rvalid) prop(cnode, AREG, &reg[0]);
		if (reg[1].rvalid) prop(cnode, BREG, &reg[1]);
		/* fall through to reset all reg info in default	*/
	default:
		if (creg)
		    creg->rvalid = creg->load = creg->store = 0;
		else
		    {
		    reg[0].rvalid = reg[0].store = reg[0].load = 0;
		    reg[1].rvalid = reg[1].store = reg[1].load = 0;
		    }
	}
    }
}

/*
 *	Function:	rmnode
 *
 *	Arguments:	a pointer to the node to be removed
 *
 *	Returns:	nothing
 *
 *	Description:
 *
 *	     Remove a node and increment redcod.
 */

rmnode(ap)
struct node *ap;
{

redop++;

nchange++;

ap->back->forw = ap->forw;
ap->forw->back = ap->back;
#ifdef DEBUG
printf("rmnode code %d, subop %d\n", ap->op, ap->subop);
#endif
}
/*
 *	Function:	getcnum
 *
 *	Arguments:	pointer to string
 *
 *	Returns:	value of string or 0 on error.
 *
 *	Description:
 *	     finds octal sign and handles octal, else lets getnum do
 *	decimal conversion.  Makes sure number ends with '\0'
 *
 */

getcnum(string)
char *string;
{
register char *p;
register int n;

p = string;

if (*p == '#') p++;

if(*p != '@') n = getnum(p);
else
	{
	n = 0;
	p++;
	while ('0' <= *p && *p <= '9')
		n = (n<<3) + *p++ - '0';
	if (*p != 0)
		n = 0;
	}
if (n & 0200) n = n | 0177400;	/* sign extension	*/

return(n);
}

/*
 *	Function:	follow
 *
 *	Arguments:	ptr to a node, and register used (AREG or BREG)
 *
 *	Returns:	1 if first op on regused is a load for all paths
 *			0 if not, or if CBR is encountered before a TST or CMP
 *
 *	Description:
 *	     Follows out all possible paths, making sure that the reg
 *	is not used before it is reloaded.  For conditional branches
 *	follows both paths, follows ref for BRA or JMP.
 *
 */

follow(aptr, reg)
register struct node *aptr;
register int reg;
{
int cmpflg, retval;
register char op;

cmpflg = 0;	/* mark that there hasn't been a cmp or tst yet for CBR */
retval = 1;	/* if we fall through to end then the reg is dead	*/

for(; aptr != 0; aptr = aptr->forw)
	{
	op = aptr->op;
	if (op == LABEL || op == DLABEL)
		if (aptr->subop != 0)
			{
			retval = 1;	/* already checked	*/
			break;
			}
		else mark(aptr);	/* set aptr->subop = 1	*/
	if (aptr->subop == reg)
		{
		if (op==LDA || op==CLR || op==TAB || op==TBA || op==PUL)
			{
			retval = 1;
			break;
			}
		else
			{
			retval = 0;
			break;
			}
		}
	else if (op == TAB || op == TBA || op == JSR || op == CBA)
		{
		retval = 0;
		break;
		}
	else if (op == CBR)
		if (cmpflg)
			if (aptr->forw && aptr->ref)	/* missing links? */
				{
				retval = follow(aptr->forw,reg) &
					 follow(aptr->ref,reg);
				break;
				}
			else
				{
				retval = 0;
				break;
				}
		else
			{
			retval = 0;
			break;
			}
	else if (op == BRA || op == JMP)
		if (aptr->ref)	/* link still there?	*/
			{
			retval = follow(aptr->ref, reg);
			break;
			}
		else
			{
			retval = 0;
			break;
			}
	if (op == CMP || op == TST) cmpflg++;
	else cmpflg = 0;
	}
return(retval);
}


/*
 *	Function:	zeroref
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Description:
 *
 *		Sets all the ref fields of LABEL's and DLABEL's to zero
 *	for use in determining when the index register is valid.
 *
 */
zeroref()
{
register struct node *p;

for(p = first.forw; p != 0 ; p = p->forw)
	if (p->op == LABEL || p->op == DLABEL)
		p->count = 0;
}

/*
 *	Function:	ckldx
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Description:
 *
 *		Marks labels as to how many branches, jumps or conditional
 *	branches reference the label with a valid index register. see rmldx().
 *
 */

ckldx()
{
register struct node *curnode, *cref;
register char *ccode;
int jflag;		/* zero if last instruction was a JMP or BRA	*/

validx = 0;
jflag = 1;
zeroref();

for(curnode = first.forw; curnode != 0 ; curnode = curnode->forw)
    {
    ccode = curnode->code;
    cref =  curnode->ref;
    switch(curnode->op)
	{
	case LDX:
		if (ccode)
			if (equstr(ccode, "index"))
				{
				validx = 1;
				break;
				}
	case TSX:
	case INX:
	case DEX:
		validx = 0;
		break;
	case LABEL:
	case DLABEL:
		if (curnode->count == curnode->refc)
			if ((jflag != 0 && validx != 0) || jflag == 0)
				validx = 1;
			else	validx = 0;
		else validx = 0;
		break;
	case CBR:
		if (validx)
			if (cref)
				cref->count =+ 1;
		break;
	case JMP:
	case BRA:
		if (validx)
			if (cref)
				cref->count =+ 1;
		validx = 0;
		break;
	}
    if (curnode->op == BRA || curnode->op == JMP)
	 jflag = 0;
    else jflag = 1;
    }
rmldx();
}

/*
 *	Function:	rmldx
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Description:
 *
 *		Removes the unnecessary ldx index's by using the info from
 *	ckldx() and checking on the validity of x.
 *
 */

rmldx()
{
register struct node *curnode;
register int jflag;	/* zero if previous inst was BRA or JMP		*/
register int op;	/* the operator type of the current node	*/

validx = 0;
jflag = 1;

for(curnode = first.forw; curnode != 0 ; curnode = curnode->forw)
	{
	op = curnode->op;	/* use register to hold op	*/
	if (op == LABEL)
		{
		if (curnode->count == curnode->refc)
		    if ((jflag != 0 && validx != 0) || jflag == 0)
			validx = 1;
		    else validx = 0;
		else validx = 0;
#ifdef DEBUG
printf("L%d, validx %d, jflag %d, ",curnode->labno,validx,jflag);
printf("count %d, refc %d\n",curnode->count, curnode->refc);
#endif
		}
	if (op == LDX)
		if (equstr(curnode->code, "index"))
			if (validx != 0) rmnode(curnode);
			else validx = 1;
		else validx = 0;
	if (op == TSX || op == DEX || op == INX)
		validx = 0;
	if (op == JMP || op == BRA)
		jflag = 0;
	else	jflag = 1;
	}

zeroref();
}

/*	Function:	mark
 *
 *	Argument:	pointer to a node
 *
 *	Returns:	nothing
 * Puts node on list of marked nodes and sets subop = 1.
 */
mark(aptr)
struct node *aptr;
{
    aptr->subop = 1;

    if (markp >= &marked[LIMIT])
	{
	write(2, "Optimizer: marked label list overflow\n", 37);
	abort();
	}
    *markp++ = aptr;
}

/*	Function:	unmark
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 * Goes through array of pointers to marked nodes and clears node->subop
 * and pointer in array.
 */

unmark()
{
    register struct node **remark;

    markp = remark = &marked[0];

    while (*remark != 0)
	{
	(*remark)->subop = 0;	/* clear subop	*/
	*remark++ = 0;		/* clear pointer array	*/
	}
}
/*
 *	Function:	abs
 *
 *	Argument:	integer
 *
 *	Returns:	absolute value of the integer
 */

abs(number)
int number;
{
return(number < 0 ? -number: number);
}
/*
 *	Function:	prop
 *
 *	Arguments:	pointer to a node, reg it uses, ptr to reginfo struct
 *
 *	Returns:	nothing
 *
 *	Description:
 *
 *		This procedure propagates known values of a or b through
 *	the code in hopes of precomputing conditional branches.  It will
 *	replace the BRA or TST and CBR with a BRA to the eventual destination.
 *	It keeps track of INS's to put the proper number in front of the
 *	bra if need be.
 *
 */
prop(aptr, reg, preg)
struct node *aptr;
int reg;
struct reginfo *preg;
{
int retval, inscnt;
register struct node *tptr, *next;

inscnt = retval = 0;

for (tptr = aptr; retval == 0 ; tptr = tptr->forw)
    {
    if (tptr->op == BRA)
	if (tptr->ref && inscnt == 0)
	    tptr = (tptr->ref)->back;	/* we"ll go forward in iteration */
	else retval++;
    else if (tptr->op == LABEL)
	if (tptr->subop != 0) retval++;
	else mark(tptr);	/* set tptr->subop to 1	*/
    else if (tptr->op == INS) inscnt++;
    else if (tptr->op == TST)
	{
	retval++;
	if (tptr->subop == reg)
	    {
	    tptr = tptr->forw;
	    if (tptr->op == CBR)
		{
		next = tptr->forw;	/* set up to follow */
		switch(tptr->subop)
		    {
		    case BEQ:
		    case BHI:
		    case BLS: if (preg->rvalue == 0) next = tptr->ref; break;
		    case BGE:
		    case BPL: if (preg->rvalue >= 0) next = tptr->ref; break;
		    case BGT: if (preg->rvalue > 0) next = tptr->ref; break;
		    case BLE: if (preg->rvalue <= 0) next = tptr->ref; break;
		    case BLT:
		    case BMI: if (preg->rvalue < 0) next = tptr->ref; break;
		    case BNE: if (preg->rvalue != 0) next = tptr->ref; break;
		    case BCC:
		    case BVC: next = tptr->ref; break;
		    case BCS:
		    case BVS:
		    default:	/* don't do anything	*/
			next = 0;
		    }
		if (next != 0)
		    {
		    next = insertl(next);
		    if (aptr->op == BRA && aptr->ref) decref(aptr->ref);
		    aptr->combop = BRA;
		    aptr->ref = next;
		    aptr->refc = 0;
		    aptr->code = 0;
		    aptr->labno = next->labno;
		    nchange++;
		    while (inscnt != 0)
			{
			insertn(aptr, INS);
			inscnt--;
			}
		    }
		}
	    }
	}
    else retval++;
    }
unmark();	/* clear all marked LABEL and DLABEL nodes	*/
}

/*
 *	Function:	insertn
 *
 *	Arguments:	ptr to a node, combined op of node to be inserted
 *
 *	Returns:	nothing
 *
 *	Inserts a node with the combined op given before the node specified.
 */

insertn(bptr, combop)
register struct node *bptr;
int combop;
{
register struct node *retptr;

retptr = alloc(sizeof first);

retptr->combop = combop;
retptr->labno = retptr->ref = retptr->code = retptr->refc = 0;
retptr->back = bptr->back;
retptr->forw = bptr;
bptr->back->forw = retptr;
bptr->back = retptr;
}
