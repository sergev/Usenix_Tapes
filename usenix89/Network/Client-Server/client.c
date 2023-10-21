#include <stdio.h>
#include <netdb.h>

#include "comm.h"
#include "client.h"

#include <sys/time.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <net/if.h>

#ifdef DEBUG
#undef DEBUG
#define DEBUG(x)	(printf x, fflush(stdout), 1)
#define DEBUGGING	(1)
#else
#define DEBUG(x)	(0)
#define DEBUGGING	(0)
#endif DEBUG

#ifndef FD_SET

#define NBBY (8)
#define NFDBITS	(sizeof(fd_set) * NBBY)	/* bits per mask */
#define FD_SETSIZE NFDBITS

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))

#endif FD_SET

static int serv_sock = -1;

int server_socket() { return serv_sock; }

static struct sockaddr server_addr;
static int server_addr_size = sizeof(server_addr);

/*
 *	void set_select_server( struct server*(*new_func)(struct server*,int) )
 *
 *	These structures are used to determine which of the servers which
 *	answer the initial call will be selected for signing on.  The
 *	servers which have answered are listed in the NULL-terminated array
 *	pointed to by all_servers.  This is passed to the function pointed
 *	to by select_server, which should choose one and return a pointer
 *	to it.  The user may select a different choosing function than the
 *	default (select the first one) by calling the function
 *	set_select_server().
 */

#define NUM_SERVERS (5)

static struct server server_space[NUM_SERVERS];
static int num_server_spaces = NUM_SERVERS;
static int num_servers = 0;
static struct server *all_servers = server_space;

static struct server *
default_select_server( servers, num_servers )
struct server *servers;
int num_servers;
{
    /* Select first server */
    return &servers[0];
}

static struct server *(*select_server)() = default_select_server;

void
set_select_server( new_function )
struct server *(*new_function)();
{
    select_server = new_function;
}

/*
 *	void set_dropped_func( void (*)( char *, int ) );
 *
 *	When a SIGN_OFF message is received from the server, the function
 *	specified by dropped_func is called with a description of the rest
 *	of the buffer in which the message is stored.  It may take any action
 *	that the user wants, including exiting.  If it is set to 0 (the
 *	default) then no function is called, and the recv_server function will
 *	just return with the DROPPED return value.
 */

static void (*dropped_func)() = 0;

void
set_dropped_func( new_df )
void (*new_df)();
{
    dropped_func = new_df;
}

/*
 *	int obtain_service( char *name, int port, int timeout_sec, int verbose,
 *			    char *sign_on_msg, int sign_on_len )
 *
 *	    Sends a broadcast message over the net asking for the existence
 *	of a server socket on the given port associated with the named
 *	service.  The port argument is a default port number if the service
 *	is not found in the local net DB.  If there is no response after
 *	timeout_sec seconds, then we return with -1.  If a response is
 *	received, then the socket is connected to that address and the socket
 *	number is returned.  The socket type used is SOCK_DGRAM.
 */

int
obtain_service( serv_name, serv_port, timeout_sec, verbose,
	        sign_on_msg, sign_on_len )
