
# include <stdio.h>
# include "typedefs.h"
# include "globals.h"
# include "string2.h"

FILE	*popen();

/*
 *  Check command to see if it's OK
 */
ok_command(line)
char line[];
{
	int	i;
	char	*c;

	c = line;
	while (*c != ';') c++;
	*c = '\0';
	for (i=0; i<=num_cmds; i++)
	{
		if ((strindex(line,comlist[i]) != -1) && 
			(strcmp(line,"") != 0))
		{
			return(0);
		}
	}
	return(-1);
}

/*
 * Commands.c -- Does @ checking
 *
 * Called by check_commands()
 *
 */
do_command(line)
char	line[];
{
	char *c;
	FILE *fp, *fopen();
	char	name[80];
	char	buffer[80], buffer2[80];
	int	proc_id, result;
	int	status;
	char comline[80];

	line[strlen(line)-1] = '\0';
	if (strncmp(line,"@RR",3) == 0)
	{
		sprintf(namet,"/usr/ucb/Mail -s \"Return Receipt\" %s",fromline);
		fp = popen(namet,"w");
		fprintf(fp,"\nHere is the return receipt you requested.\n\n");
		pclose(fp);
	}
	if ((strncmp(line,"@SH",3) == 0) && 
	    (strcmp(subline,"Command") == 0))
	{
		c = line; c += 4;
		strcpy(buffer,c);
		buffer[strlen(buffer)+1] = '\0';
		buffer[strlen(buffer)] = ';';
		if (ok_command(buffer) == 0)
		{
			sprintf(buffer2,"%s | /usr/ucb/Mail -s",buffer);
			sprintf(buffer2,"%s \"%s\" %s",buffer2,line,fromline);
			proc_id = fork();
			if (proc_id == 0)
			{ execlp("/bin/sh","sh","-c",buffer2,(char *)0); 	}
			wait(&status);
		} else
		{
			sprintf(namet,"/usr/ucb/Mail -s \"MEP\" %s",fromline);
			fp = popen(namet,"w");
			fprintf(fp,"\nYou do not have access to the command:\n");
			fprintf(fp,"\n%s\n",buffer);
			pclose(fp);
		}
	}
}

check_commands()
{
	FILE *md, *fopen();
	char	buffer[512];
	
	md = fopen(namem,"r");
	fgets(buffer,512,md);
	while (!feof(md))
	{
		if (buffer[0] == '@') do_command(buffer);
		fgets(buffer,512,md);
	}
	fclose(md);
}
