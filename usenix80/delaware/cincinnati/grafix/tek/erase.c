extern tekbuf[];

extern int ohiy;
extern int ohix;
extern int oloy;
extern int plmode;
erase(){
	int i;
		putch(033);
		putch(014);
		fflush(tekbuf);
		ohiy= -1;
		ohix = -1;
		oloy = -1;
		plmode = 0;
		return;
}
