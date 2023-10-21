
/* kb.h -*-update-version-*-
** HFVR VERSION=Fri May 23 14:46:26 1986
*/

#include <stdio.h>
#include <ctype.h>

typedef enum { EOFsym, FOUNDsym, NOTFOUNDsym } SYM;

#define FALSE	0
#define TRUE	1
typedef int  BOOL;

#define SIZE	20	/* max length of keywords */

#define	TSIZE 128
typedef int	KEYTAB[TSIZE];	/* for fsindex */

/* 3-address codes */
typedef enum {	RFIcode,	/* return (*arg1) */
				/* Return From Interpreter */
		IFTcode,	/* if *arg1 then val[arg2]=true, goto arg3 */
		IFFcode,	/* if !*arg1 then val[arg2]=false,goto arg3 */
		NOTcode,	/* val = NOT(*arg1) */
		KEYcode,	/* val = findkey(key) */
		COPcode,	/* val = *arg1 */
		CITcode,	/* val = (*arg1 ? *arg1 : findkey(key) ) */
				/* Copy If True */
		CIFcode,	/* val = (*arg1 ? findkey(key) : *arg1 ) */
				/* Copy If False */
	     } CODEsym;

typedef struct {	BOOL	*arg1;	/* ptr to argument 1 */
			BOOL	*arg2;	/* ptr to argument 2 */
			BOOL	*arg3;	/* ptr to argument 3 */
			int	count;	/* to indicate if used */
			BOOL	value;  /* result of this 3-address code */
			char	key[SIZE];
			KEYTAB	*table;	/* ptr to fsindex table */
			CODEsym code;	/* which code it is */
	       } CODE;

/* usage of CODE by the different codes
**
** CODE	arg1		arg2		arg3		count	key	value
** ----	------------	-------------	------------	-----	---	-----
** IFT	&val[pc-1]	&val[toassign]	&ins[togoto]	
** IFF	&val[pc-1]	&val[toassign]	&ins[togoto]	
** KEY							count	key	T/F
** COP	&val[copfrom]	&count[copfrom]				key	T/F
** CIT	&val[copfrom]	&count[copfrom]				key	T/F
** CIF	&val[copfrom]	&count[copfrom]				key	T/F
** RFI	&val[pc-1]
*/

extern void initfsindex();
extern char *fsindex();
extern char *sindex();
extern dflag;
