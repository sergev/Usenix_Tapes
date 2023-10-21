#include <stdio.h>
#include "parms.h"
#include "structs.h"

/* Edit a sequencer-file. */

#define EDIT 1
#define XTRACT 2
#define CREATE 3

struct seq_f entry;
FILE *fp, *tmp;
char tmpname[]="/tmp/edseqXXXXXX";
char *editor;
char line[80];

main(argc, argv)
char **argv;
{
    int i;
    int mode = EDIT;

    if ((editor=getenv("NFED"))==NULL)
	if ((editor=getenv("EDITOR"))==NULL)
	    editor=EDITOR;

    mktemp(tmpname);
    for (i=1; i<argc; i++)
    {
	if (argv[i][0] == '-')
	{
	    switch (argv[i][1])
	    {
		case 'e': mode=EDIT;    break;
		case 'x': mode=XTRACT;  break;
		case 'c': mode=CREATE;  break;
		default:  fprintf(stderr, "Unknown mode: -%c\n", argv[i][1]);
			exit(1);
	    }
	} else
	{
	    switch (mode)
	    {
		case XTRACT:
		    if ((fp=fopen(argv[i],"r"))==NULL)
		    {
			perror(argv[i]);
			break;
		    }
		    output(fp, stdout);
		    fclose(fp);
		    break;
		case CREATE:
		    if ((fp=fopen(argv[i],"w"))==NULL)
		    {
			perror(argv[i]);
			break;
		    }
		    input(fp, stdin);
		    fclose(fp);
		    break;
		case EDIT:
		    if ((fp=fopen(argv[i],"r+"))==NULL)
		    {
			perror(argv[i]);
			break;
		    }
		    if ((tmp=fopen(tmpname,"w+"))==NULL)
		    {
			perror(argv[i]);
			fclose(fp);
			break;
		    }
		    output(fp, tmp);
		    fflush(tmp);
		    fclose(fp);
		    sprintf(line,"%s %s", editor, tmpname);
		    system(line);
		    if ((fp=fopen(argv[i],"w"))==NULL)
		    {
			perror(argv[i]);
			fclose(tmp);
			unlink(tmpname);
			break;
		    }
		    fseek(tmp, 0l, 0);
		    input(fp, tmp);
		    fclose(fp);
		    fclose(tmp);
		    unlink(tmpname);
	    }
	}
    }
}

output(fp1, fp2)
FILE *fp1, *fp2;
{
    while (fread(&entry, sizeof(entry), 1, fp1))
    {
	fprintf(fp2, "%02d/%02d/%02d %02d:%02d  %s\n",
		     entry.lastin.w_year,
		     entry.lastin.w_month,
		     entry.lastin.w_day,
		     entry.lastin.w_hours,
		     entry.lastin.w_mins,
		     entry.nfname);
    }
}

input(fp1, fp2)
FILE *fp1, *fp2;
{
char buf[100];
int num;

    while (fgets(buf, sizeof(buf), fp2))
    {
	num = sscanf(buf, "%d/%d/%d %d:%d %s",
	       &entry.lastin.w_year,
	       &entry.lastin.w_month,
	       &entry.lastin.w_day,
	       &entry.lastin.w_hours,
	       &entry.lastin.w_mins,
	       entry.nfname);
	if (num==6)
	    fwrite(&entry, sizeof(entry), 1, fp1);
	else
	    fprintf(stderr, "Wrong syntax: %s", buf);
    }
}
