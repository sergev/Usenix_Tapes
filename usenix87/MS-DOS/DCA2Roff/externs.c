
int in_fcr = 0;		/* if CRE and in_fcr, then required return */
int in_it = 0;		/* in an indent tab */

int sf_length = 0;	/* length of structured field */
int sf_class = 0;	/* class of structured field */
int sf_type = 0;	/* type of structured field */
int sf_format = 0;	/* format of structured field */

int mb_class = 0;	/* class of a multi byte command */
int mb_count = 0;	/* size of a multi byte command */
int mb_type = 0;	/* type of a multi byte command */

int blpt = 0;		/* pointer to output buffer */
char bufline[255];	 /* output buffer */

int sf_incnt = 0;	/* how many characters have we read in this sf */

char ctemp;		/* some temp regs */
char dtemp;		/* some temp regs */
char etemp;		/* some temp regs */
char ftemp;		/* some temp regs */
char gtemp;		/* some temp regs */
char htemp;		/* some temp regs */

int itemp;		/* some temp regs */
int jtemp;		/* some temp regs */
int ktemp;		/* some temp regs */
int ltemp;		/* some temp regs */
int mtemp;		/* some temp regs */
int ntemp;		/* some temp regs */

char tline[255];	/* a temp buffer */
