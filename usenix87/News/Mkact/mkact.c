/* mkact - remake news active file for 2.10.2
 * vix 13may85 [written]
 *
 * usage:	$0 <oldactivefile >newactivefile
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>

#define	TRUE	1
#define	FALSE	0
#define MAXSTR	128
#define BASE	"/usr/spool/news/"
#define COMM	"ls -1F %s | sort -n"

main()
{
	char	line[MAXSTR], ng[MAXSTR], can[2], dir[MAXSTR], command[MAXSTR];
	long	maxart, minart;
	int	x, l;
	FILE	*sort, *popen();

	while (4 == fscanf(stdin, "%s %ld %ld %1s", ng, &maxart, &minart, can))
	{
		makedir(dir, ng);
		if (present(dir))
		{
			sprintf(command, COMM, dir);
			if (!(sort = popen(command, "r")))
				perror(command);
			else
			{
				for (l = 0;  fgets(line, MAXSTR-1, sort);  l++)
				{
					if (0 != (x = atoi(line)))
					{
						if (line[strlen(line)-2] != '/')
						{
							if (1 == l)
								minart = x;
							maxart = x;
						}
					}
				}
			}
			pclose(sort);
		}
		printf("%s %05d %05d %s\n", ng, maxart, minart, can);
	}
}

static makedir(dir, ng)
char	*dir, *ng;
{
	strcpy(dir, BASE);
	dir += strlen(BASE);
	while (*ng) {
		if (*ng == '.')
			*dir = '/';
		else
			*dir = *ng;
		dir++; ng++;
	}
	*dir = '\0';
}

static present(name)
char	*name;
{
	int	file;

	if (-1 == (file = open(name, O_RDONLY, 0)))
		return (FALSE);
	close(file);
	return (TRUE);
}
