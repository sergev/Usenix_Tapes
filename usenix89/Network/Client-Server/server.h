/*
 *	server.h: Copyright 1986, Lee Iverson.
 */

#include <sys/types.h>
#include <sys/socket.h>

#define NO_SERVICE	(-1)
#define NO_SOCKET	(-2)
#define NO_BIND		(-3)

extern int allow_new_clients;

extern int setup_server(/* char *, int */);

extern void set_new_client_message(/* char *, int */);
extern void set_new_client_func(/* void (*)(int, char *, int) */);
extern void set_dead_client_func(/* void (*)(int, char *, int) */);

extern int  recv_client(/* char *, int *, int */);
extern void send_client(/* int, char *, int, int */);
extern void drop_client(/* int, char *, int, int */);

extern void for_each_client(/* void (*)(int) */);
