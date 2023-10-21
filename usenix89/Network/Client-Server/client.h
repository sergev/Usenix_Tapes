/*
 *	client.h: Copyright 1986, Lee Iverson.
 */

#include <sys/types.h>
#include <sys/socket.h>

/*
 *	Error values returned by obtain_service(). [should check errno]
 */

#define NO_SERVER	(-1)
#define START_SERVER	(-2)

/*
 *	Special values returned by recv_server().
 */

#define RECV_ERROR	(-1)	/* Check errno */
#define RECV_DROPPED	(-2)	/* Server dropped us */

struct server {
    struct sockaddr addr;
    int addr_len;
    char *msg;
    int msg_len;
};

extern int server_socket();

extern void set_dropped_func(/* void (*)(char *, int) */);
extern void set_select_server(/* struct server *(*)(struct server *, int) */);

extern int obtain_service(/* char *, int, int */);

extern int recv_server(/* char *, int, int */);
extern int send_server(/* char *, int, int */);
extern int sign_off(/* char *, int, int */);
