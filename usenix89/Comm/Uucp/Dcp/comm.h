/*	declarations for comm.asm
**
**	compilation must use the Ze switch to enable the
**	"far" keyword for the small memory model
**
**	Robin Rohlicek   3/86
*/

void far select_port( int );	/*	select active port (1 or 2) */

void far save_com();		/*	save the interupt vectors */

void far restore_com();		/*	restore those vectors */

int far install_com();		/*	install our vectors */

void far open_com( 		/*	open com port */
	int, 	/* baud */
	int, 	/* 'M'odem or 'D'irect */
	int, 	/* Parity 'N'one, 'O'dd, 'E'ven, 'S'pace, 'M'ark */
	int, 	/* stop bits (1 or 2) */
	int);	/* Xon/Xoff 'E'nable, 'D'isable */

void far close_com();		/* 	close com port */

void far dtr_off();		/*	clear DTR */

void far dtr_on();		/*	set DTR */

long far r_count();		/*	receive counts */
	/* high word = total size of receive buffer */
	/* low word = number of pending chars */
#define r_count_size() ( (int) (r_count()>>16) )
#define r_count_pending() ( (int) r_count() )

int far receive_com();		/*	get one character */
	/* return -1 if none available */

long far s_count();		/*	send counts */
	/* high word = total size of transmit buffer */
	/* low word = number of bytes free in transmit buffer */
#define s_count_size() ( (int) (s_count()>>16) )
#define s_count_free() ( (int) s_count() )

void far send_com(int);		/* 	send a character */

void far send_local(int);	/*	simulate receive of char */

void far sendi_com(int);	/*	send immediately */

void far break_com();		/*	send a BREAK */

int * far com_errors();		/*	pointer to error counts 
					(in static area) */
#define COM_EOVFLOW 0	/* buffer overflows */
#define COM_EOVRUN  1	/* receive overruns */
#define COM_EBREAK  2	/* break chars */
#define COM_EFRAME  3	/* framing errors */
#define COM_EPARITY 4	/* parity errors */
#define COM_EXMIT   5	/* transmit erros */
#define COM_EDSR    6	/* data set ready errors */
#define COM_ECTS    7	/* clear to send errors */
#define COM_NERR    8	/* number of errors */
