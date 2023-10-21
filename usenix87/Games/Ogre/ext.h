#include <stdio.h>
#include "ogre.h"
#define CRATEROUS 12

#ifdef MAIN

UNIT unit[N_UNITS];
OGRE ogre;
int n_units;
int mark;
struct{
	int l_hex;
	int r_hex;
} craters[50];
int donecraters;
int numcraters;

#else

extern UNIT unit[N_UNITS];
extern OGRE ogre;
extern int n_units;
extern int mark;
struct{
	int l_hex;
	int r_hex;
} craters[50];
int donecraters;
int numcraters;

#endif


