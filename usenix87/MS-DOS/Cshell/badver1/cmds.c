#include <stdio.h>
#include <fcntl.h>
typedef struct
{
	char *cmdname;
	int (*func)();
} builtin;

char *str_lower();

extern int result;

extern int cmds(),ls(), cp(), rm(), do_prog(),pushd(),popd(),drive(), ver(),
		more(),fgrep(),scr_clear(),set(),ch_mod(),cat(),echo(), 
		y(),t(),dump(),
		last(),invalid(),mv(),md(),touch(),cd(),pwd(),rd(),hist(),my_exit();

my_exit(argc,argv)
	char *argv[];
{
	exit(result);
}

ver()
{
	extern char *version;
	write(2,version,strlen(version));
	write(2,"\r\n",2);
}

extern builtin commands[];
extern int numcmds;
char *histerr = "no history";
int j,hiscount;
char *history[] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
					 NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

int histsize = (sizeof(history)/sizeof(char *));

cmds()
{
	char *current;
	register int i,j,col;
	col = 1;
	for (i = 0; i < numcmds; i++)
	{
		current = commands[i].cmdname;
		write(1,current,j = strlen(current));
		for (;j < 16;j++)
			write(1," ",1);
		if (col == 4)
		{
			col = 1;
			crlf();
		}
		else
			++col;
	}
	crlf();
}

findcmd(cmdbuf)
	char *cmdbuf;
{
	register int low,high,mid;
	char localbuf[256];
	int cond;
	strcpy(localbuf,cmdbuf);
	cmdbuf = str_lower(localbuf);
	low = 0;
	high = numcmds - 1;
	while (low <= high)
	{
		mid = (low+high) / 2;
		if	( ( cond =  strncmp( cmdbuf,
								 commands[mid].cmdname,
								 strlen(commands[mid].cmdname) ) ) < 0 )
				high = mid - 1;
		else if (cond > 0)
				low = mid + 1;
		else
		{
			/* kludge to allow for program invocations like d:command */
			if (cmdbuf[1] == ':')
				if (cmdbuf[2] == '\0')
					return mid;
				else
					return -1;
			return mid;
		}
	}
	return -1;
}

hist()
{
	register int i;
	char localbuf[256];
	if (j < histsize)
		i = 0;
	else
		i = j - histsize + 1;
	for (;i <= j; i++)
	{
		sprintf(localbuf,"%d : %s\r\n",i,history[i % histsize]);
		write(1,localbuf,strlen(localbuf));
	}
}

last()
{
	printf("return code of last command %d\n",result);
	return result;
}
