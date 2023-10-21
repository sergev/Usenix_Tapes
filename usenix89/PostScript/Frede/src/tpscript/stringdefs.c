#include	"hash.h"

#include		"stringdefs.h"

SPECIAL_NAME	specnames[] = {
	{"ru",		DEF_ru,		0, }, 
	{"12",		DEF_12,		SN_FRACTION, },
	{"14",		DEF_14,		SN_FRACTION, },
	{"34",		DEF_34,		SN_FRACTION, },
	{"13",		DEF_13,		SN_FRACTION, },
	{"18",		DEF_18,		SN_FRACTION, },
	{"23",		DEF_23,		SN_FRACTION, },
	{"38",		DEF_38,		SN_FRACTION, },
	{"58",		DEF_58,		SN_FRACTION, },
	{"78",		DEF_78,		SN_FRACTION, },
	{"sr",		DEF_sr,		0, },
	{"sq",		DEF_sq,		0, },
	{"ff",		DEF_ff,		0 },
	{"Fi",		DEF_Fi,		0 },
	{"Fl",		DEF_Fl,		0 },
	{ (char *)0,	(char *)0,	0}
};

SPECIAL_NAME	multdefs[] =
{
	{"fract",	DEF_fract,	SN_FRACTION },
	{ (char *)0,	(char *)0,	0}
};
