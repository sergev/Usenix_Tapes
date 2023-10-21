/*
 *	Structure for referencing pieces of localtime()
 */
struct time{
	int	t_seconds;
	int	t_minutes;
	int	t_hours;
	int	t_day_month;
	int	t_month;
	int	t_year;
	int	t_day_week;
	int	t_day_year;
	int	t_flag;
};
