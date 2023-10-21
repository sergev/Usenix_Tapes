#include "hdr.h"

main(argc, argv) 
int argc;
char * argv[];
{
    FILE * earpiece = stdin;
    int    chunk;
    int    rc;
    int    no_bytes;
    int    check_value;
    int    no_errors;

    no_errors = 0;

    rc = read( fileno(earpiece), &chunk, sizeof(chunk) );
    ErrorSystem( __FILE__, __LINE__, fileno(earpiece), rc );

#   ifdef
       fprintf( stderr, "%s, line %d:chunk %d\n", __FILE__, __LINE__, chunk );
       fflush( stderr );
#      endif

    check_value = chunk;
    (void) gettimeofday( &start_time, &time_zone );
    for (no_bytes = 0; ;no_bytes += rc) {
        rc = read( fileno(earpiece), buf, chunk );
        ErrorSystem( __FILE__, __LINE__, fileno(earpiece), rc );

	if (rc == 0) 
	    break;

	if ((rc != chunk) && (no_errors < MAX_ERRORS)) {
	    ErrorMsg( __FILE__, __LINE__, 
	              "warning:partial chunk %d != chunk %d", rc, chunk );
	    no_errors++;
	    }

#       ifdef DEBUG
	    if (no_bytes % check_value == 0) {
    	        fprintf( stderr, "%s, line %d:chunk, no_bytes %d, %d\n", 
			         __FILE__, __LINE__, chunk, no_bytes );
    	        fflush( stderr );
		check_value *= 4;
	        }
#	    endif
	}
    (void) gettimeofday( &end_time, &time_zone );

#   ifdef DEBUG
        fprintf( stderr, "%-8s:no_bytes %d\n", "listener", no_bytes );
#       endif

    (void) timesPrint( argv[0], chunk, start_time, end_time );
    fflush( stderr );
}
