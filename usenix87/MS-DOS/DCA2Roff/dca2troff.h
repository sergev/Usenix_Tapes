#include <stdio.h>

extern int in_fcr;		/* if CRE and in_fcr, then required return */
extern int in_it;		/* in an indent tab */

extern int sf_length;		/* length of structured field */
extern int sf_class;		/* class of structured field */
extern int sf_type;		/* type of structured field */
extern int sf_format;		/* format of structured field */

extern int mb_class;		/* class of a multi byte command */
extern int mb_count;		/* size of a multi byte command */
extern int mb_type;		/* type of a multi byte command */

extern int blpt;		/* pointer into output buffer */
extern char bufline[];		/* output buffer */

extern int sf_incnt;		/* how many chars have we read in this sf */

extern char ctemp;		/* some temp regs */
extern char dtemp;		/* some temp regs */
extern char etemp;		/* some temp regs */
extern char ftemp;		/* some temp regs */
extern char gtemp;		/* some temp regs */
extern char htemp;		/* some temp regs */

extern int itemp;		/* some temp regs */
extern int jtemp;		/* some temp regs */
extern int ktemp;		/* some temp regs */
extern int ltemp;		/* some temp regs */
extern int mtemp;		/* some temp regs */
extern int ntemp;		/* some temp regs */

extern char tline[];		/* a temp buffer */
