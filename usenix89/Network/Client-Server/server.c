#ifndef lint
static char Copyright[] = "server.c: Copyright 1986, Lee Iverson";
#endif lint

#include <stdio.h>
#include <netdb.h>

#include "comm.h"
#include "server.h"

#include <netinet/in.h>
#include <net/if.h>

#ifdef DEBUG
#undef DEBUG
#define DEBUGGING (1)
#define DEBUG(x) (printf x, fflush(stdout), 1)
#else
#define DEBUGGING (0)
#define DEBUG(x) (0)
#endif DEBUG

extern char *malloc();
void inactive_client();

#define TBUF_SIZE (512)
static char tbuf[TBUF_SIZE];

static int serv_sock = -1;

int allow_new_clients = 1;

#define UNDEFINED "<undefined>"

static char *service_name = UNDEFINED;
static int service_name_len = sizeof(UNDEFINED);

/*
 *	The message sent to potential clients.
 */

static char new_client_msg[512];
static int new_client_msg_len = 0;

void 
set_new_client_message( msg, len )
char *msg;
int len;
{
    bcopy( msg, new_client_msg, len );
    new_client_msg_len = len;
}

/*
 *	Set up for the client table.
 */

struct client {
    struct sockaddr addr;
    short addr_len;
    char active;
};

static struct client client_space[10];
static int num_client_spaces = 10;
static int num_clients = 0;
static struct client *all_clients = client_space;

/*
 *	Initialize for new client hook.
 */

static void (*new_client_func)() = 0;

void
set_new_client_func( ncf )
void (*ncf)();
{
    new_client_func = ncf;
}

/*
 *	Initialize for dead client hook.
 */

static void (*dead_client_func)() = 0;
void
set_dead_client_func( dcf )
void (*dcf)();
{
    dead_client_func = dcf;
}

/*
 *	int setup_server( char* name, int port )
 *
 *	Set up the server side of the client/server model to receive datagram
 *	messages on a socket with the given service name, on the given port.
 *	If the service named exists in the service table (/etc/services) then
 *	it uses the port number specified therein, otherwise it uses the value
 *	passed as the port parameter.  If the port parameter is 0 and the
 *	named service is not found in the service table, then the function
 *	returns with an error value.  The return value of the function
 *	is the number of the socket initialized, with a negative number
 *	indicating and error.  The negative return values are:
 *		NO_SERVICE	(-1)	Unrecognized service.
 *		NO_SOCKET	(-2)	Unable to create socket (use errno).
 *		NO_BIND		(-3)	Unable to bind socket (use errno).
 */
 
int
setup_server( name, port )
char *name;
int port;
{
    int sock;
    struct sockaddr_in sin;
    struct servent  *serv,  *getservbyname();
    struct protoent *proto, *getprotobyname();
    int serv_proto = 0;

    /* Save the name of the service */
    service_name_len = strlen(name) + 1;
    service_name = malloc( service_name_len );
    bcopy( name, service_name, service_name_len );

    /* Initialize service information */
    port = htons(port);
    if ( (serv = getservbyname( name, (char *) 0 )) != NULL ) {
	port = serv->s_port;
#ifdef PROTO_SUPPORTED
	if ( (proto = getprotobyname(serv->s_proto)) != NULL ) {
	    serv_proto = proto->p_proto;
	}
#endif PROTO_SUPPORTED
    }

    /* If service port not determined, then return with an error */
    if ( port == 0 ) {
	return NO_SERVICE;
    }

    /* Set up the receiving socket */
    if ( (sock = socket( AF_INET, SOCK_DGRAM, serv_proto )) < 0 ) {
	return NO_SOCKET;
    }

    /* Bind this socket to the receiving port */
    sin.sin_family = AF_INET;
    sin.sin_port = port;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if ( bind( sock, (caddr_t)&sin, sizeof(sin) ) < 0 ) {
	return NO_BIND;
    }

    DEBUG(( "Server at port %d on socket %d\n", ntohs(port), sock ));
    serv_sock = sock;
    return sock;
}

#if DEBUGGING
#include <ctype.h>

char *
show_data( p, len )
unsigned char *p;
int len;
{
    static char buf[TBUF_SIZE];
    char *q;
    char *index();
    
    for ( q = buf; len > 0; ++p, --len ) {
	if ( *p != 0 && isascii(*p) ) {
	    *q++ = *p;
	}
	else {
	    sprintf( q, "|%d|", *p );
	    q = index(q,'|') + 1;
	    q = index(q,'|') + 1;
	}
    }
    *q = 0;
    return buf;
}
#endif

