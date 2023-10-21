#include "nav.h"
#include <ctype.h>

/*
 *     Store the next Alt datum into the AltArray entry given.
 *
 */
StoreAlt(alt,c,s)
int alt[], c;
char *s;
{
        
        alt[c] = atoi (s);
        return 0;
}
/*
 *      Store the next Airspeed datum into the Airspeed array
 *
 */
StoreSpeed (speed, c, s)
int speed[], c;
char *s;
{
        speed[c] = atoi (s);
        return 0;
}
/*
 *      Store the next Wind datum into the Wind array
 *
 */
StoreWind (wind, c, s)
int wind[], *c;
char *s;
{
int winds_aloft, direction, speed;
        winds_aloft = atoi (s);
        direction = (winds_aloft / 100) * 10;
        if (direction >= 360) direction = direction - 360;
        speed = winds_aloft % 100;
        wind[*c] = direction;
        wind[*c+1] = speed;
        return 0;
}   
/*
 *	Attempt to find airport in array by matching IDs, put result in airport.
 *	Return 1 on error, 0 on success.
 *	Ignore case by upping case in buf.
 */
FindApt(airport,s,array,count)
struct apt *airport;
char *s;
struct apt array[];
int count;
{
	char buf[IDLEN];
	register int i;

	strcpy(buf,s);
	uppit(buf);
	for (i=0; i < count; i++){
		if (strcmp(array[i].id,buf) == 0){
			*airport = array[i];
			return 0;	/* we find only one at a time here */
		}
	}
	return 1;
}

/*
 *	Attempt to find VOR in array by matching IDs, put result in vortac.
 *	Return 1 on error, 0 on success.
 *	Ignore case by upping case in buf.
 */
FindVor(vortac,s,array,count)
struct vor *vortac;
char *s;
struct vor array[];
int count;
{
	char buf[IDLEN];
	register int i;

	strcpy(buf,s);
	uppit(buf);
	for (i=0; i < count; i++){
		if (strcmp(array[i].id,buf) == 0){
			*vortac = array[i];
			return 0;	/* we find only one at a time here */
		}
	}
	return 1;
}

/*
 *	pretty self-explanatory
 */
uppit(s)
char *s;
{
	while ((*s != '\0') && (*s != '\n')){
		if ((isalpha(*s) && islower(*s))){
			*s=toupper(*s);
		}
		if (isspace(*s)){
			*s=' ';		/* Guarantee that it's a blank char */
		}
		s++;
	}
}