char *serv_name;
int serv_port;
int timeout_sec;
int verbose;
char *sign_on_msg;
int sign_on_len;
{
    int sock;
    long curr_time, end_time, time();
    struct timeval timeout;
    struct servent  *serv,  *getservbyname();
    struct protoent *proto, *getprotobyname();
    int serv_proto = 0;
    static char tbuf[64];
    struct server *selected_server;
    int serv_name_len;
    int server_addr_len;
    void request_service();
    void add_server_response(), free_servers();

    serv_name_len = strlen(serv_name) + 1;

#if DEBUGGING
    verbose = 1;
#endif

    /* Get service information */
    serv_port = htons(serv_port);
    if ( (serv = getservbyname( serv_name, (char *) 0 )) != NULL ) {
	serv_port = serv->s_port;
#ifdef PROTO_SUPPORTED
	if ( (proto = getprotobyname(serv->s_proto)) != NULL )
	    serv_proto = proto->p_proto;
#endif PROTO_SUPPORTED
    }

    DEBUG(( "Access port %d and proto %d\n", ntohs(serv_port), serv_proto ));

    /* Initialize the socket */
    if ( (sock = socket( AF_INET, SOCK_DGRAM, serv_proto )) < 0 ) {
	perror( "creating socket to server" );
	exit(1);
    }

    /* Make the request */
    if ( verbose ) {
	printf( "[Request %s service ...", serv_name ); fflush(stdout);
    }
    request_service( sock, serv_port, serv_name, serv_name_len );

    /* Collect responses until time is up */
    for ( curr_time = time(0), end_time = curr_time + timeout_sec;
	  curr_time < end_time;
	  curr_time = time(0) ) {
	int msg_size;
	fd_set mask;

	/* Initialize the timeout structure */
	timeout.tv_sec = end_time - curr_time;
	timeout.tv_usec = 0;

	/* Now, wait for some response */
	FD_ZERO(&mask);
	FD_SET(sock,&mask);
	if ( select(FD_SETSIZE,&mask,(fd_set *)0,(fd_set *)0,&timeout) <= 0 ||
	     !FD_ISSET( sock, &mask ) ) {
	    break;
	}

	/* So, peek at the response (to get address) */
	server_addr_len = server_addr_size;
	if ( (msg_size = recvfrom( sock, tbuf, sizeof(tbuf), 0,
				   &server_addr, &server_addr_len )) < 0 ) {
	    perror( "receive initial response" );
	    exit(1);
	}

	/* Make sure that this is a response to the service requested */
	if ( !strcmp( serv_name, tbuf ) ) {
	    /* Enter this response into the collection received so far */
	    add_server_response(&server_addr,server_addr_len,tbuf,msg_size);

	    /* We got a response (YAY!) */
	    if ( verbose ) {
		printf( " %d", num_servers );
		fflush(stdout);
	    }

	    DEBUG(( "[by %s %d]...",
		    inet_ntoa(((struct sockaddr_in *) &server_addr)->sin_addr),
		    ntohs(((struct sockaddr_in *) &server_addr)->sin_port) ));
	}
    }

    /* Failure if no responses */
    if ( num_servers == 0 ) {
	if ( verbose ) {
	    printf( " timed out]\n" );
	    fflush(stdout);
	}
	return NO_SERVER;
    }

    /* Connect socket to the address */
    if ( !(selected_server = (*select_server)(all_servers,num_servers)) ) {
	return START_SERVER;
    }
    server_addr = selected_server->addr;
    server_addr_size = selected_server->addr_len;
    DEBUG(("connect(%d,%#x (%s),%d)\n",sock,&server_addr,
	   inet_ntoa(((struct sockaddr_in *)&server_addr)->sin_addr),
	   server_addr_size));
    if ( connect( sock, &server_addr, server_addr_size ) < 0 ) {
	perror( "connect to server" );
	exit(1);
    }

    /* Send the sign-on message to the selected server */
    serv_sock = sock;
    {
	char sign_on_buf[512];

	sign_on_buf[0] = MSG_SIGN_ON;	/* Sign on flag to server */
	bcopy( serv_name, sign_on_buf+1, serv_name_len );
	bcopy( sign_on_msg, sign_on_buf+serv_name_len+1, sign_on_len );
	send_server( sign_on_buf, serv_name_len+sign_on_len+1, 0 );
    }
    free_servers();

    /* Return with a happy (:-) response */
    if ( verbose ) {
	printf( " connected]\n" );
	fflush(stdout);
    }
    return sock;
}

/*
 *	int get_ifc( int sock )
 *
 *	    Put the IFCONF data structure into the ifc structure and return
 *	the number of ifreq structures it contains.
 */

static struct ifconf ifc;

int
get_ifc( sock )
int sock;
{
    static char ifc_buffer[BUFSIZ];

    ifc.ifc_len = sizeof(ifc_buffer);
    ifc.ifc_buf = ifc_buffer;
    if ( (ioctl( sock, SIOCGIFCONF, (char *) &ifc )) < 0 ) {
	perror( "get ifconf" );
	exit(1);
    }
    return ifc.ifc_len / sizeof(struct ifreq);
}

#if DEBUGGING
#include <ctype.h>

