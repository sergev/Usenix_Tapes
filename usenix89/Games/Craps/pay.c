#include "types.h"
#include "ext.h"

pay_winners()
{
	int clrp=0,mark=0;

	if(sum==7) {
		if(!point) {
			announce("Seven, A Natural! ~");
			ppassln();
			cdontln();
			ccome(1);
			pdonts();
		} else {
			announce("Seven out! ~");
			announce(line);
			cpassln();
			cplace();
			ccome(0);
			cdontb();
			pcomeb(1);
			pdontln();
			pdonts();
			do_dont(sum);
			clrp=1;
		}
	} else if(sum==11) {
		announce("Eee-yo 'LEVEN! ~");
		if(!point) {
			ppassln();
			cdontln();
		} else {
			pcomeb(0);
			cdontb();
		}
	} else if(sum==2 || sum==3 || sum==12) {
		announce("Craps! ");
		if(sum==2) announce(" - aces! ~");
		else if(sum==3) announce(" - ace-duece! ~");
		else announce(" - sixes! ~");
		if(!point) {
			cpassln();
			if(sum!=12) pdontln();
			else bar_the_12();
		} else {
			ccomeb();
			if(sum!=12) pdontb();
			else bar_the_12();
		}
	} else {
		sprintf(line,"%s!",nums[sum]);
		if(dice[1]==dice[0]) strcat(line,", The Hardway!");
		strcat(line," ~");
		announce(line);
		if(sum==point) {
			ppassln();
			pplace(sum);
			cdontln();
			do_dont(sum);
			clrp=1;
			do_come(sum,1);
		} else {
			if(point) pplace(sum);
			else mark=1;
			csdont(sum,0);
			do_dont(sum);
			do_come(sum,(point!=0));
		}
	}
	pfield();
	props(sum);
	if(clrp) cl_point();
	if(mark) mark_point(sum);
	pr_an();
	update(0);
}
