/**			 save_opts.h 			**/

/** Some crazy includes for the save-opts part of the Elm program!

    (C) Copyright 1986, Dave Taylor
**/

#define ALTERNATIVES		0
#define ALWAYSDELETE		1
#define ALWAYSLEAVE		2
#define ARROW			3
#define AUTOCOPY		4
#define BOUNCEBACK		5
#define CALENDAR		6
#define COPY			7
#define EDITOR			8
#define EDITOUT			9
#define FORMS			10
#define FULLNAME		11
#define KEYPAD			12
#define LOCALSIGNATURE		13
#define MAILBOX			14
#define MAILDIR			15
#define MENU			16
#define MOVEPAGE		17
#define NAMES			18
#define NOHEADER		19
#define PAGER			20
#define POINTNEW		21
#define PREFIX			22
#define PRINT			23
#define REMOTESIGNATURE		24
#define RESOLVE			25
#define SAVEMAIL		26
#define SAVENAME		27
#define SHELL			28
#define SIGNATURE		29
#define SOFTKEYS		30
#define SORTBY			31
#define TIMEOUT			32
#define TITLES			33
#define USERLEVEL		34
#define WARNINGS		35
#define WEED			36
#define WEEDOUT			37

#define NUMBER_OF_SAVEABLE_OPTIONS	WEEDOUT+1

struct save_info_recs { 
	char 	name[NLEN]; 	/* name of instruction */
	long 	offset;		/* offset into elmrc-info file */
	} save_info[NUMBER_OF_SAVEABLE_OPTIONS] = 
{
 { "alternatives", -1L }, { "alwaysdelete", -1L }, 	{ "alwaysleave", -1L },
 { "arrow", -1L},         { "autocopy", -1L },      	{ "bounceback", -1L },
 { "calendar", -1L }, 	  { "copy", -1L },          	{ "editor", -1L },
 { "editout", -1L }, 	  { "forms", -1L },         	{ "fullname", -1L },
 { "keypad", -1L }, 	  { "localsignature", -1L },	{ "mailbox", -1L }, 
 { "maildir", -1L }, 	  { "menu", -1L }, 		{ "movepage", -1L }, 
 { "names", -1L },        { "noheader", -1L }, 		{ "pager", -1L }, 
 { "pointnew", -1L},      { "prefix", -1L },       	{ "print", -1L }, 
 { "remotesignature",-1L},{ "resolve", -1L },       	{ "savemail", -1L }, 
 { "savename", -1L },     { "shell", -1L },         	{ "signature", -1L },
 { "softkeys", -1L },	  { "sortby", -1L }, 		{ "timeout", -1L },
 { "titles", -1L },       { "userlevel", -1L }, 	{ "warnings", -1L },
 { "weed", -1L },         { "weedout", -1L }
};
