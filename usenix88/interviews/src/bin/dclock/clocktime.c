/*
 * time related functions for digital clock
 */

#include <string.h>
#include <time.h>

#include "clocktime.h"

#ifndef hpux
extern void gettimeofday( void *, void * );
#endif

Clock :: Clock () {
#ifdef hpux
    gettimeofday((timeval*)&gmt, 0);
#else
    gettimeofday(&gmt, 0);
#endif
    nextMinute = gmt.sec;
}

int Clock :: NextTick () {
#ifdef hpux
    gettimeofday((timeval*)&gmt, 0);
#else
    gettimeofday(&gmt, 0);
#endif
    return nextMinute - gmt.sec;
}

void Clock :: GetTime ( char * date, int& h, int& m, int& s ) {
    struct tm local;

    local = * localtime( &gmt.sec );
    h = local.tm_hour;
    m = local.tm_min;
    s = local.tm_sec;
    char ds[26];
    strcpy( ds, asctime( &local ));
    strncpy( date, ds, 10 );			// day, month, day of month
    date[10] = '\0';
    strncat( date, ds+19, 5 );			// year
    date[15] = '\0';
    nextMinute = gmt.sec + (60 - s);
}
