#include <stdio.h>
#include <signal.h>

#include <sys/types.h>
#include <netinet/in.h>

#include "service.h"
#include "server.h"

#define NO_CLIENTS_MSG	"We don't have any clients (yet)"

char in_buf[BUFSIZ];

void
change_number_of_clients( by )
int by;
{
    static char buf[64];
    static num_clients = 0;

    num_clients += by;
    if ( num_clients != 0 ) {
	sprintf( buf, "We have %d clients.", num_clients );
    }
    else {
	strcpy( buf, "We don't have any clients." );
    }
    set_new_client_message( buf, strlen(buf)+1 );
}

void
register_new_client( client, msg, len )
int client;
char *msg;
int len;
{
    change_number_of_clients(1);
    printf( "   Client %d signs on: [%2d] %s\n", client, len, msg );
}

void
client_signed_off( client, msg, len )
int client;
char *msg;
int len;
{
    printf("   Client %d signed off with message: [%2d] %s\n",client,len,msg);
    change_number_of_clients(-1);
}

#define ABNORMAL_TERM "Abnormal termination"

void
drop_client_like_hot_potato(client)
int client;
{
    printf( "   Dropping client %d.\n", client );
    drop_client( client, ABNORMAL_TERM, sizeof(ABNORMAL_TERM), 0 );
    change_number_of_clients(-1);
}

void
drop_all_clients_and_exit(exit_val)
int exit_val;
{
    printf("Drop all clients, then exit with value %d\n",exit_val);
    for_each_client( drop_client_like_hot_potato );
    exit(exit_val);
}

main()
{
    setlinebuf( stdout );
    setlinebuf( stderr );

    set_new_client_message( NO_CLIENTS_MSG, sizeof(NO_CLIENTS_MSG) );
    set_new_client_func( register_new_client );
    set_dead_client_func( client_signed_off );

    signal( SIGINT,  drop_all_clients_and_exit );
    signal( SIGTERM, drop_all_clients_and_exit );

    change_number_of_clients(0);

    if ( setup_server( SERV_NAME, SERV_PORT ) < 0 ) {
	perror( "setup_server" );
	exit(1);
    }

    for ( ;; ) {
	int recv_size, client_num;

	recv_size = sizeof(in_buf);
	if ( (client_num = recv_client( in_buf, &recv_size, 0 )) < 0 ) {
	    perror( "receive" );
	    continue;
	}

	/* Data received, display it. */
	printf( "From client %2d: [%2d] %s\n", client_num, recv_size, in_buf );

	/* Reply to the bugger. */
#define MSG "Hello."
	send_client( client_num, MSG, sizeof(MSG), 0 );
    }
}