/*
 *	int recv_client( char *buf, int *buf_len, int flags )
 *
 *	Receive another message from a client.  If the message is a request
 *	for the service and the server is ready to receive new clients, then
 *	an acknowledgement is returned.  Otherwise, the message is received
 *	from the client and is inserted into the buffer, with the client
 *	number returned.  It should be noted that the buf_len parameter is
 *	a value/return parameter which should point to a value which is the
 *	length of the buffer on input, and is modified to represent the length
 *	of the data received on output.  A negative return value means that
 *	no message was received.
 */

int
recv_client( buf, len, flags )
char *buf;
int *len;
int flags;
{
    struct sockaddr recv_addr;
    int addr_len;
    int client_num;
    int buf_len = *len;

    for ( ;; ) {
	/* Get the message */
	addr_len = sizeof(recv_addr);
	DEBUG(("Listen for input\n"));
	*len = recvfrom( serv_sock, buf, buf_len, flags,
		         &recv_addr, &addr_len );

	/* If the socket is non-blocking and no data, just return */
	if ( *len < 0 ) {
	    return *len;
 	}

	/* Check for a request message */
	DEBUG(("Message [%2d] %s\n",*len,show_data(buf,*len)));
	switch ( *buf ) {
	  case MSG_REQUEST:
	    /* Maybe acknowledge request */
	    DEBUG((" - Request [%2d] %s\n",*len-1,show_data(buf+1,*len-1)));
	    if ( allow_new_clients && !strcmp(buf+1,service_name) ) {
		int name_len = strlen(service_name) + 1;
		bcopy( service_name, tbuf, name_len );
		bcopy( new_client_msg, tbuf+name_len, new_client_msg_len );
		DEBUG(("   Return [%2d] %s\n",name_len+new_client_msg_len,
		       show_data(tbuf,name_len+new_client_msg_len)));
		sendto( serv_sock, tbuf, name_len+new_client_msg_len, 0,
		        &recv_addr, addr_len );
		DEBUG(("   Acknowledge %s [%d]\n",
		       inet_ntoa(((struct sockaddr_in *)&recv_addr)->sin_addr),
		       ntohs(((struct sockaddr_in *)&recv_addr)->sin_port)));
	    }
	    break;
	  case MSG_SIGN_ON:
	    /* Sign on this client */
	    DEBUG((" - Sign on to %s\n", buf+1 ));
	    if ( allow_new_clients && !strcmp(buf+1,service_name) ) {
		client_num = add_new_client( &recv_addr, addr_len );
		if ( new_client_func ) {
		    (*new_client_func)( client_num,
				        buf + service_name_len + 1,
				        *len - (service_name_len + 1) );
		}
	    }
	    else {
		/* Send a sign off message */
#define REJECT_MSG "Attempt to sign on rejected."
		tbuf[0] = MSG_SIGN_OFF;
		bcopy( REJECT_MSG, tbuf, sizeof(REJECT_MSG) );
		DEBUG((" - Attempt to sign on rejected.\n"));
		sendto( serv_sock, tbuf, sizeof(REJECT_MSG)+1, 0,
		       &recv_addr, addr_len );
	    }
	    break;
	  case MSG_SIGN_OFF:
	    /* Sign the client off if he still exists */
	    DEBUG((" - Sign off [%2d] %s\n",*len-1,
		   show_data(buf+1,*len-1)));
	    if ( (client_num = find_client( &recv_addr, addr_len )) >= 0 ) {
		if ( dead_client_func ) {
		    (*dead_client_func)( client_num, buf + 1, *len - 1 );
		}
		inactive_client(client_num);
	    }
	    break;
	  default:
	    /* All other messages */
	    if ( (client_num = find_client( &recv_addr, addr_len )) >= 0 ) {
		DEBUG((" - Message received from %d\n",client_num));
		return client_num;
	    }
	    else {
		DEBUG((" - Not a client, ignored.\n"));
	    }
	    break;
	}
    }
}

/*
 *	void send_client( int client_num, char *buf, int buf_len, int flags )
 *
 *	Send the message in the buffer pointed to by buf to the client
 *	indicated by the client number.  This is a no-op if the client
 *	number is invalid or inactive.
 */

