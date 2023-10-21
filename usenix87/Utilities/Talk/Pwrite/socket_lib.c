
#include "header.h"


/*****************************************************************************
 *
 *	SCOKET_ID = make_a_socket();
 *
 *		make_a_socket() creates a socket local to the system and
 *		binds the name FILE_NAME to it. This function is called
 *		by new_server_socket() and new_client_socket(). A SOCKET_ID
 *		is returned on success, -1 if not.
 *
 *****************************************************************************/

make_a_socket()
{
	int	SOCKET_ID;

	errno = 0;
	if ((SOCKET_ID = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		return(-1);
	}

	return(SOCKET_ID);
}


/*****************************************************************************
 *
 *	SOCKET_ID = s_new_server(FILE_NAME);
 *
 *		s_new_server() creates a socket ready for connection
 *		by connect_server(). This function returns a SOCKET_ID
 *		if successful, otherwise -1.
 *	
 *****************************************************************************/

s_new_server(FILE_NAME)
char	*FILE_NAME;
{
	int	SOCKET_ID;
	struct	sockaddr SOCKET_INFO;

	errno = 0;
	if ((SOCKET_ID = make_a_socket()) < 0)
	{
		return(-1);
	}

	SOCKET_INFO.sa_family = AF_UNIX;
	strcpy(SOCKET_INFO.sa_data, FILE_NAME);
	unlink(SOCKET_INFO.sa_data);
	errno = 0;
	if (bind(SOCKET_ID, &SOCKET_INFO, sizeof(SOCKET_INFO)) < 0)
	{
		return(-1);
	}

	chmod(SOCKET_INFO.sa_data, 0666);
	listen(SOCKET_ID,5);
	return(SOCKET_ID);
}


/*****************************************************************************
 *	
 *	SOCKET_ID = s_new_client();
 *
 *		s_new_client() creates and returns a SOCKET_ID ready
 *		ready for connrction using connect_client(). A -1 is returned
 *		if something messed up.
 *	
 *****************************************************************************/

s_new_client()
{
	int	SOCKET_ID;

	errno = 0;
	if ((SOCKET_ID = make_a_socket()) < 0)
	{
		return(-1);
	}

	return(SOCKET_ID);
}


/*****************************************************************************
 *
 *	s_connect_server(SOCKET_ID, FILE_NAME);
 *
 *		s_connect_server() connects the already created SOCKET_ID
 *		with a UNIX file. -1 is returned if the connection cannot
 *		be made. Otherwise, the filename of the client's socket
 *		placed in FILE_NAME and a SOCKET_ID file descriptor is
 *		for the newly accpeted socket is returned.
 *	
 *****************************************************************************/

s_connect_server (SOCKET_ID, FILE_NAME)
int	SOCKET_ID;
char	*FILE_NAME;
{
	int	NEW_SOCKET;
	int	namelen;
	struct	sockaddr SOCKET_INFO;

	namelen = sizeof(SOCKET_INFO);
	errno = 0;
	if ((NEW_SOCKET = accept(SOCKET_ID, &SOCKET_INFO, &namelen)) < 0)
	{
		return(-1);
	}

	strcpy(FILE_NAME,SOCKET_INFO.sa_data);
	return(NEW_SOCKET);
}


/*****************************************************************************
 *	
 *	s_connect_client(SOCKET_ID, FILE_NAME);
 *
 *		s_connect_client() connects a SOCKET_ID already created with
 *		new_client_socket() and connects it (with the UNIX file
 *		FILE_NAME). Zero is returned upon success, -1 if you're not
 *		so fortunate.
 *	
 *****************************************************************************/

s_connect_client (SOCKET_ID, FILE_NAME)
int	SOCKET_ID;
char	*FILE_NAME;
{
	int	namelen;
	struct	sockaddr SOCKET_INFO;

	namelen = sizeof(SOCKET_INFO);
	SOCKET_INFO.sa_family = AF_UNIX;
	strcpy(SOCKET_INFO.sa_data, FILE_NAME);
	errno = 0;
	return(connect(SOCKET_ID, &SOCKET_INFO, namelen));
}


/*****************************************************************************
 *
 *	s_select(MASK);
 *
 *****************************************************************************/

s_select (MASK)
int	MASK;
{
	int	mask;

	mask = MASK;
	select(20, &mask, 0, 0, &TIMEOUT);
	return(mask);
}


/*****************************************************************************
 *
 *	s_read(SOCKET_ID, STRING, NUM_CHARS);
 *
 *		s_read() reads NUM_CHARS bytes from the socket SOCKET_ID
 *		and places them in STRING. This function returns -1 if
 *		reading is not possible from SOCKET_ID, this means that
 *		the socket has been closed down from the other end.
 *		A SOCKET_ID of 0 will read from standard input. If no
 *		problems occur, the number of characters actually read
 *		from the socket is returned.
 *
 *****************************************************************************/

s_read(SOCKET_ID,STRING,NUM_CHARS)
int	SOCKET_ID;
char	*STRING;
int	NUM_CHARS;
{
	int	num_read;

	if (s_select(SOCKET_ID))
	{
		if ((num_read = read(SOCKET_ID, STRING, NUM_CHARS)) <= 0)
		{
			return(-1);	/* Socket was closed from other side */
		}
		STRING[num_read] = NULL;
		return(num_read);
	}

	return(0);
}


/*****************************************************************************
 *
 *	s_write(SOCKET_ID, STRING);
 *
 *		s_write() writes a NULL terminated STRING to the socket
 *		specified by SOCKET_ID. The number of characters written to
 *		the socket is returned. -1 is returned something goes wrong.
 *
 *****************************************************************************/

s_write(SOCKET_ID,STRING)
int	SOCKET_ID;
char	*STRING;
{
	return(write(SOCKET_ID, STRING, strlen(STRING)));
}


/*****************************************************************************
 *
 *	s_delete(SOCKET_ID);
 *
 *		s_delete() closes down a socket, as specified by
 *		SOCKET_ID;
 *
 *****************************************************************************/

s_delete(SOCKET_ID)
int SOCKET_ID;
{
	errno = 0;
	shutdown(SOCKET_ID, 2);
	return(close(SOCKET_ID));
}

