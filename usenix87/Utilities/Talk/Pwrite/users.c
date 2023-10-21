
#include "header.h"

FILE *fp;
struct utmp ubuf;

p_setup_me()
{
	CURRENT_WINDOW = NUM_T = NUM_C = 0;
	SNOOPER = -1;
	ACTIVE_MASK = 0 | (1 << 0);	/* 0 is stdin		*/

	strcpy(T[0].__tty,rindex(ttyname(0),'/') + 4);
	strcpy(T[0].__username,getenv("USER"));
	sprintf(T[0].__file,"/tmp/.pw_%s",T[0].__tty);
	if ((T[0].__sock = s_new_server(T[0].__file)) < 0)
		panic("Error creating your socket.",0);

	T[0].__connected = 1;
	w_new_user(0);
	NUM_T++; NUM_C++;
	ACTIVE_MASK |= (1 << T[0].__sock);
}

p_delete(n)
int	n;
{
	int	a;

	if (T[n].__connected)
		NUM_C--;

	delwin(T[n].__win1);
	delwin(T[n].__win2);

	if (SMART)
	{
		wclear(T[n].__win1);
		wclear(T[n].__win2);
		wrefresh(T[n].__win1);
		wrefresh(T[n].__win2);
	}
	else
	{
		printf("\nUser '%s' on 'tty%s' has disconnected%c\n",T[n].__username,T[n].__tty,7);
	}

	if (T[n].__sock >= 0)
	{
		ACTIVE_MASK &= ~(1 << T[n].__sock);
		s_delete(T[n].__sock);
	}
	strcpy(T[n].__tty,"");
	strcpy(T[n].__username,"");
	strcpy(T[n].__file,"");
	T[n].__sock = -1;
	T[n].__dead = 1;
	T[n].__connected = 0;

	if (NUM_C < 1 ||  n == 0)
		goodbye();

	if (n == SNOOPER)
		SNOOPER = -1;

	if (n == CURRENT_WINDOW)
		w_nextcorners();
}

p_answer()
{
	char	t[8], u[32];
	int	a, c, s;

	strcpy(STRING,"");
	s = s_connect_server(T[0].__sock,STRING);

	if (s < 0)
		return(-1);

	a = read(s,STRING,BUFSIZ);	/* "j6|puchyr|"	*/
	STRING[a] = NULL;
	STRING[2] = NULL;
	strcpy(t,STRING);
	aslstr(STRING,3);
	for (a=0; STRING[a] != '|'; u[a] = STRING[a++]);
	u[a] = NULL;
	a = p_add(t,u,s,1);
	w_new_user(a);
	sprintf(STRING,"%c\n\"%s\" ANS \"%s\".%c",BEGIN_SECRET,T[0].__tty,T[a].__tty,END_SECRET);
	p_sendstr(STRING);
	NUM_C++;
}

p_add(t,u,s,c)
char	*t, *u;
int	s, c;
{
	int	a;

	if (NUM_C >= MAX_USERS)
	{
		panic("You cannot add any more.",4);
		return(0);
	}

	for (a=1; a < NUM_T; a++)
	{
		if (T[a].__dead)
			break;
	}

	sprintf(T[a].__file,"/tmp/.pw_%s",t);
	strcpy(T[a].__tty,t);
	strcpy(T[a].__username,u);
	T[a].__dead = 0;
	T[a].__secret = 0;
	T[a].__sock = s;
	T[a].__connected = c;

	if (c)
		ACTIVE_MASK |= (1 << T[a].__sock);

	if (a >= NUM_T)
		NUM_T++;

	CURRENT_WINDOW = a;
	return(a);
}

p_beg()
{
	int	a;

	for (a=0; a < NUM_T; a++)
	{
		if (!T[a].__dead && !T[a].__connected)
		{
			if (T[a].__sock < 0)
				T[a].__sock = s_new_client();

			if (s_connect_client(T[a].__sock,T[a].__file) >= 0)
			{
				sprintf(STRING,"%s|%s|",T[0].__tty,T[0].__username);
				s_write(T[a].__sock,STRING);
				T[a].__connected = 1;
				delwin(T[a].__win1);
				delwin(T[a].__win2);
				w_new_user(a);
				sprintf(STRING,"%c\n\"%s\" GOT \"%s\".%c",BEGIN_SECRET,T[0].__tty,T[a].__tty,END_SECRET);
				p_sendstr(STRING);
				NUM_C++;
				ACTIVE_MASK |= (1 << T[a].__sock);
			}
		}
	}
}

p_who()
{
	if ((fp = fopen("/etc/utmp","r")) == NULL)
	{
		panic("Can't open /etc/utmp. (Major Bug!!)\n",0);
	}

	for (NUM_W=0; fread((char *) &ubuf, sizeof(ubuf), 1, fp) == 1; )
	{
		if (*ubuf.ut_name)
		{
			strcpy(W[NUM_W].W_username,ubuf.ut_name);
			W[NUM_W].W_tty[0] = ubuf.ut_line[3];
			W[NUM_W].W_tty[1] = ubuf.ut_line[4];
			W[NUM_W].W_tty[2] = NULL;
			NUM_W++;
		}
	}

	fclose(fp);
}

p_check(s,how)
char	*s;
int	how;
{
	int	a, l;

	if (how)
	{
		p_who();
		if ((l = strlen(s)) <= 2)
		{
			for (a=0; a < NUM_W; a++)
			{
				if (!strcmp(W[a].W_tty,s))
					break;
			}
		}
		else
		{
			for (a=0; a < NUM_W; a++)
			{
				if (!strcmp(W[a].W_username,s))
					break;
			}
		}

		if (a < NUM_W)
			return(a);
		else
			return(-1);
	}
	else
	{
		if ((l = strlen(s)) <= 2)
		{
			for (a=0; a < NUM_T; a++)
			{
				if (!strcmp(T[a].__tty,s))
					break;
			}
		}
		else
		{
			for (a=0; a < NUM_T; a++)
			{
				if (!strcmp(T[a].__username,s))
					break;
			}
		}

		if (a < NUM_T)
			return(a);
		else
			return(-1);
	}
}

p_sendch(c)
int	c;
{
	int	a;

	for (a=1; a < NUM_T; a++)
		if (T[a].__connected && !T[a].__dead && T[a].__sock >= 0)
			write(T[a].__sock,&c,1);
}

p_sendstr(s)
char	*s;
{
	int	a;

	for (a=1; a < NUM_T; a++)
		if (T[a].__connected && !T[a].__dead && T[a].__sock >= 0)
			write(T[a].__sock,s,strlen(s));
}

p_secretcommand(n,c)
int	n, c;
{
	char	temp[16];
	int	a;

	switch(c)
	{
		case  23:
				sprintf(STRING,"%c\n\"%s\" HAS",BEGIN_SECRET,T[0].__tty);
				for (a=0; a < NUM_T; a++)
				{
					if (!T[a].__dead)
					{
						strcat(STRING," \"");
						strcat(STRING,T[a].__tty);
						strcat(STRING,"\"");
					}
				}
				sprintf(temp,".%c",END_SECRET);
				strcat(STRING,temp);
				write(T[n].__sock,STRING,strlen(STRING));
				break;
	}
}