void
send_client( client_num, buf, buf_len, flags )
int client_num;
char *buf;
int buf_len;
int flags;
{
    if ( client_num < num_clients && all_clients[client_num].active ) {
	DEBUG(("   Send to %d [%2d] %s\n",client_num,buf_len,
	       show_data(buf,buf_len)));
	sendto( serv_sock, buf, buf_len, flags,
	        &all_clients[client_num].addr,
	        all_clients[client_num].addr_len );
    }
}

/*
 *	void drop_client( int client_num, char *buf, int buf_len, int flags )
 *
 *	Drop the client.  This sends a message to the client which specifies
 *	that he has been dropped, and marks him as invalid, so any further
 *	messages from him will be ignored.
 */

void
drop_client( client_num, buf, buf_len, flags )
int client_num;
char *buf;
int buf_len;
int flags;
{
    if ( client_num < num_clients && all_clients[client_num].active ) {
	tbuf[0] = MSG_SIGN_OFF;
	bcopy( buf, tbuf+1, buf_len );
	DEBUG(("   Dropping client %d.\n",client_num));
	send_client( client_num, tbuf, buf_len+1, flags );
	inactive_client(client_num);
    }
}

/*
 *	int find_client( struct sockaddr *addr, int addr_len )
 *
 *	Find the client number of a client given the client's address.  If the
 *	client hasn't signed on, then return a negative value.
 */

static int
find_client( addr, addr_len )
struct sockaddr *addr;
int addr_len;
{
    register int i;

    DEBUG(("   Looking for address %s [%d]\n",
	   inet_ntoa(((struct sockaddr_in *)addr)->sin_addr),
	   ntohs(((struct sockaddr_in *)addr)->sin_port)));
	   
    for ( i = 0; i < num_clients; ++i ) {
	if ( all_clients[i].active &&
	     addr_len == all_clients[i].addr_len &&
	     !bcmp( (char *) addr, (char *) &all_clients[i].addr, addr_len) ) {
	    DEBUG(("   Found at client %d\n", i));
	    return i;
	}
    }
    DEBUG(("   Not signed on!\n"));
    return -1;
}

/*
 *	int add_new_client( struct sockaddr *addr, int addr_len )
 *
 *	Add a new client to the client table with the given address.  Returns
 *	the client number of the new client.
 */

static int
add_new_client( addr, addr_len )
struct sockaddr *addr;
int addr_len;
{
    char *index();
    struct client *next = 0;
    int client_num = -1;
    int i;

    /* Find an inactive client */
    for ( i = 0; next == 0 && i < num_clients; ++i ) {
	if ( !all_clients[i].active ) {
	    next = &all_clients[i];
	    client_num = i;
	}
    }

    /* If didn't find an inactive client, allocate a new one */
    if ( next == 0 ) {
	/* If we have run out of space for saving clients, then expand */
	if ( num_clients == num_client_spaces ) {
	    struct client *old_clients = all_clients;
	    all_clients = (struct client *)
	      malloc( (num_client_spaces + 5) * sizeof(struct client) );
	    bcopy( (char *) old_clients,
		   (char *) all_clients,
		  num_client_spaces * sizeof(struct client) );
	    if ( old_clients != client_space ) free( old_clients );
	    num_client_spaces += 5;
	}

	/* Go to next client location */
	next = &all_clients[num_clients];
	client_num = num_clients;
	++num_clients;
    }

    /* Copy the address into the new client structure */
    next->active = 1;
    next->addr = *addr;
    next->addr_len = addr_len;

    /* Register the new client and return */
    DEBUG(("   New client %d at %s\n", client_num,
	   inet_ntoa(((struct sockaddr_in *)addr)->sin_addr)));

    return client_num;
}

/*
 *	void for_each_client( void (*func)(int) )
 *
 *	This calls the parameter function once for each client that is still
 *	active.  The client number is passed to the function.  This is very
 *	useful for dropping all active clients on an interrupt, for example.
 */

void
for_each_client( func )
void (*func)();
{
    int i;
    
    DEBUG(("Call %#x() for each active client.\n"));
    for ( i = 0; i < num_clients; ++i ) {
	if ( all_clients[i].active ) {
	    DEBUG(("   Client %d is active.\n"));
	    (*func)(i);
	}
    }
}

/*
 *	void inactive_client( int client_num )
 *
 *	Mark a client as inactive.  His client number can be reused.
 */

static void
inactive_client( client_num )
int client_num;
{
    if ( client_num < num_clients ) {
	DEBUG(("   Client %d is inactive.\n",client_num));
	all_clients[client_num].active = 0;
    }
}
