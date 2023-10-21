/* Copyright MITRE Corp. */

#define  MAIN_FONT_DIR 		"/usr/local/lib/fonts/vtfonts"

/* This structure holds the array of labels for the Pf keys on the
   vt100 panel. It is best not to try to add or subtract from the
   structure. The labels themselves can be easily changed in the
   array "keys" which follows. Just keep them to 6 chars in length.
*/

struct key_labels {
  caddr_t button_handle;
  char norm_label[6];
  char mred_label[6];
  char ibm_label[6];
  char wd11_label[6];
  char emacs_label[6];
};

/* These are the labels which appear on the PF keys when the mouse
   clicks the label choice item (looks better in a wide window) */
struct key_labels keys[23] = {
  0,	"",	"",             "",		"",   		 "",                        /* dummy     */
  0,	"1",	"-line",	"10",           "back",          "",                        /* 1 key = 1 */
  0,	"2",	"v",		"11",           "line",          "",                        /* 2 key = 2*/     
  0,	"3",	"-page",	"12",           "uppr",          "",                        /* 3 key = 3 */    
  0,	"4",	"<--",		"7",            "word",          "",                        /* 4 key = 4 */    
  0,	"5",	"home",		"8",            "para",          "",                        /* 5 key = 5 */    
  0,	"6",	"-->",		"9",            "bold",          "",                        /* 6 key = 6 */    
  0,	"7",	"+line",	"4",            "< >",           "",                        /* 7 key = 7 */    
  0,    "8",	"^",		"5",            "tabp",          "",                        /* 8 key = 8 */    
  0,	"9",	"+page",	"6",            "under",         "",                        /* 9 key = 9 */    
  0,	"0",	"rplcw",	"pf+12",        "advan",         "",			    /* 0 key = 10 */  
  0,	"PF1",	"pick",		"1",            "gold",          "",                        /* PF1 key = 11 */ 
  0,	"PF2",	"put",		"2",            "page",          "",                        /* PF2 key = 12 */ 
  0,	"PF3",	"-tabw",	"3",            "sent",          "",                        /* PF3 key = 13 */ 
  0,	"PF4",	"+tabw",	"PA1",          "  ",            "",			    /* PF4 key = 14 */
  0,	"-",	"+srch",	"PA2",          "aftwd",         "",                        /* - key = 15 */   
  0,	",",	"-srch",	"PA3",          "phras",         "",                        /* , key = 16 */   
  0,	"enter","goto",		"clear",        "swap",          "",			    /* enter key = 17 */
  0,	".",	"use",		"insrt",        "selct",         "",			    /* . key = 18 */  
  0,	"^",	"open",		"^",            "ruler",         "",                        /* up key = 19 */  
  0,	"-->",	"close",	"-->",          "cut",           "",                        /* right key = 20 */
  0,	"v",	"fill",		"v",            "delte",         "",                        /* down key = 21 */
  0,	"<--",	"tmpin",	"<--",          "paste",         ""			    /* left key = 22 */
};
