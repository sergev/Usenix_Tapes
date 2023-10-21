/*
 * the option definitions in this file are created from
 * the options structure definition in options.c
 * via the shell file makeoptions
 */

/* option flags */
#define	op_bool	0
#define op_int	1
#define op_spcl	2
#define op_str	4
#define op_init	8
#define op_trap 16

struct option{
	char	op_name;
	char	op_flag;
	int	op_value;
	char*	op_text;
	}options[];

/* option names */
#define	allflag 	   (options[0].op_value)
#define	sbflag 	   (options[1].op_value)
#define	crashpid 	   (options[2].op_value)
#define	dbflag 	   (options[3].op_value)
#define	indflag 	   (options[4].op_value)
#define	jsflag 	   (options[5].op_value)
#define	sflag 	   (options[6].op_value)
#define	leftbreak 	   (options[7].op_value)
#define	margflag 	   (options[8].op_value)
#define	owriteflag 	   (options[9].op_value)
#define	pagethresh 	  (options[10].op_value)
#define	rflag 	  (options[11].op_value)
#define	spellflag 	  (options[12].op_value)
#define	tabspace 	  (options[13].op_value)
#define	ucflag 	  (options[14].op_value)
#define	wflag 	  (options[15].op_value)
#define	xflag 	  (options[16].op_value)
#define	runfile 	  (options[17].op_value)
#define	yflag 	  (options[18].op_value)
#define	expsize 	  (options[19].op_value)
#define NOPTS 20
