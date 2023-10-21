extern tekbuf[];
extern struct{
	char ispeed, ospeed;
	char e,k;
	int mode;
	}tty;
closevt(){
	closepl();
}
closepl(){
	putch(015);
	fflush(tekbuf);
	gtty(tekbuf[0], &tty);
	tty.mode = tty.mode | 04; 	/* turn lc translation back on */
	stty (tekbuf[0], &tty);
	signal(2,0);	/* enable interrrupts */
}
