#
/*
 *      chicken little -  announce the sky is falling
 *      routine to send message to all users that the system is going down
 *      we call the program sdown and it lives in /etc/sdown as
 *      well as /sys/sdown.  ksys uses this program
 *
 *      dlm 1.3 10-Nov-1977     fix to prevent cl from hanging around
 *                              waiting for hung terminals
 *                              This is especially true on pseudo ttys
 *                              which have not been opened,
 *                              but also other ttys which might hang
 *
 *                              I assume alarm clock interrupt
 *                              is available; otherwise no problems
 */
char id[] "~|^`cl.c\t1.3\n";

char *dmes "Time sharing is over -- please log off NOW!";
char *ttyn "/dev/tty?";
int ring; /* means the alarm clock rang */
#define SIGCLK  14  /* alarm clock interrupt */

main(narg,arg)
int narg;
char *arg[];
{
	char buf[3];
	char *mes;
	int lmes;
	int inf,outf;
	extern int ringring();

	chdir("/"); /* release any mounted disks */
	signal(SIGCLK,&ringring); /* setup alarm clock trap */
	mes = (narg < 2 ? dmes : arg[1]);
	lmes = 0;
	while (mes[lmes++]);
	inf = open("/etc/ttys",0);
	while (read(inf,buf,3) == 3) {
		ttyn[8] = buf[1];
		ring = 0; /* reset alarm flag */
		setexit();
		if(ring) goto nxt;
		alarm(8);  /* start timer */
		if (buf[0] == '1' && (outf = open(ttyn,1)) >= 0) {
			write(outf,"\n\r\n\07",4);
			write(outf,mes,lmes);
			write(outf,"\n\r\n\07",4);
			close(outf);
		}
		alarm(0); /* turn off alarm */
nxt:
		while (buf[2] != '\n') if (read(inf,&buf[2],1) != 1) goto syncit;
	}

syncit: 
	sync();
	close(inf);
}

ringring(){
	signal(SIGCLK,&ringring); /* setup alarm clock trap */
	alarm(0); /* turn off alarm */
	ring++;
#ifdef DEBUG
printf("Alarm clock rang\n");
#endif
	reset();
}
