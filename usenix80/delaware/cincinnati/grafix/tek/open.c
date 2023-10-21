extern tekbuf[];
extern closepl();

struct {
	char ispeed, ospeed;
	char e,k;
	int mode;
	}tty;
openvt ()
{
	openpl();
}
openpl(){
	tekbuf[0] = dup(1);
	gtty(tekbuf[0], &tty);
	tty.mode = tty.mode & ~04 | 040000;	/* turn off lower case translation */
				/* also turn on form feed delay */
	stty(tekbuf[0], &tty);
	signal(2, &closepl);
}
