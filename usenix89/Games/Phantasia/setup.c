/*
 * setup.c  Program to set up all files for Phantasia
 *
 *	This program tries to verify the parameters specified in
 *	the Makefile.  Since Phantasia assumes its files exist,
 *	simple errors can result in core dumps.
 *
 *	This program tries to check against this.
 * 
 *	Note that this is not fool-proof, and that this could
 *	be much more clever in checking everything.
 */

#include "phant.h"
#include <sys/types.h>
#include <sys/stat.h>

main(argc,argv) 		/* program to init. files for Phantasia */
int argc;
char	**argv;
{
FILE	*fp;
struct	stats	sbuf;
struct	nrgvoid grail;
struct	stat	fbuf;
register    int loop;
char	stbuf[128];

    srand((int) time(NULL));	/* prime random numbers */
    /* try to check RAND definition */
    for (loop = 1000; loop; loop--)
	{
	if (rand() > ((int) RAND))
	    {
	    sprintf(stbuf,"%.1f",(double) RAND);
	    Error("%s is a bad value for RAND.\n",stbuf);
	    }
	}
    umask(077);
    /* check where Phantasia lives */
    if (stat(PATH",&fbuf) < 0)
	{
	perror(PATH");
	exit(1);
	/*NOTREACHED*/
	}
    if (fbuf.st_mode & S_IFDIR == 0)
	Error("%s is not a directory.\n",PATH");
    /* try to create data files */
    if ((fp = fopen(goldfile,"w")) == NULL)
	Error("cannot create %s.\n",goldfile);
    else
	fclose(fp);
    if ((fp = fopen(motd,"w")) == NULL)
	Error("cannot create %s.\n",motd);
    else
	fclose(fp);
    if ((fp = fopen(messfile,"w")) == NULL)
	Error("cannot create %s.\n",messfile);
    else
	fclose(fp);
    /* do not reset character file if it already exists */
    if (stat(peoplefile,&fbuf) < 0)
	{
	buildchar(&sbuf);
	strcpy(sbuf.name,"<null>");
	if ((fp = fopen(peoplefile,"w")) == NULL)
	    Error("cannot create %s.\n",peoplefile);
	else
	    {
	    fwrite(&sbuf,sizeof(sbuf),1,fp);
	    fclose(fp);
	    }
	}
    grail.active = TRUE;
    grail.x = roll(-1.0e6,2.0e6);
    grail.y = roll(-1.0e6,2.0e6);
    if ((fp = fopen(voidfile,"w")) == NULL)
	Error("cannot create %s.\n",voidfile);
    else
	{
	fwrite(&grail,sizeof(grail),1,fp);
	fclose(fp);
	}
    if ((fp = fopen(lastdead,"w")) == NULL)
	Error("cannot create %s.\n",lastdead);
    else
	{
	fputs(" ",fp);
	fclose(fp);
	}
#ifdef ENEMY
    if ((fp = fopen(enemyfile,"w")) == NULL)
	Error("cannot create %s.\n",enemyfile);
    else
	fclose(fp);
#endif
    if ((fp = fopen(sbfile,"w")) == NULL)
	Error("cannot create %s.\n",sbfile)
    else
	fclose(fp);
    if (getuid() != UID)
	fprintf(stderr,"Warning: UID (%d) is not equal to current uid.\n",UID);
    exit(0);
}

buildchar(stat) 	    /* put in some default values */
/* Note that this function is almost the same as initchar().
   It is used to insure that unexpected values will not be found in a
   new character file.				*/
struct	stats	*stat;
{
    stat->x = roll(-125,251);
    stat->y = roll(-125,251);
    stat->exp = stat->lvl = stat->sin = 0;
    stat->crn = stat->psn = 0;
    stat->rng.type = NONE;
    stat->rng.duration = 0;
    stat->pal = FALSE;
    stat->hw = stat->amu = stat->bls = 0;
    stat->chm = 0;
    stat->gem = 0.1;
    stat->gld = roll(25,50) + roll(0,25) + 0.1;
    stat->quks = stat->swd = stat->shd = 0;
    stat->vrg = FALSE;
    stat->typ = 0;
}

Error(str,file)     /* print an error message, and exit */
char	*str, *file;
{
    fprintf(stderr,"Error: ");
    fprintf(stderr,str,file);
    exit(1);
    /*NOTREACHED*/
}
