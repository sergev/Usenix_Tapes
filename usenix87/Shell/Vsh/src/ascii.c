#include "strings.h"	/* See strings.h for comments */

char charclass  [129] = 
{
/* -1    */
    EF,
/* 00-07 */
	EF, UD, UD, EF, EF, UD, UD, UD,
/* 08-0f */
	UD, WS, EL, UD, UD, EL, UD, UD,
/* 10-17 */
	UD, UD, UD, UD, UD, UD, UD, UD,
/* 18-1f */
	UD, UD, UD, UD, WS, UD, UD, UD,
/* 20-27 */
	WS, SC, SC, SC, SC, SC, SC, SC,
/* 28-2f */
	SC, SC, SC, SC, SC, SC, SC, SC,
/* 30-37 */
	NU, NU, NU, NU, NU, NU, NU, NU,
/* 38-3f */
	NU, NU, SC, SC, SC, SC, SC, SC,
/* 40-47 */
	SC, UA, UA, UA, UA, UA, UA, UA,
/* 48-4f */
	UA, UA, UA, UA, UA, UA, UA, UA,
/* 50-57 */
	UA, UA, UA, UA, UA, UA, UA, UA,
/* 58-5f */
	UA, UA, UA, SC, SC, SC, SC, SC,
/* 60-67 */
	SC, LA, LA, LA, LA, LA, LA, LA,
/* 68-6f */
	LA, LA, LA, LA, LA, LA, LA, LA,
/* 70-77 */
	LA, LA, LA, LA, LA, LA, LA, LA,
/* 78-7f */
	LA, LA, LA, SC, SC, SC, SC, EF
} ;
