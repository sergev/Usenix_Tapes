/*
 *	RT11 EMULATOR
 *	communication region layout
 *
 *	Daniel R. Strick
 *	April 16, 1979
 */

struct	_syscom	{
	wordp	c_start;	/* program starting address */
	wordp	c_stack;	/* initial stack pointer */
	word	c_jobst;	/* job status flags */
	wordp	c_uload;	/* USR load address */
	wordp	c_hmem;		/* user high memory address */
	char	c_error;	/* EMT error code */
	char	c_notused;
	wordp	c_rmon;		/* address of resident monitor */
	char	c_fchar;	/* terminal fill character */
	char	c_filno;	/* terminal fill count */
};
#define	syscom	(* (struct _syscom *) (char *) 040)

#define	C_NOTTW	0000100		/* inhibit tty wait */
#define	C_EHALT	0000200		/* halt on I/O error */
#define	C_CHAIN	0000400		/* load words 0500-0776 anyway */
#define	C_OVRLY	0001000		/* job uses linker overlay */
#define	C_TTRAW	0010000		/* special tty mode */
#define	C_REENT	0020000		/* job may be reentered */
#define	C_LOWCS	0040000		/* permit lower case terminal input */
#define	C_USRSW	0100000		/* USR swap bit (?) */