char *
show_data( p, len )
unsigned char *p;
int len;
{
    static char buf[512], *q;
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
 *	void request_service( int sock, int port, char *name, int name_len )
 *
 *	    This function broadcasts a request for the given service using
 *	the socket sock.  The request is directed at servers on the named
 *	port on all machines in the local INET network.
 */

void
request_service( sock, port, name, name_len )
int sock;
int port;
char *name;
int name_len;
{
    struct sockaddr_in sin;
    struct sockaddr dst;
    int off = 0, on = 1;
    int n;
    static char data[64];
    int data_len = name_len + 1;

#ifdef SO_BROADCAST
    struct ifreq *ifr;

    /* Set up the data buffer for an initial request */
    data[0] = MSG_REQUEST;
    bcopy( name, data+1, name_len );
    DEBUG(("Broadcast [%2d] %s\n",data_len,show_data(data,data_len) ));

    /* Set socket for broadcast mode */
    if ( (setsockopt( sock,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on) )) < 0 ) {
	perror( "set socket option for broadcast" );
	exit(1);
    }

    /* Initialize broadcast address and bind socket to it */
    sin.sin_family = AF_INET;
    sin.sin_port = port;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    bind( sock, (struct sockaddr *) &sin, sizeof(sin) );

    for ( n = get_ifc(sock), ifr = ifc.ifc_req; n > 0; --n, ++ifr ) {

	/* Only deal with AF_INET networks */
	if ( ifr->ifr_addr.sa_family != AF_INET ) continue;

	/* Use the current address by default */
	bzero( (char *) &dst, sizeof(dst) );
	bcopy( (char *)&ifr->ifr_addr, (char *)&dst, sizeof(ifr->ifr_addr) );

	/* Get the flags */
	if ( ioctl( sock, SIOCGIFFLAGS, (char *) ifr ) < 0 ) {
	    perror( "get ifr flags" );
	    exit(1);
	}

	/* Skip unusable cases */
	if ( !(ifr->ifr_flags & IFF_UP) ||	    /* If not up, OR */
	     (ifr->ifr_flags & IFF_LOOPBACK ) ||    /* if loopback, OR */
	     !(ifr->ifr_flags & (IFF_BROADCAST | IFF_POINTOPOINT)) )
	    continue;

	/* Now, determine the address to send to */
	if ( ifr->ifr_flags & IFF_POINTOPOINT ) {
	    if ( ioctl( sock, SIOCGIFDSTADDR, (char *) ifr ) < 0 ) {
		perror( "get ifr destination address" );
		exit(1);
	    }
	    bcopy( (char *) &ifr->ifr_dstaddr,
		   (char *) &dst,
		   sizeof(ifr->ifr_dstaddr) );
	}
	if ( ifr->ifr_flags & IFF_BROADCAST ) {
	    if ( ioctl( sock, SIOCGIFBRDADDR, (char *) ifr ) < 0 ) {
		perror( "get ifr broadcast address" );
		exit(1);
	    }
	    bcopy( (char *) &ifr->ifr_broadaddr,
		   (char *) &dst,
		   sizeof(ifr->ifr_broadaddr) );
	}

	/* ... make sure that the port number is OK */
	if ( dst.sa_family == AF_INET ) {
	    struct sockaddr_in *sa_in = (struct sockaddr_in *) &dst;
	    sa_in->sin_port = port;
	    DEBUG(( "Request address = %s (%d)\n",
		    inet_ntoa(sa_in->sin_addr), ntohs(port) ));
	}

	/* ... and send the request */
	DEBUG((" Send to %d [%2d] %s\n",sock,data_len,show_data(data,data_len)));
	if ( sendto( sock, data, data_len, 0,
		     (struct sockaddr *) &dst, sizeof(dst) ) < 0 ) {
	    perror( "sendto" );
	    exit(1);
	}
    }

    /* Reset socket option to normal operation */
    if ( (setsockopt( sock,SOL_SOCKET,SO_BROADCAST,&off,sizeof(off) )) < 0 ) {
	perror( "reset socket option for no broadcast" );
	exit(1);
    }

#else !defined(SO_BROADCAST)
    struct hostent *host, *gethostent();

    /* Set up the data buffer for an initial request */
    data[0] = MSG_REQUEST;
    bcopy( name, data+1, name_len );
    DEBUG(("Broadcast [%2d] %s\n",data_len,show_data(data,data_len) ));

    DEBUG(("Attempting pseudo broadcast\n"));

    /* Simulate broadcast if you ain't got it */
    while ( (host = gethostent()) != NULL ) {
	/* Only deal with AF_INET networks */
	if ( host->h_addrtype != AF_INET ||
	     *host->h_addr == 127 /* LOOPBACK */ ) {
	    DEBUG(("Host %s has address family %d & first %d\n",
		   host->h_name, host->h_addrtype,
		   (unsigned char) *host->h_addr ));
	    continue;
	}

	/* Start building the address from scratch */
	bzero( (char *) &dst, sizeof(dst) );

	{
	    struct sockaddr_in *sa_in = (struct sockaddr_in *) &dst;

	    /* Copy the host address into the destination address */
	    bcopy( (char *) host->h_addr,
		   (char *) &(sa_in->sin_addr),
		   host->h_length );

	    /* ... make sure that the port number is OK */
	    sa_in->sin_family = host->h_addrtype;
	    sa_in->sin_port = port;
	}

	/* ... and send the request */
	DEBUG(( "Send request to %s at %s %d\n", host->h_name,
	        inet_ntoa(*(struct in_addr *)host->h_addr), ntohs(port) ));
	DEBUG((" Send [%2d] %s\n",data_len,show_data(data,data_len)));
	if ( sendto( sock, data, data_len, 0,
		     (struct sockaddr *) &dst, sizeof(dst) ) < 0 ) {
	    perror( "sendto" );
	    exit(1);
	}
    }
#endif SO_BROADCAST
}

