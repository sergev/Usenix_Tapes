#include <stdio.h>

#include <sys/file.h>

#include "service.h"
#include "client.h"

#define TIMEOUT 3

#define SIGN_ON	    "sign on."
#define NUM_MSGS 5

char in_buf[BUFSIZ];

struct server *
select_server( server_table, num_servers )
struct server *server_table;
int num_servers;
{
    int i;

    if ( num_servers == 1 ) {
	printf( "Server message: [%2d] %s\n",
	        server_table[0].msg_len, server_table[0].msg );
	return &server_table[0];
    }
    for ( i = 0; i < num_servers; ++i ) {
	printf( "%2d: [%2d] %s\n", i,
	        server_table[i].msg_len, server_table[i].msg );
    }
    while ( i < 0 || i >= num_servers ) {
	printf( "Which one? " ); fflush(stdout);
	if ( scanf( "%d", &i ) == 0 ) {
	    printf( "All right, none of them!\n" );
	    return 0;
	}
    }
    printf("Selected %d\n", i); fflush(stdout);
    return &server_table[i];
}

void
dropped_by_server( msg, len )
char *msg;
int len;
{
    printf( "Dropped by server process: [%2d] %s\n", len, msg );
    exit(1);
}

main( argc, argv)
int argc;
char *argv[];
{
    int i;
    int sock_status;
    char *msg = "They're here!";
    char *sign_off_msg = "Bye bye";
    char buf[512];
    int chars;

    set_dropped_func( dropped_by_server );
    set_select_server( select_server );

    if ( argc > 1 ) msg = argv[1];

    if ( obtain_service( SERV_NAME, SERV_PORT, TIMEOUT, 0,
			 SIGN_ON, sizeof(SIGN_ON) ) < 0 ) {
	fprintf( stderr, "Service request timed out\n" );
	exit(1);
    }
    printf( "Client side started\n" );

    /* Set the socket to do non-blocking reads */
    sock_status = fcntl( server_socket(), F_GETFL, 0 );
    sock_status |= FNDELAY;
    fcntl( server_socket(), F_SETFL, sock_status );

    /* Start into the main loop of send/receive */
    for ( i = 0; i < NUM_MSGS; ++i ) {
	if ( (chars = recv_server( buf, 512, 0 )) > 0 ) {
	    printf( "Received [%d bytes] '%s'\n", chars, buf );
	}
	send_server( msg, strlen(msg)+1, 0 );
	printf( "Sent message #%d\n", i );
	sleep(2);
    }
    if ( (chars = recv_server( buf, 512, 0 )) > 0 ) {
	printf( "Received [%d bytes] '%s'\n", chars, buf );
    }
    sign_off( sign_off_msg, strlen(sign_off_msg)+1, 0 );
}

