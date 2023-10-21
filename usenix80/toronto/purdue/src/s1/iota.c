/*
 * iota -- output indexes
 *
 *	% iota
 *	% iota end
 *	% iota start end
 *	% iota start end incriment
 *
 */

int start	1;
int end;
int incr	1;
char *format	"%d\n";

main(argc, argp)
char **argp;
{
	register i;

	if(argc > 1 && argp[1][0] == '-'){
		if(argp[1][1] == 'o'){
			format = "%o\n";
			argc--;
			argp++;
		} else {
			printf("bad flag: %s\n", argp[1]);
			exit();
		}
	}
	switch(argc){
	default:
		printf("syntax:  iota end  | iota start [end [incr]]\n");
		exit();
	case 2:
		end = a2i(argp[1]);
		break;
	case 4:
		incr = a2i(argp[3]);
	case 3:
		start = a2i(argp[1]);
		end = a2i(argp[2]);
	}
	if(incr == 0)
		incr = 1;
	else if(incr < 0)
		incr = -incr;
	if(start <= end)
		for(i=start; i<= end; i =+ incr)
			pr(i);
	else
		for(i=end; i>=start; i =- incr)
			pr(i);
}

pr(n)
{
	printf(format, n);
}
