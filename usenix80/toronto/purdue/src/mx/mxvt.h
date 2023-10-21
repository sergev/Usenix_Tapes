
/*
 * mx virtual terminal definitions
 */

#define IAC     037             /* interpret following as command */

#define C_DM    0               /* data mark */
#define C_GT    1               /* get tty */
#define C_ST    2               /* set tty, followed by 6 bytes */
#define C_IN    3               /* interrupt */
#define C_QU    4               /* quit */
#define C_EF    5               /* end of file */
