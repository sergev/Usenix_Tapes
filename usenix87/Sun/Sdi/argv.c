/******************************************  argv.c  *******************/
#include <sunwindow/notify.h>
#include <stdio.h>
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * Handle command line arguments, and do some font fiddling.
 */

process_args(argc, argv)
int argc;
char **argv;
{
	extern int time_to_play, 
			starting_icon,
			starting_icon_time,
			starting_level,
			starting_skill,
			restoring_game;
	while (--argc > 0) {
		argv++;
		if ((*argv)[0] != '-') {
			fprintf(stderr, "Unrecognizable argument '%s'.\n", *argv);
			exit(1);
		} else {
			switch((*argv)[1]) {
				case 'b': {
					/* name of a file with a blast pixrect to use */
					struct pixrect *pr, *icon_load_mpr();
					char error_msg[256], *oldv = *argv;
					argc--; argv++;
					if (! out_of_args(argc, oldv)) {
						if ((pr = icon_load_mpr(*argv, error_msg)) == NULL) {
							printf("Could not get pr '%s'.\n", *argv);
							printf("%s",error_msg);
						} else {
							switch(oldv[2]) {
		/* cities */			case 'c': change_circ(citykillcircles, pr); break;
		/* lasers */			case 'l': change_circ(lasercircles, pr); break;
		/* laser ends */		case 'k': change_circ(laserkillcircles, pr); break;
		/* missile death */		case 'm': change_circ(blastkillcircles, pr); break;
		/* interceptors */		case 'b': change_circ(bigblastcircles, pr); break;
								default: {
									printf("unrecognized option: '%s'.\n",oldv);
									exit(1);
								}
							}
						}
					}
					break;
				}
				case 'c': {
					/* name of a file with a city pixrect to use */
					argc--; argv++;
					if (! out_of_args(argc, "-c")) {
						init_city_bits(*argv);
					}
					break;
				}	
				case 'd': {
					/* delay to use between screen updates */
					argc--; argv++;
					if (! out_of_args(argc, "-d")) {
						blast_delay = atol(*argv);
					}
					break;
				}
				case 'f': {
					/* new score file */
					argc--; argv++;
					if (! out_of_args(argc, "-f")) {
						scorefile = *argv;
					}
					break;
				}	
				case 'g': {
					extern int gamemaster;
					/* set gamemaster mode */
					gamemaster = 1;
					break;
				}
				case 'h': {
					/* height of playing windows */
					argc--; argv++;
					if (! out_of_args(argc, "-h")) {
						max_y = max(MINWIN, atol(*argv));
					}
					break;
				}	
				case 'i': {
					/* starting icon type */
					argc--; argv++;
					if (! out_of_args(argc, "-i")) {
						starting_icon = atol(*argv);
						starting_icon = min(2, max(0, starting_icon));
					}
					/* starting icon time */
					argc--; argv++;
					if (! out_of_args(argc, "-i")) {
						starting_icon_time = atol(*argv);
						starting_icon_time = min(50, max(1, starting_icon_time));
					}
					break;
				}
				case 'p': {
					/* select the pointer style */
					extern int cursor_type;
					argc--; argv++;
					if (! out_of_args(argc, "-p")) {
						cursor_type = atol(*argv);
						cursor_type = min(2, max(0, cursor_type));
					}
					break;
				}
				case 'r': {
					/* restore a saved game */
					argc--; argv++;
					if (! out_of_args(argc, "-r")) {
						strcpy(save_file_name, *argv);
						restoring_game = 1;
					}
					break;
				}
				case 's': {
					/* starting skill*/
					argc--; argv++;
					if (! out_of_args(argc, "-s")) {
						starting_skill = atol(*argv);
						starting_skill = min(2, max(0, starting_skill));
					}
					break;
				}
				case 't': {
					/* maximum playing time, in seconds */
					argc--; argv++;
					if (! out_of_args(argc, "-t")) {
						time_to_play = atol(*argv);
					}
					break;
				}
				case 'w': {
					/* width of playing windows */
					argc--; argv++;
					if (! out_of_args(argc, "-w")) {
						max_x = max(MINWIN, atol(*argv));
					}
					break;
				}	
				default: {
					fprintf(stderr, "Unrecognizable argument '%s'.\n", *argv);
					break;
				}
			}
		}
	}
}


/* the following kludge is necessary because there seems to be no
 * SunView subroutine which sets the default font for all the windows.
 * WIN_FONT explicitly disclaims doing this.  Only -Wt on the argument
 * claims to....
 */
fixup_font(pargc, pargv, font_name)
int *pargc;
char ***pargv;
char *font_name;
{
	int i;
	int argc = *pargc;
	char **argv = *pargv;
	char **new_argv = (char **)calloc(sizeof(char *), argc+3);
	for (i = 1; i < argc; i += 1) {
		new_argv[i+2] = argv[i];
	}
	new_argv[0] = argv[0];
	new_argv[1] = "-Wt";
	new_argv[2] = font_name;
	*pargc += 2;
	*pargv = new_argv;
}

open_our_font(s)
char *s;
{
	if ((font = (struct pixfont *)pf_open(s)) == NULL)
		font = (struct pixfont *)pf_default();
}

out_of_args(n, s)
char *s;
{
	if (n > 0) {
		return 0;
	} else {
		fprintf(stderr, "Argument '%s' not followed by a value.\n", s);
		/* return 1; */
		exit(1);
	}
}
