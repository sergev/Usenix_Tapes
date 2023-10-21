#
/*
********Creating a Tape for This Program*********
**      @ASG,TXJ TAPE.,U9V,PPLxxx   . for 9-track 1600bpi tape
**      @UOM*LIBRARY.BLOCK,IOA <anyname>,TAPE.,120,720
**      @ADD,PL <your file>   . your UPPER and lower case file
**      @            . any control card (except @EOF)
*/

/*
 *      If no arg uses /dev/nrw_mt0, otherwise uses /dev/nrw_mt4 for input.
 *      Sends unpadded output (with newlines) to standard output.
 */

#define LOGREC  120
#define PHSREC  720

char    pad[1];
char    buff[PHSREC];
char    pad1[1];

int     tape;

main(argc, argv)
char    *argv[];
{
	register x, y;
	register char *p;
	int i;

	if(argc > 1) p = "/dev/nrw_rmt4";
	else p = "/dev/nrw_rmt0";

	if((tape = open(p, 0)) < 0) {
		write(2, "Cannot assign tape drive\n", 25);
		write(2, "Program stopped\n", 16);
		return;
	}

			/* for each physical record */
	while((i = read(tape, buff, PHSREC)) > 0) {
			/* Do for each card image */
		for(x = 0; x < PHSREC; x =+ LOGREC) {
			y = LOGREC;
			while(buff[x+(--y)] == ' ');  /* Truncate blanks */
			if(y<0) continue;
			buff[x+(++y)] = '\n';  /* Add a newline */
			write(1, &buff[x], y+1);  /* And write it out */
		}
	}

	if(i == -1) write(2, "READ ERROR\nProgram stopped\n", 27);
	else write(2, "EOF encountered\nNormal exit\n", 28);
	close(tape);
}
