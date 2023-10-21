/* use tracef() as tracef((tr, <format>, <format-args> ... )); */
#ifdef DEBUG
	extern int	tron;
	extern char	tr[];
#	define trace(step) strace(__FILE__, __LINE__, step)
#	define tracef(step) {sprintf step; trace(tr);}
#else
#	define trace(step)
#	define tracef(step)
#endif
