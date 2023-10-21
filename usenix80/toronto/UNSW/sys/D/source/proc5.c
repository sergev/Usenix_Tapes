/* other routines */

#include "proc.h"

getuser(uid)
int	uid;
{
	static struct pwent pe;

	pe.pw_uid=uid;
	if(getpwlog(&pe, wkbuf, sizeof wkbuf) <= 0)
		return("unknown");
	return(wkbuf);
}

bomb()
{
	/* signal handler
	 *
	 * at present just flush with message
	 *
	 */
	crash("unexpected termination. output flushed");
}

ttydecode(ttyp)
int	*ttyp;
{
	printf("ttydecode was passed %o\n", ttyp);
	printf("this routine is not yet implemented\n\n");
}

idecode(adr)
char	*adr;
{
	printf("idecode passd %o\n", adr);
	printf("this routine not yet implemented\n");
}

filedecode(adr)
char	*adr;
{
	printf("filedecode passed %o\n", adr);
	printf("this routine not yet implemented\n");
}

textdecode(tp)
unsigned tp;
{
/*
 *	decode the contents of the text stucture pointed to by tp
 */

	register int	q;
	int	te;
	struct symbol	*sp;

	printf(T_PTAB, "Text structure____ _________\n\n");
	if(capfflg)
		odump(tp, sizeof text[0]);

	sp = albin("_text\0\0\0");
	te = (tp - sp->s_symval) / sizeof text[0];
	if((te < 0) || (te >= NTEXT))
		{
		printf(T_PTAB, "\"p_textp\" was %u (%o) and bounds are %u (%o) to %u (%o)\n",
			tp, tp, sp->s_symval, sp->s_symval,
			(sp->s_symval + sizeof text[0] * NTEXT),
			(sp->s_symval + sizeof text[0] * NTEXT));
		warn("text pointer out of range");
		}
		printf(T_PTAB, "This structure is element %d of the text array\n",
			te);

	DEBUG
		printf("seek to text array entry");
	seek(corefd, tp, 0);
	DEBUG
		printf("successfully and read it into text\n");
	if(read(corefd, text, sizeof text[0]) < 0)
		crash("cannont read text area");

	if(text[0].x_daddr)
		{
		/* is on disc */
		if(infree(swapmap, (text[0].x_size + 7) >> 3, text[0].x_daddr))
			warn("text area in free swap space");
		}
	if(text[0].x_ccount)
		{
		/* in core */
		if(infree(coremap, text[0].x_size, text[0].x_caddr))
			warn("text area in free core");
		}

#ifdef	SHARED_DATA | LOWER_TEXT_SWAPS
	printf(T_PTAB, "\"x_flag\" says text area was ");
	if(text[0].x_flag)
		{
		/* there is something here */
		for(q=0; q<NOTFLAG; q++)
			{
			if((text[0].x_flag >> q) &01)
				{
				printf("%s", tflag[q]);
				text[0].x_flag =& ~(1<<q);
				}
			}
		printf("\n");
		}
	  else
		{
		printf("normal;\n");
		}
	if(text[0].x_flag)
		{
		printf(T_PTAB, "\"x_flag\" is %u (%o)\n",
			text[0].x_flag, text[0].x_flag);
		warn("unknown text area flag");
		}
#endif	SHARED_DATA | LOWER_TEXT_SWAPS

	printf(T_PTAB, "Disc address \"x_daddr\" is %u (%o), and original inode of prototype \"x_iptr\" is at %u (%o)\n",
		text[0].x_daddr, text[0].x_daddr, text[0].x_iptr, text[0].x_iptr);
	printf(T_PTAB, "There are \"x_count\" %d (%o) references to the area of size \"x_size\" %d (%o)\n",
		text[0].x_count, text[0].x_count, text[0].x_size, text[0].x_size);
	printf(T_PTAB, "The core address \"x_caddr\" is %u (%o), and there are \"x_ccount\" %d (%o) loaded references\n",
		text[0].x_caddr, text[0].x_caddr, text[0].x_ccount, text[0].x_ccount);
#ifdef	SHARED_DATA
	printf(T_PTAB, "Shared data inode is at \"x_spid\" %u (%o)\n",
		text[0].x_spid, text[0].x_spid);

#endif	SHARED_DATA

	printf("\n");

}

