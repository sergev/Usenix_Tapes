#include "hdr.h"

void
timesPrint( routine, chunk, start_time, end_time )
char * routine;
int chunk;
struct timeval start_time;
struct timeval end_time;
{
    int rc;
    extern int errno;

    while( (rc=symlink("/dev/null","timesPrint.lock")) != 0) {
	if (errno != EEXIST) ErrorSystem( __FILE__, __LINE__, -1, rc );
	}

    printf( "%-8s:chunk %d	sec %f\n",
		     routine, chunk,
		     (float)(end_time.tv_sec - start_time.tv_sec) + 
		     (float)(end_time.tv_usec - start_time.tv_usec) * 1e-6 );
    fflush( stdout );

    rc = unlink("timesPrint.lock");
    ErrorSystem( __FILE__, __LINE__, -1, rc );
}
