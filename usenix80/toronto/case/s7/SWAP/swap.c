/*
 *	Swapping area display program
 *
 *	Most of this code has been stolen from SS
 */

# include "defs.h"
# include "signal.h"

main(argc,argv)
char *argv[];
{
extern int	fix(),
		abort();

	signal(SIGINT,abort);
	signal(SIGQUIT,fix);
	init();
	date();
	for(;;)
		update();
}

/*
 *	initialize routine -
 *		setup namelist information
 *		find out terminal type
 *		clear screen and draw static information
 */
init()
{
register i;

	nlist(UNIX,nl);
	type = get_tty(ttyn(0));
	eras(type);
	draw();
	if((mfd = open(KMEM,READ)) < 0){
		eras(type);
		write(2,"can't open kmem\n",16);
		exit(1);
	}
	seek(mfd,nl[1].n_value,0);
	read(mfd,&swplo,sizeof(swplo));

	seek(mfd,nl[2].n_value,0);
	read(mfd,&nswap,sizeof(nswap));

	for(i=0; i<180; i++){
		screen.s_size[i] = ' ';
		screen.s_pid[i] = -1;
	}
}

/*
 *	cleanup screen routine, called on a ^B
 */
fix()
{
	signal(SIGQUIT,fix);
	eras(type);
	draw();
	date();
}

/*
 *	cleanup routine before exiting (exit on ^C)
 */
abort()
{
	eras(type);
	exit(0);
}

/*
 *	draw static information -
 *		time, date, and system header
 *		swap area scale factors
 *		limits on swapping area
 */
draw()
{
	curs(DTITLE,type);
	write(1,"Date: ",6);

	curs(TTITLE,type);
	write(1,"Time: ",6);

	getmesg();

	curs(STITLE,type);
	write(1,"\tFree:\t\tUsed:",13);
	
	curs(W1TITLE,type);
	write(1,"Who",3);
	
	curs(A1TITLE,type);
	printf(" 00********50********100********150*******200*******250*******300   Blocks");
	
	curs(S1TITLE,type);
	write(1,"Size",4);
	
	
	curs(W2TITLE,type);
	write(1,"Who",3);
	
	curs(A2TITLE,type);
	printf("301*******350*******400*******450********500*******550*******600    Blocks");
	
	curs(S2TITLE,type);
	write(1,"Size",4);
	

	curs(W3TITLE,type);
	write(1,"Who",3);

	curs(A3TITLE,type);
	printf("601*******650*******700*******750********800*******850*******900    Blocks");

	curs(S3TITLE,type);
	write(1,"Size",4);
}

/*
 *	print time of day in appropriate slot on screen
 */
print_time()
{
register char *pnt;
int	tvec[2];

	time(tvec);
	pnt = ctime(tvec);
	curs(TDATA,type);
	pnt += 11;
	write(1,pnt,8);
}

/*
 *	print date on appropriate slot on screen
 */
date()
{
register char *pnt;
int	tvec[2];

	time(tvec);
	pnt = ctime(tvec);
	curs(DDATA,type);
	write(1,pnt,10);
	write(1,",",1);
	pnt += 19;
	write(1,pnt,5);
}

/*
 *	given the terminal letter, find out what type of
 *	terminal it is; for Case system only recognize
 *	ADDS and BEEHIVE as usable for display
 */
get_tty(c)
char c;
{
int	fd,
	n,
	t;
register i;
char	buf[TTYSIZE];
static struct terminal term[] = {
	0, 0,
	"tektronics 4023", 15,
	"beehive", 7,
	"adds", 4,
	"adm3", 4
};

	if((fd = open(TTYS,READ)) < 0){
		write(2,"can't open ttys\n",16);
		exit(1);
	}
	n = read(fd, buf, sizeof(buf));
	for(i=1; i<n; i += 4)
		if(c == buf[i]) break;	/* found the tty */
	if(i == n){	/* no tty */
		write(2,"not a tty\n",10);
		exit(1);
	}

	t = buf[i+1]-'0';
	if((t != BEEHIVE)&&(t != ADDS)){
		write(2,"can't operate on ",16);
		write(2,term[t].t_name,term[t].t_cnt);
		write(2,"\n",1);
		exit(1);
	}

	if(t == ADDS) t = 3;	/* kluge */
	close(fd);
	return(t);
}

/*
 *	print system message as header
 */
getmesg()
{
int	fd,
	n;
char	buf[80];

	if((fd = open(SYSMESG,READ)) < 0)
		return;
	n = read(fd,buf,sizeof(buf));
	curs(MTITLE,type);
	write(1,buf,n-1);
	close(fd);
}