treedecode(ptr)
int	ptr;
{
	printf("treedecode passed %o\nnot yet implemented\n\n", ptr);
}

getmap()
{
	/*
	 *	read in the coremap and swapmap
	 */

register struct symbol	*rp;
register unsigned lb;
register struct map	*mp;
int	i;

	DEBUG
		printf("into getmap ok\n");

seek(corefd, (rp=albin("_coremap"))->s_symval, 0);
if(read(corefd, coremap, sizeof coremap) != sizeof coremap)
	crash("cant read coremap");

	DEBUG
		{
		for(i=0; i<200; i++)
			printf("coremap[%d] = %o\n",i, coremap[i]);
		}

seek(corefd, (rp=albin("_swapmap"))->s_symval, 0);
if(read(corefd, swapmap, sizeof swapmap) != sizeof swapmap)
	crash("cant read swapmap");

	DEBUG
		{
		for(i=0; i<400; i++)
			printf("swapmap[%d] = %o\n",i, swapmap[i]);
		}

/*
 *	if necessary output maps
 */

if(capcflg)
	{
	/* coremap */
	DEBUG
		printf("output coremap\n");
	printf("\n\nCoremap_______ values *64 bytes\n\n");
	printf("%10tArea%50tDisposition\n\n");

	for(lb = 0, mp = coremap; mp->m_size; mp++)
		{
		if(lb != mp->m_addr)
			printf("%7o - %7o%50tallocated\n",
				lb, mp->m_addr);

		printf("%7o - %7o%50tfree\n",
			mp->m_addr, mp->m_addr + mp->m_size);

		lb = mp->m_addr + mp->m_size;
		}
	if(lb != MAXMEM)
		printf("%7o - %7o%50tallocated\n",
			lb, MAXMEM);
	}

if(capsflg)
	{
	/* swapmap */
	DEBUG
		printf("output swapmap\n");
	printf("\n\nSwapmap_______ values *64 bytes\n\n");
	printf("%10tArea%50tDisposition\n\n");

	for(lb = MINSWAP, mp = swapmap; mp->m_size; mp++)
		{
		if(lb != mp->m_addr)
			printf("%7o - %7o%50tallocated\n",
				lb, mp->m_addr);

		printf("%7o - %7o%50tfree\n",
			mp->m_addr, mp->m_addr + mp->m_size);

		lb = mp->m_addr + mp->m_size;
		}
	if(lb != MAXSWAP)
		printf("%7o - %7o%50tallocated\n",
			lb, MAXSWAP);
	}

}

infree(mp, size, addr)
unsigned size, addr;
struct map	*mp;
{
	register struct map	*bp;

	/* find bp: bp[-1].m_addr <= addr < bp[0].m_addr */

	for(bp=mp; bp->m_size && bp->m_addr <= addr; bp++);

	if( bp > mp && (bp-1)->m_addr + (bp-1)->m_size > addr)
		return(1);
	if(bp->m_size && bp->m_addr < addr + size)
		return(1);
	return(0);
}

putregs()
{
printf("\n\nLow Core Words and Registers are\n\n");
printf("word0 = %7u (%7o)\n\n", regs.r_word0, regs.r_word0);
printf("word1 = %7u (%7o)\n\n", regs.r_word1, regs.r_word1);
printf("r0    = %7u (%7o)\n\n", regs.r_reg[0], regs.r_reg[0]);
printf("r1    = %7u (%7o)\n\n", regs.r_reg[1], regs.r_reg[1]);
printf("r2    = %7u (%7o)\n\n", regs.r_reg[2], regs.r_reg[2]);
printf("r3    = %7u (%7o)\n\n", regs.r_reg[3], regs.r_reg[3]);
printf("r4    = %7u (%7o)\n\n", regs.r_reg[4], regs.r_reg[4]);
printf("r5    = %7u (%7o)\n\n", regs.r_reg[5], regs.r_reg[5]);
printf("sp    = %7u (%7o)\n\n", regs.r_reg[6], regs.r_reg[6]);
printf("pc    = %7u (%7o)\n\n", regs.r_reg[7], regs.r_reg[7]);
printf("kdsa6 = %7u (%7o)\n\n", regs.r_kisa6, regs.r_kisa6);
#ifdef	BIG_UNIX
printf("kisa5 = %7u (%7o)\n\n", regs.r_kisa5, regs.r_kisa5);
#endif	BIG_UNIX
}