/*
 *	void add_server_response( struct sockaddr *addr, int addr_len,
 *				  char *msg, int msg_len )
 *
 *	Add the given server address and message to the list of servers
 *	which have responded to the initial broadcast request.  This
 *	table will be used for selection of the appropriate server to
 *	register with.
 */

static void
add_server_response( addr, addr_len, msg, msg_len )
struct sockaddr *addr;
int addr_len;
char *msg;
int msg_len;
{
    char *malloc(), *index();
    struct server *next;

    /* If we have run out of space for saving servers, then expand */
    if ( num_servers == num_server_spaces ) {
	struct server *old_servers = all_servers;
	all_servers = (struct server *)
	  malloc( (num_server_spaces + 5) * sizeof(struct server) );
	bcopy( (char *) old_servers,
	      (char *) all_servers,
	      num_server_spaces * sizeof(struct server) );
	if ( old_servers != server_space ) free( old_servers );
	num_server_spaces += 5;
    }

    /* Go to next server location */
    next = &all_servers[num_servers];
    ++num_servers;

    /* Copy the information into the newly allocated server structure */
    msg_len -= strlen(msg) + 1;
    msg = index(msg,'\0') + 1;
    DEBUG(("Received message [%2d] %s\n",msg_len,show_data(msg,msg_len)));
    next->addr = *addr;
    next->addr_len = addr_len;
    next->msg_len = msg_len;
    next->msg = malloc( msg_len );
    bcopy( msg, next->msg, msg_len );
}

/*
 *	void free_servers()
 *
 *	Clean up the memory allocated for saving server information
 *	after the correct server has been selected.
 */

static void
free_servers()
{
    register int i;

    /* Free space allocated for messages */
    for ( i = 0; i < num_servers; ++i ) {
	free( all_servers[i].msg );
    }

    /* Free the server space if was allocated by malloc */
    if ( all_servers != server_space ) {
	free( all_servers );
    }
}

/*
 *	int recv_server( char *buf, int buf_len, int flags )
 *
 *	Read a message from the server socket.  Any messages that arrive on
 *	this socket which aren't from the server are ignored (silently).
 */

int
recv_server( buf, buf_len, flags )
char *buf;
int buf_len;
int flags;
{
    struct sockaddr addr;
    int addr_len;
    int cnt;

    for ( ;; ) {
	DEBUG(("Listening for message from server\n"));
	addr_len = sizeof(addr);
	cnt = recvfrom( serv_sock, buf, buf_len, flags,
		       &addr, &addr_len );
	if ( cnt < 0 ) {
	    DEBUG(("   No message\n"));
	    return cnt;
	}
	DEBUG((" From %s %d (server at %s %d)\n",
	       inet_ntoa(((struct sockaddr_in *)&addr)->sin_addr),
	       ntohs(((struct sockaddr_in *)&addr)->sin_port),
	       inet_ntoa(((struct sockaddr_in *)&server_addr)->sin_addr),
	       ntohs(((struct sockaddr_in *)&server_addr)->sin_port)));
	if ( server_addr_size == addr_len &&
	     !bcmp( (char *) &server_addr, (char *) &addr, addr_len ) ) {
	    switch ( *buf ) {
	      case MSG_SIGN_OFF:
		DEBUG(("   Received sign off message: [%2d] %s\n",
		       cnt-1,show_data(buf+1,cnt-1)));
		if ( dropped_func ) (*dropped_func)( buf+1, cnt-1 );
		return RECV_DROPPED;
	    }
	    DEBUG(("   Received message: [%2d] %s\n",cnt,show_data(buf,cnt)));
	    return cnt;
	}
    }
}

/*
 *	int send_server( char *buf, int buf_len, int flags )
 *
 *	Send a message to the server.
 */

int
send_server( buf, buf_len, flags )
char *buf;
int buf_len;
int flags;
{
    DEBUG(("Send message [%2d] %s\n",buf_len,show_data(buf,buf_len)));
    return send( serv_sock, buf, buf_len, flags );
}

/*
 *	int sign_off( char *buf, int buf_len, int flags )
 *
 *	Tell the server that we are signing off.
 */

int
sign_off( buf, buf_len, flags )
char *buf;
int buf_len;
int flags;
{
    char tbuf[512];

    DEBUG(("Signing off with [%2d] %s\n",buf_len,show_data(buf,buf_len)));
    tbuf[0] = MSG_SIGN_OFF;
    bcopy( buf, tbuf+1, buf_len );
    return send_server( tbuf, buf_len+1, flags );
}
