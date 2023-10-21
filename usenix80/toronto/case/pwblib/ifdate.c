
/*
 *	ifdate(s)
 *	char *s;
 *
 *	ifdate - this routine will check a string (s)
 *	like "MoTu0800-1730" to see if the present
 *	time is within the given limits.
 *
 *	String alternatives:
 *		Wk - Mo thru Fr
 *		zero or one time means all day
 *		Any - any day
 *
 *	return codes:
 *		0 - not within limits
 *		1 - within limits
 */
# include "stdio.h"
# include "time.h"

ifdate(s)
char *s;
{
static char *days[] = {
	"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa", 0
};
long clock;
int i, tl, th, tn, dayok 0;
struct time *localtime();
struct time *tp;

	time(&clock);
	tp = localtime(&clock);
	while(isalpha(*s)){
		for(i=0; days[i]; i++){
			if(prefix(days[i],s))
				if(tp->t_day_week == i)
					dayok = 1;
		}
		if(prefix("Wk",s))
			if(tp->t_day_week >= 1 && tp->t_day_week <= 5)
				dayok = 1;
		if(prefix("Any",s))
			dayok = 1;
		s++;
	}
	if(dayok == 0)
		return(0);
	i = sscanf(s,"%d-%d",&tl,&th);
	tn = tp->t_hours * 100 + tp->t_minutes;
	if(i < 2)
		return(1);
	if(tn >= tl && tn <= th)
		return(1);
	return(0);
}
