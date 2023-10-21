/*
 *  Demo procedures
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include "demo_xdr.h"
static demo_res res;

demo_res *
ctime_5(arg)
clock *arg;
{
	res.which = RET_DATE;
	res.demo_res.date = (date)ctime(arg);
	return &res;
}

demo_res *
localtime_5(arg)
clock *arg;
{
	res.which = RET_TM;
	res.demo_res.tmp = (tm *)localtime(arg);
	return &res;
}

demo_res *
gmtime_5(arg)
clock *arg;
{
	res.which = RET_TM;
	res.demo_res.tmp = (tm *)gmtime(arg);
	return &res;
}

demo_res *
asctime_5(arg)
tm *arg;
{
	res.which = RET_DATE;
	res.demo_res.date = (date)asctime(arg);
	return &res;
}

demo_res *
timezone_5(arg)
tzargs *arg;
{
	res.which = RET_STR;
	res.demo_res.str = (str)timezone(arg->zone, arg->dst);
	return &res;
}

demo_res *
dysize_5(val)
int *val;
{
	res.which = RET_DAYS;
	res.demo_res.days = dysize(*val);
	return &res;
}
