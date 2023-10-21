/*		LOGGER - keep track of who is on	*/

main(argc,argv)
int argc ;
char *argv[] ;
{
	int tty,fi,delay,n,i,j ;
	int buf[256] ;
	char *cbuf ;
	struct {
		char name[8] ;
		char tty ;
		char pad1 ;
		int time[2] ;
		char pad2[2] ;
		} *p ;
	struct {
		char name[8] ;
		char onflag ;
		} log[128] ;


	delay = (argc==2) ? atoi(argv[1]) : 10 ;
	if ((fi = open("/etc/utmp",0)) < 0) {
		printf("cannot open /etc/utmp\n") ;
		exit() ;
		}

	for (tty = 0; tty<128; tty++) log[tty].onflag = 0 ;

	while (n = read(fi,buf,512)) {
		for (p = &buf; (n =- 16)>0; p++) {
			if (p->name[0]=='\0') continue ;
			tty = (p->tty) & 0177 ;
			for (i = 0; i<8; i++) {
				putchar((p->name[i] != '\0') ? p->name[i] : ' ') ;
				log[tty].name[i] = p->name[i] ;
				}
			printf("tty%c", p->tty) ;
			cbuf = ctime(p->time) ;
			for (i = 3; i<16; i++) putchar(cbuf[i]) ;
			putchar('\n') ;
			log[tty].onflag = 1 ;
			}
		}


	while (1) {
		sleep(delay) ;
		seek(fi,0,0) ;
		while (n = read(fi,buf,512)) {
			for (p = &buf; (n =- 16)>0; p++) {
				if (p->name[0] == '\0') continue ;
				tty = (p->tty) & 0177 ;
				j = 0 ;
				for (i = 0; i<8; i++) {
					if (p->name[i] != log[tty].name[i]) j = 1 ;
					}
				if (log[tty].onflag == 0 || j) {
					for (i = 0; i<8; i++) {
						putchar('\07') ;
						putchar(log[tty].name[i] = p->name[i]) ;
						}
					printf("logged in on tty%c\n", p->tty) ;
					}
				log[tty].onflag = 2 ;
				}
			}

		for (tty = 0; tty<128; tty++) {
			if (log[tty].onflag == 1) {
				for (i = 0; i<8; i++) putchar(log[tty].name[i]) ;
				printf("logged off tty%c\n", tty) ;
				log[tty].onflag = 0 ;
				}
			else if (log[tty].onflag == 2) log[tty].onflag = 1 ;
			}
		}

} /* end main */
