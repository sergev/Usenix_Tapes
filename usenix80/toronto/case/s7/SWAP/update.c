#

/*
 *	Update display of swapmap - use swapmap and text arrray read from kernel
 *	memory and proc structure from gprocs call to construct
 *	diagram.
 */

# include "defs.h"

update()
{
register struct proc *p2;
register struct map *p1;
register struct text *p3;
unsigned	baddr,
		bsize;
register i;
int sum;

	gprocs(proc);

	/*
	 * read text array
	 */
	seek(mfd,nl[3].n_value,0);
	read(mfd,text,sizeof(text));

	/*
	 * read swapmap
	 */
	seek(mfd,nl[0].n_value,0);
	read(mfd,swapmap,sizeof(swapmap));

	/*
	 * cycle through procs constructing swapped text segments
	 */
	for(p2=proc;p2 < &proc[NPROC];p2++)
		if((p2->p_stat) && !(p2->p_flag & SLOAD)){
			baddr = convert(p2->p_addr-swplo); /* block address */
			bsize = convert((p2->p_size >> 3) & 0177);
			display(&baddr,&bsize,DATA);
			baddr += (bsize/2);
			if(screen.s_pid[baddr] != p2->p_pid){	/* display name */
				if(!*users[p2->p_uid])
					getuser(p2->p_uid);
				udisplay(users[p2->p_uid],baddr);
				screen.s_pid[baddr] = p2->p_pid;
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
			display(&baddr,&bsize,p3->x_count ? TEXT:SVTXT);
			baddr += (bsize/2);
			if(screen.s_pid[baddr] != 0){
				udisplay("root",baddr+(bsize/2));
				screen.s_pid[baddr] = 0;
			}
		}

	/*
	 * calculate free and used space from swapmap
	 */
	for(p1=swapmap,sum=0;p1->m_size;p1++)
		sum += p1->m_size;
	sdisplay(sum);

	/*
	 * write size display to screen
	 */
	curs(3,8,type);
	write(1,screen.s_size,60);
	curs(3,15,type);
	write(1,screen.s_size+59,60);
	curs(3,22,type);
	write(1,screen.s_size+119,60);

	/*
	 * reset name markers for next time
	 */
	for(i=0; i<180; i++)
		if((screen.s_pid[i] != -1) && (screen.s_size[i] != 'X')
		   && (screen.s_size[i] != 'V')){
			udisplay("     ",i);
			screen.s_pid[i] = -1;
		}
	/*
	 * update time
	 */
	print_time();

	/*
	 * clear out array for next time
	 */
	for(i=0;i<180;i++)
		screen.s_size[i] = ' ';
}

/*
 *	turn user id into user name
 *		if uid is not found in /etc/passwd put
 *		'?'s in place of name
 */

getuser(uid)
int uid;
{
char buf[80];
register char	*pbuf,
		*p;
register i;

	p = users[uid];
	if(getpw(uid,buf)){ /* unkown user */
		for(i=0;++i<5; *p++ = '?');
		*p = '\0';
	}else{
		for(pbuf=buf,i=0;*pbuf != ':' && ++i < 5;*p++ = *pbuf++);
		*p = '\0';
	}
}

/*
 *	display swap area usage for segment starting at
 *	addr and of size size; use swplo to sort out disk
 *	addresses.
 */

display(i_addr,i_size,type)
unsigned *i_addr,*i_size;
char type;
{
register unsigned addr, size;

	addr = *i_addr;
	size = *i_size;
	if(size <= 1){
		screen.s_size[addr] = (type == 1 ? 'V' : 'X');
		return;
	}
	if(screen.s_size[addr] != ' '){ /* overlap */
		addr++; size--;
		(*i_addr)++; (*i_size)--;
		if(size == 1){
			screen.s_size[addr] = (type == 1 ? 'V' : 'X');
			return;
		}
	}
	switch(type){

	case DATA:
		screen.s_size[addr++] = '<';
		size--;
		while(--size)
			screen.s_size[addr++] = '-';
		screen.s_size[addr] = '>';
		break;
	case TEXT:
		screen.s_size[addr++] = '[';
		size--;
		while(--size)
			screen.s_size[addr++] = '=';
		screen.s_size[addr] = ']';
		break;
	case SVTXT:
		screen.s_size[addr++] = '{';
		size--;
		while(--size)
			screen.s_size[addr++] = '=';
		screen.s_size[addr] = '}';
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

udisplay(name,place)
char *name;
int place;
{
register i;
register line;

	if(place < 61)
		line = 4;
	else if(place < 121){
		line = 11; place -= 60;
	}else{
		line = 17; place -= 120;
	}
	for(i=0;i < MNAME-1; i++){
		curs(3+place,i+line,type);
		write(1,name++,1);
	}
}

/*
 *	display summary of swap space usage
 */

sdisplay(free)
unsigned free;
{
	curs(SADATA,type);
	printf("%d",free);

	curs(SUDATA,type);
	printf("%d",nswap-free);
}
