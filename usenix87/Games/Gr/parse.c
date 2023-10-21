#ifndef lint
static char *RCSid = "$Header: parse.c,v 1.5 86/12/02 20:28:02 mcooper Locked $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /big/src/usc/bin/gr/RCS/parse.c,v $
 * $Revision: 1.5 $
 * $Date: 86/12/02 20:28:02 $
 * $State: Exp $
 * $Author: mcooper $
 * $Locker: mcooper $
 *
 *------------------------------------------------------------------
 *
 * Michael A. Cooper
 * University Computing Services, 
 * University of Southern California
 * (mcooper@oberon.USC.EDU)
 *
 *------------------------------------------------------------------
 * $Log:	parse.c,v $
 * Revision 1.5  86/12/02  20:28:02  mcooper
 * "#ifdef DEBUG"'s all dprintf's.
 * 
 * Revision 1.4  86/07/17  16:15:45  mcooper
 * All fatal errors now use fatal().
 * 
 * Revision 1.3  86/07/14  16:00:40  mcooper
 * - Don't setup default logfile.
 * - Mostly de-linted.
 * 
 * Revision 1.2  86/07/14  15:55:30  mcooper
 * Added rcs headers.
 * 
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "gr.h"

extern int debug;

struct cfinfo *
parse(fd, game)
FILE *fd;
char *game;
{
	char lbuf[BUFSIZ], tmp[3][BUFSIZ];
	int config = FALSE;
	int foundname = FALSE;
	struct cfinfo cf;
	
	/*
	 * setup some defaults
	 */
	cf.cf_nofreettys = 2;
	cf.cf_load = 4.0;
	cf.cf_users = 8;
	(void) strcpy(cf.cf_spname, "Special");
	(void) strcpy(cf.cf_hidedir, "/usr/games/.hide");
	(void) strcpy(cf.cf_nogames, "/usr/games/nogames");

	while (fgets(lbuf, sizeof(lbuf), fd)) {
		if (lbuf[0] == COMMENT || lbuf[0] == '\n')
			continue;

		tmp[0][0] = NULL;
		tmp[1][0] = NULL;
		tmp[2][0] = NULL;

		(void) sscanf(lbuf, "%s%s%s", tmp[0], tmp[1], tmp[2]);
#ifdef DEBUG
		dprintf("tmp[0] = '%s' tmp[1] = '%s' tmp[2] = '%s'\n", 
			tmp[0], tmp[1], tmp[2]);
#endif DEBUG

		if (strncmp(tmp[0], "CONFIG", 6) == 0) {
			config = TRUE;
#ifdef DEBUG
			dprintf("found config\n");
#endif DEBUG
			continue;
		}

		if (config) {
			if (strncmp(tmp[0], "badttys", 7) == 0) {
				(void) strcpy(cf.cf_badttys, tmp[1]);
			}
			else if (strncmp(tmp[0], "freettys", 8) == 0) {
				(void) strcpy(cf.cf_freettys, tmp[1]);
				cf.cf_nofreettys = atoi(tmp[2]);
			}
			else if (strncmp(tmp[0], "spname", 6) == 0) {
				(void) strcpy(cf.cf_spname, tmp[1]);
			}
			else if (strncmp(tmp[0], "tod", 3) == 0) {
				cf.cf_todmorn = atoi(tmp[1]);
				cf.cf_todeven = atoi(tmp[2]);
			}
			else if (strncmp(tmp[0], "debug", 5) == 0) {
				if (strcmp(tmp[1], "on") == 0) 
					debug = TRUE;
				else
					debug = FALSE;
			}
			else if (strncmp(tmp[0], "hidedir", 7) == 0) {
				if(tmp[1][0] == NULL) {
					fatal("No hiden directory defined.");
					/*NOTREACHED*/
				}
				(void) strcpy(cf.cf_hidedir, tmp[1]);
			}
			else if (strncmp(tmp[0], "nogames", 7) == 0) {
				(void) strcpy(cf.cf_nogames, tmp[1]);
			}
			else if (strncmp(tmp[0], "logfile", 7) == 0) {
				if(tmp[1][0] == NULL) {
					fatal("Keyword \"logfile\" set, but not defined.");
					/*NOTREACHED*/
				}
				(void) strcpy(cf.cf_logfile, tmp[1]);
			}
			continue;
		} else if (!foundname) {
			if ((strcmp(tmp[0], game) == 0)||(strcmp(tmp[0],"default") == 0)){
#ifdef DEBUG
				dprintf("parse: found game name\n");
#endif DEBUG
				foundname = TRUE;
				(void) sscanf(lbuf, "%s%lf%d%d", 
					cf.cf_game, &cf.cf_load, &cf.cf_users, &cf.cf_priority);
			} 
#ifdef DEBUG
			else dprintf("word = '%s'\n");
#endif DEBUG
		}
	}
#ifdef DEBUG
	dprintf("\nCF: game = '%s' load = %.2f users = %d prio = %d\n", 
		cf.cf_game, cf.cf_load, cf.cf_users, cf.cf_priority);
	dprintf("cf_badttys = '%s' cf_free = '%s' \ncf_nofree = %d\n\n",
		cf.cf_badttys, cf.cf_freettys, cf.cf_nofreettys);
	dprintf("cf_hidedir = '%s' cf_nogames = '%s'\ncf_logfile = '%s'\n",
		cf.cf_hidedir, cf.cf_nogames, cf.cf_logfile);
	dprintf("cf_spname = '%s'\n", cf.cf_spname);
	dprintf("debug is %d\n", debug);
#endif DEBUG
	return(&cf);
}
