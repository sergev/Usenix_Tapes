#

/*
 *	Update display of swapmap - use swapmap and text arrray read from kernel
 *	memory and proc structure from gprocs call to construct
 *	diagram.
 */

# include "gv.h"

extern struct{
	char		lname[8];
	int		stype;
	unsigned	value;
}nl[];

swap_map()
{
register struct proc *p2;
register struct text *p3;
unsigned	baddr,
		bsize;
char	c;
register i;
int sum;
extern int swap_user();

	for(i=0; i < 180; i++)
		screen.csize[i] = screen.cstat[i] = ' ';

	gprocs(proc);

	/*
	 * read text array
	 */
	seek(mem,nl[1].value,0);
	read(mem,text,sizeof(text));

	/*
	 * cycle through procs constructing swapped text segments
	 */
	for(p2=proc;p2 < &proc[NPROC];p2++)
		if((p2->p_stat) && !(p2->p_flag & SLOAD)){

			baddr = convert(p2->p_addr-swplo); /* block address */
			bsize = convert((p2->p_size >> 3) & 0177);
			swap_display(&baddr,&bsize,DATA);

			switch(p2->p_stat){

			case SSLEEP:
				c = 'S';
				break;

			case SZOMB:
				c = 'Z';
				break;

			case SWAIT:
				c = 'W';
				break;

			case SRUN:
				c = 'R';
				break;

			case SIDL:
				c = 'I';
				break;

			case SSTOP:
				c = 'T';
				break;
			}

			baddr += (bsize/2);
			screen.cstat[baddr] = c;
			if(screen.ppid[baddr] != p2->p_pid){	/* display name */
				name(p2->p_uid,baddr,&swap_user);
				screen.ppid[baddr] = p2->p_pid;
			}
		}

	/*
	 * search text array for swapped pure text segments and
	 * segments with sticky bit set (display these differently).
	 */
	for(p3=text;p3 < &text[NTEXT]; p3++)
		if(p3->x_iptr){ /* valid record */
			baddr = convert(p3->x_daddr-swplo);
			bsize = convert((p3->x_size >> 3)& 0177);
			swap_display(&baddr,&bsize,p3->x_count ? TEXT:SVTXT);
			baddr += (bsize/2);
			screen.cstat[baddr] = "0123456789ABCDEF"[p3->x_count];
			if(screen.ppid[baddr] != ROOT){
				swap_user(baddr,"root");
				screen.ppid[baddr] = ROOT;
			}
		}

	swap_sum(SADATA,SUDATA);

	/*
	 * write size display to screen
	 */
	curs(3,8,FD,ttype);
	rite(screen.csize,60);
	curs(3,15,FD,ttype);
	rite(screen.csize+60,60);
	curs(3,22,FD,ttype);
	rite(screen.csize+120,60);

	for(i=0; i < 180; i++)
		if(screen.cstat[i] == ' ' && screen.ppid[i] != -1){
			swap_user(i,"       ");
			screen.ppid[i] = -1;
		}
}

/*
 *	display swap area usage for segment starting at
 *	addr and of size size; use swplo to sort out disk
 *	addresses.
 */

swap_display(i_addr,i_size,type)
unsigned *i_addr,*i_size;
char type;
{
register unsigned addr, size;

	addr = *i_addr;
	size = *i_size;
	if(size <= 1){
		screen.csize[addr] = (type == DATA ? 'V' : 'X');
		return;
	}
	if(screen.csize[addr] != ' '){ /* overlap */
		addr++; size--;
		(*i_addr)++; (*i_size)--;
		if(size == 1){
			screen.csize[addr] = (type == DATA ? 'V' : 'X');
			return;
		}
	}
	switch(type){

	case DATA:
		screen.csize[addr++] = '<';
		size--;
		while(--size)
			screen.csize[addr++] = '-';
		screen.csize[addr] = '>';
		break;
	case TEXT:
		screen.csize[addr++] = '[';
		size--;
		while(--size)
			screen.csize[addr++] = '=';
		screen.csize[addr] = ']';
		break;
	case SVTXT:
		screen.csize[addr++] = '{';
		size--;
		while(--size)
			screen.csize[addr++] = '=';
		screen.csize[addr] = '}';
		break;
	default:
		write(2,"display\n",8);
	}
}

/*
 *	display name owner of swap space area over
 *	swap area
 *
 */

swap_user(place,name)
char *name;
int place;
{
register i;
register line;

	if(place < 60)
		line = 4;
	else if(place < 120){
		line = 11; place -= 60;
	}else{
		line = 17; place -= 120;
	}
	for(i=0;i < MNAME-1; i++){
		curs(3+place,i+line,FD,ttype);
		rite(name++,1);
	}
}

/*
 *	display summary of swap space usage
 */

swap_sum(availy,availx,usedy,usedx)
{
register unsigned free;
register struct map *p1;
	/*
	 * read swapmap
	 */
	seek(mem,nl[0].value,0);
	read(mem,swapmap,sizeof(swapmap));

	/*
	 * calculate free and used space from swapmap
	 */
	for(p1=swapmap,free=0;p1->m_size;p1++)
		free += p1->m_size;

	curs(availy,availx,FD,ttype);
	rite(itoa(free),3);

	curs(usedy,usedx,FD,ttype);
	rite(itoa(nswap-free),3);
}
