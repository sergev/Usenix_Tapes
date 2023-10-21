/*
 * dennis@rlgvax
 * prints offsets of fields in TCP connection control block.
 * called by fixtcp sh script
 */
#include <stdio.h>
#include <sys/types.h>		/* u_char */
#include <netinet/tcp.h>	/* tcp_seq typedef */
#include <netinet/tcp_timer.h>	/* tcp timers */
#include <netinet/tcp_var.h>	/* tcp connection control block */
#include <netinet/tcp_fsm.h>	/* defines for tcp states */

/* use S3/S5 strrchr(), but on 4.x systems, remap to Berkeley rindex */
#ifdef BSD4
#	define strrchr	rindex
#endif

#define STR_SAME !strcmp
#define STR_DIFF strcmp

/* fw non-int functions */
char *basename();

/* external non-int functions */
extern	char	*strrchr();

main(argc, argv)
	int	argc;
	char	**argv;
{
	char	*cmd;
	struct	tcpcb	*p = 0;

	cmd = basename(argv[0]);

	if (argc != 2)
		{
usage:
		fprintf(stderr, "usage: %s state|2msl|FIN_WAIT2|TIME_CLOSE\n", cmd);
		exit(1);
		}

	if (STR_SAME(argv[1], "state"))
		printf("0x%x\n", &p->t_state);	/* state offset */
	else if (STR_SAME(argv[1], "2msl"))
		printf("0x%x\n", &p->t_timer[TCPT_2MSL]);	/* timer offset */
	else if (STR_SAME(argv[1], "FIN_WAIT2"))
		printf("0x%x\n", TCPS_FIN_WAIT_2);	/* state value */
	else if (STR_SAME(argv[1], "TIME_CLOSE"))
		printf("0x%x\n", TCPS_TIME_WAIT);	/* state value */
	else
		goto usage;
}

/*
 * return basename of full path name
 */
char *
basename(path)
	char	*path;
{
	char	*cp;		/* general char pointer */

	if ((cp = strrchr(path, '/')) == NULL)	/* no rightmost slash */
		return path;
	else
		return cp;
}
