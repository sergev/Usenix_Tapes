/*
 *  Client program for rpcgen demo
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include "demo_xdr.h"

char *Prog, *Host;

struct Calls {
	char	*name;			/* name of call */
	enum ret_status arg_type;	/* arg type expected */
	int	procno;			/* proc number */
} Calls[] = {
	{ "ctime",	RET_CLOCK,	CTIME		},
	{ "localtime",	RET_CLOCK,	LOCALTIME	},
	{ "gmtime",	RET_CLOCK,	GMTIME		},
	{ "asctime",	RET_TM,		ASCTIME		},
	{ "timezone",	RET_TZ,		TIMEZONE	},
	{ "dysize",	RET_YEAR,	DYSIZE		},
	{ NULL }
};

char	 *Arg;
demo_res Res;
xdrproc_t Xargs, Xres;

main(argc, argv)
char **argv;
{
	struct Calls *callp;
	int ret;

	Prog = argv[0];
	if (argc < 3)
		usage();

	for (callp = Calls; callp->name; callp++)
		if (!strcmp(callp->name, argv[2]))
			break;
	if (callp->name == NULL) {
		fprintf(stderr, "%s: call \"%s\" not recognized.\n",
			Prog, argv[2]);
		exit(1);
	}
	Host = argv[1];
	argc -= 3; argv += 3;
	getargs(argc, argv, callp);
	Xres = xdr_demo_res;
	if (ret = callrpc(Host, DEMOPROG, DEMOVERS, callp->procno,
		Xargs, Arg, Xres, &Res))
	{
		fprintf(stderr, "%s: rpc failed: ", Prog);
		clnt_perrno(ret);
		fprintf(stderr, "\n");
		exit(1);
	}
	printres(callp);
}

getargs(argc, argv, callp)
int argc;
char **argv;
struct Calls *callp;
{
	static long l_arg;
	static timeval tv;
	static tzargs tz_arg;

	gettimeofday(&tv, &tz_arg);
	switch (callp->arg_type) {
	    case RET_CLOCK:
		l_arg = (long)(argc > 0? atoi(argv[0]) : tv.tv_sec);
		Arg = (char *)&l_arg;
		Xargs = xdr_clock;
		break;
	    case RET_TM:
		l_arg = (long)(argc > 0? atoi(argv[0]) : tv.tv_sec);
		Arg = (char *)localtime(&l_arg);
		Xargs = xdr_tm;
		break;
	    case RET_TZ:
		if (argc == 1) {
			fprintf(stderr, "\
%s: call \"%s\" takes two arguments.\n", Prog, callp->name);
			exit(1);
		} else if (argc > 1) {
			tz_arg.zone = atoi(argv[0]);
			tz_arg.dst = atoi(argv[1]);
		}
		Arg = (char *)&tz_arg;
		Xargs = xdr_tzargs;
		break;
	    case RET_YEAR:
		l_arg = (argc > 0? atoi(argv[0]) : 1986);
		Arg = (char *)&l_arg;
		Xargs = xdr_int;
		break;
	    default:
		fprintf(stderr, "%s: panic: unknown arg type %d\n",
			Prog, callp->arg_type);
		exit(1);
	}
}

printres(callp)
struct Calls *callp;
{
	switch (Res.which) {
	    case RET_DATE:
		printf("date from %s: %s\n", Host, Res.demo_res.date);
		break;
	    case RET_TM:
#define TZP(x) Res.demo_res.tmp->tm_/**/x
		printf("\
time info from %s: sec=%d, min=%d, hour=%d, mday=%d, mon=%d,\n\
                   year=%d, wday=%d, yday=%d, isdst=%d\n",
			Host, TZP(sec), TZP(min), TZP(hour), TZP(mday),
			TZP(mon), TZP(year), TZP(wday), TZP(yday), TZP(isdst));
		break;
	    case RET_STR:
		printf("Return string from %s: %s\n", Host, Res.demo_res.str);
		break;
	    case RET_DAYS:
		printf("Return from %s: %d days\n", Host, Res.demo_res.days);
		break;
	    case RET_ERROR:
		printf("%s returned error %d (%s)\n", Host,
			Res.demo_res.err.err_number,
			Res.demo_res.err.err_text);
		break;
	    default:
		printf("%s returned an unknown return type (%d)\n",
			Host, Res.which);
		break;
	}
}

usage()
{
	struct Calls *callp;

	fprintf(stderr, "usage: %s host call [args]\n", Prog);
	fprintf(stderr, "valid calls are:\n");
	for (callp = Calls; callp->name; callp++)
		fprintf(stderr, "\t%s\n", callp->name);
	exit(1);
}
