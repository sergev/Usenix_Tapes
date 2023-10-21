#include <stdio.h>
#include "hdr.h"
char    buf[MAX_CHUNK];

main(argc, argv) 
int argc;
char * argv[];
{
    FILE * mouthpiece;
    int    chunk;
    int    rc;
    int    no_bytes;

    for (no_bytes = 0; no_bytes < MAX_CHUNK; no_bytes++)
	buf[no_bytes] = -1;

    for (chunk = MAX_CHUNK; chunk >= MIN_CHUNK; chunk -= 128) {
        mouthpiece = (FILE *) popen( "listener", "w" );

#       ifdef DEBUG
	    fprintf( stderr, "%s, line %d:mouthpiece %d\n", 
				__FILE__, __LINE__, mouthpiece );
	    fflush( stderr );
	    fprintf( stderr, "%s, line %d:fileno(mouthpiece) %d\n", 
				__FILE__, __LINE__, fileno(mouthpiece) );
	    fflush( stderr );
#           endif

	if (mouthpiece == (FILE *) NULL)
	    ErrorSystem( __FILE__, __LINE__, -1, -1 );

	rc = write( fileno(mouthpiece), &chunk, sizeof(chunk) );
	ErrorSystem( __FILE__, __LINE__, fileno(mouthpiece), rc );

	(void) gettimeofday( &start_time, &time_zone );
	for (no_bytes = 0; no_bytes < (NO_BYTES-chunk); no_bytes += chunk) {
#           ifdef DEBUG
            if (no_bytes > 1000*1000)
	        fprintf( stderr, "%s, line %d:no_bytes %d	NO_BYTES %d\n", 
				 __FILE__, __LINE__, no_bytes, NO_BYTES );
#               endif

	    rc = write( fileno(mouthpiece), buf, chunk );
	    ErrorSystem( __FILE__, __LINE__, fileno(mouthpiece), rc );
	    }
	if (no_bytes < NO_BYTES) {
#           ifdef DEBUG
	        fprintf( stderr, "%s, line %d:NO_BYTES-no_bytes %d\n", 
				 __FILE__, __LINE__, NO_BYTES-no_bytes );
#               endif

	    rc = write( fileno(mouthpiece), buf, NO_BYTES-no_bytes );
	    ErrorSystem( __FILE__, __LINE__, fileno(mouthpiece), rc );

	    no_bytes += (NO_BYTES-no_bytes);
	    }
	(void) gettimeofday( &end_time, &time_zone );

#       ifdef DEBUG
            fprintf( stderr, "%-8s:no_bytes %d\n", "talker", no_bytes );
#           endif

        (void) timesPrint( "talker", chunk, start_time, end_time );
	rc = pclose( mouthpiece );
	ErrorSystem( __FILE__, __LINE__, fileno(mouthpiece), rc );
	}
}
