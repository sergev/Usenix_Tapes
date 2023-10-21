/*T bpatch - A binary file patch/dump utility */
/*S Introduction */
/*F bpatch ***********************************************************
* bpatch
* by Garry M Johnson - 09/17/84
* (C) Copyright 1984, 1985
*
* Change History:
*  03/05/86 - added further terminal independence
*             added use of ioctl (see main and mstdin)
*             added -D versus -d command line option
*             added use of standard getopt
*             cleaned up code, eliminated function "ikf"
*             added original versions of ezlib functions, such as
*               icc, setterm, cm, mstdin, erase, length, move
*             added ^R, ^Q, ^N, and ^P commands
*             added ^F, !, and ^X (-X) commands
*			  changed name to "bpatch"
*			  added direct address command (g)
*             added ASCII search capability
*  07/07/86 - converted to use curses
*             modified direct addressing to use suffixes
*             updated HELP function
*
*   Steven List @ Benetics Corporation, Mt. View, CA
*   {cdp,engfocus,idi,oliveb,plx,tolerant}!bene!luke!itkin
*********************************************************************/
/*E*/
/*S includes, globals, and defines */
/*Page Eject*/
#include	<curses.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

static int pbrk = 0;
struct stat sb;

void icc ();
void copyrec ();
void schwapp ();

	/* ------------------------------------------------------------ */
	/* Some defines added by the moderator to get it work on 4.2    */
	/* ------------------------------------------------------------ */
#ifdef	MOD_HAX
    /* Fifos?  We ain't got no steenkin' fifos. */
#define S_IFIFO		123450
    /* "Spelling differences." */
#define beep()		fprintf (stderr, "\007")
#define cbreak()	crmode()
    /* Our curses doesn't translate keypad keys to single characters. */
#define keypad(a, b)	/* null */
#define KEY_UP		'^'
#define KEY_DOWN	'v'
#define KEY_LEFT	'<'
#define KEY_RIGHT	'>'
#define KEY_HOME	'@'
#endif	/* MOD_HAX */


	/* ------------------------------------------------------------ */
	/* Some convenient defines                                      */
	/* ------------------------------------------------------------ */

#define DEL '\177'
#define HEX 1
#define ALPHA 0

	/* ------------------------------------------------------------ */
	/* general purpose identification and control variables         */
	/* ------------------------------------------------------------ */

char	filename[64];			/* current file being examined		*/
char	record[16][16];			/* record (page) buffer				*/
char	unch_rec[16][16];		/* record before any changes		*/
int 	zp;						/* current input character			*/

int		block = 0;				/* block size if -b in command		*/
int		block_spec;				/* true if file is block special	*/
int		bytes = 0;				/* number of bytes from last read	*/
int		char_spec;				/* true if file is char special		*/
int		debug = 0;				/* true if debug is turned on		*/
int		dir_spec;				/* true if file is directory		*/
int		dump = 0;				/* nonzero if dump instead of change*/
int		ebcdic = 0;				/* true if -e option				*/
int		fifo_spec;				/* true if file is fifo				*/
int		honly = 0;				/* true if dump is to be hex only	*/
int		mod = 0;				/* true if record has been modified	*/
int		pause = 0;				/* true if -p option				*/
int		rawfile = 0;			/* true if file is c/b/p			*/
int		reclen = 0;				/* record length, if -r				*/
int		recno = 0;				/* current record (page) number		*/
int		stay = 0;				/* true if no position change 		*/
int		swab = 0;				/* true if byte swapping is on		*/
int		windowed = 0;			/* true if windowing - not dump		*/

long	position = 0;			/* byte address in file				*/

WINDOW *hexwin = NULL;
WINDOW *alphawin = NULL;
WINDOW *errwin = NULL;

/*S main - control all the work from here */
/*H main *************************************************************
*
*                            main
*
*	set up the globals, initilize the state, and process the file
*
*********************************************************************/
/*E*/
main (argc, argv)
int argc;
char *argv[];
{
	extern WINDOW *subwin ();
	extern WINDOW *newwin ();

#ifdef	MOD_HAX
#else	/* use original code... */
	struct termio asis;
#endif	/* MOD_HAX */

	register char	*cp;				/* general purpose char ptr	*/
	extern   char	*gets ();			/* get string from stdin	*/
	         char	m = '\017';			/* mask for hex edit		*/
	         char	response[512];		/* general purpose buffer	*/
	         int 	z;					/* character read in		*/
         
	int		breakp ();			/* signal trapping function			*/
	int		c;					/* current screen column			*/
	int		change = 0;			/* true if cmd line option toggled	*/
	int		fid;				/* file descriptor					*/
	int		firstfile;			/* arg # of first file in cmd line	*/
	int		h;					/* temp for hex edit				*/
	int		i;					/* general purpose loop index		*/
	int		j;					/* general purpose loop index		*/
	int		r;					/* current screen row				*/
	int		hexc;				/* current cursor column in hexwin	*/

	long	byteaddr;			/* planned byte address for 'G'		*/
	long	size;				/* file size in bytes				*/
	long	status;				/* EOF if at end of file or error	*/

	extern int optind;			/* getopt index into argv			*/
	extern char *optarg;		/* getopt pointer to opt arg		*/

	extern long getnum ();
	extern char *instr ();		/* get a string from the cmd line	*/
	extern int reset ();		/* exit function - reset terminal	*/

	/* ------------------------------------------------------------ */
	/* set up signal handling                                       */
	/* ------------------------------------------------------------ */

	if (!dump) signal (SIGINT, breakp);

	signal (SIGTERM, reset);

	/* ------------------------------------------------------------ */
	/* process command line arguments                               */
	/* ------------------------------------------------------------ */

	while ((i = getopt (argc, argv, "r:dD:b:pxXse")) != EOF)
	{
		switch (i)
		{
			case	'b':		/* blocking					*/
				block = atoi (optarg);
				if (block < 1 || block > 10240)
				{
					fprintf (stderr,
						"invalid block size: %d\n", block);
					exit (1);
				}
				break;
			case	'd':		/* straight dump - no limit	*/
				dump = -1;
				break;
			case	'D':		/* dump - page count spec	*/
				dump = atoi (optarg);
				break;
			case	'e':		/* file is ebcdic			*/
				ebcdic = 1;
				break;
			case	'p':		/* pause between pages - dump	*/
				pause = 1;
				break;
			case	'r':		/* record length for dump		*/
				reclen = atoi (optarg);
				break;
			case	's':		/* byte swapping required		*/
				swab = 1;
				break;
			case	'x':		/* hex dump only				*/
				honly = 1;
				break;
			case	'X':
				debug = 1;
				break;
			default:			/* uhoh							*/
				fprintf (stderr,
"usage: bpatch [ -b blocksz ] [ -d<ump> ] [ -D pagecnt ] [ -e<bcdic> ]\n");
				fprintf (stderr,
"              [ -p<ause> ] [ -r reclen ] [ -s<wap bytes> ] [ -x<only> ]\n");
				exit (1);
		}
	}

	/* ------------------------------------------------------------ */
	/* check for valid combinations of options                      */
	/* ------------------------------------------------------------ */

	if ((honly || block || reclen || pause ) && !dump)
	{
		fprintf (stderr, "-x|-b|-r|-p requires -d or -D\n");
		exit (2);
	}

	/* ------------------------------------------------------------ */
	/* At least one file name must be specified on the cmd line     */
	/* ------------------------------------------------------------ */

	if (optind == argc)
	{
		fprintf (stderr, "no file name(s) specified\n");
		exit (2);
	}

	/* ------------------------------------------------------------ */
	/* set up the screen, if this is an interactive session         */
	/* ------------------------------------------------------------ */

	if (!dump)
	{
		windowed = 1;
		initscr ();
		nonl ();
		noecho ();
		cbreak ();
		keypad (stdscr, TRUE);
		hexwin = subwin (stdscr, 16, 48, 4, 4);
		keypad (hexwin, TRUE);
		alphawin = subwin (stdscr, 16, 16, 4, 57);
		keypad (alphawin, TRUE);
		errwin = subwin (stdscr, 1, 80, 23, 0);

#ifdef	MOD_HAX
		/* This is not exactly what the original code does,
		   but it's good enough.  -r$ */
		raw();
#else	/* use original code... */
		ioctl (0, TCGETA, &asis);
		asis.c_cc[VINTR] = '\0';
		asis.c_iflag &= ~IXON;
		asis.c_iflag &= ~IXOFF;
		asis.c_iflag &= ~IXANY;
		ioctl (0, TCSETA, &asis);
#endif	/* MOD_HAX */
	}

	/* ------------------------------------------------------------ */
	/* save the first file's index for backing up later             */
	/* ------------------------------------------------------------ */

	firstfile = optind;

	/* ------------------------------------------------------------ */
	/* open the first file                                          */
	/* ------------------------------------------------------------ */

	for (fid = -1; fid < 0 && optind < argc;)
	{
		fid = ckfile (argv[optind], &size);
		if (fid < 0) optind++;
	}
	if (fid < 0)
	{
		fprintf (stderr, "could not handle the file list\n");
		exit (2);
	}

	strncpy (filename, argv[optind], sizeof filename);

	if (block != 0)
	{
		size = -1;
	}

	recno = 0;
	stay = 0;
	mod = 0;
	status = 0;

	/* ------------------------------------------------------------ */
	/* Until the user exits...                                      */
	/* ------------------------------------------------------------ */

	if (!dump) clear ();

	while (status != EOF)
	{
	/* ------------------------------------------------------------ */
	/* change of location - read and display                        */
	/* ------------------------------------------------------------ */
		if (stay == 0)
		{
			position = lseek (fid, (long)(recno * 256), 0);

			if ((bytes = bread (fid, record, 256, block)) < 0)
			{
				errmsg ("error on reading file %s", filename);
				status = EOF;
				continue;
			}
			if (bytes > 0)
			{
				if (swab) schwapp (record, 256);

				copyrec (record, unch_rec, sizeof record);

				show (bytes, record, filename, size, recno,
						position, m,reclen, dump, ebcdic, swab,
						block, honly);
			}
			mod = 0;
		}
	/* ------------------------------------------------------------ */
	/* not interactive - keep dumping or open next file             */
	/* ------------------------------------------------------------ */
		if (dump)
		{
			if ((dump < 0 && bytes == 0) || (--dump == 0))
			{
				if (optind == argc) status = EOF;
				else
				{
					close (fid);
					fid = -1;
					for (optind++; fid < 0 && optind < argc;)
					{
						fid = ckfile (argv[optind], &size);
						if (fid < 0) optind++;
					}

					strncpy (filename, argv[optind], sizeof filename);

					if (block != 0)
					{
						size = -1;
					}
					recno = 0;
					stay = 0;
					status = lseek (fid, (long)0, 0);
				}
			}
			++recno;
	/* ------------------------------------------------------------ */
	/* if pause, beep and wait                                      */
	/* ------------------------------------------------------------ */
			if (status != EOF && pause)
			{
				pbrk = 0;
				fprintf (stderr, "\007");
				gets (response);

				if (pbrk) status = EOF;
			}

			continue;
		}
	/* ------------------------------------------------------------ */
	/* if we got here, this is an interactive session               */
	/* ------------------------------------------------------------ */
		stay = 0;
		move (22, 0);
	/* ------------------------------------------------------------ */
	/* get the user's command                                       */
	/* ------------------------------------------------------------ */
		response[0] = EOF;
		mvaddstr (22, 0, "> ");
		clrtoeol ();
		refresh ();
		zp = getch ();

		if (debug && !dump)
		{
			if (isascii (zp) && isprint (zp))
				errmsg ("command entered is %c", zp);
			else errmsg ("command entered is ^%c (%#x)", zp + '@', zp);
			getch ();
		}

		if (isascii (zp) && isalpha (zp) && islower (zp))
			zp = toupper (zp);
	/* ------------------------------------------------------------ */
	/* here we go - what does the user want?                        */
	/* ------------------------------------------------------------ */
		refresh ();

		switch (zp)
		{ 
			case	'!':			/* shell escape				*/
				echo ();
				move (23,0);
				clrtoeol ();
				addstr ("shell command: ");
				refresh ();
				getstr (response);
				erase ();
				refresh ();
				nl ();
				system (response);
				noecho ();
				nonl ();
				move (23,0);
				standout ();
				addstr (" <Press any key> ");
				standend ();
				clrtoeol ();
				refresh ();
				getch ();
				clear ();
				break;

			case	'?':			/* HELP						*/
				dbg_msg ("Help");
				dohelp ();
				touchwin (stdscr);
				refresh ();
				stay = 1;
				break;

			case	'/':			/* search for a string		*/
				stay = 1;
				if (mod)
				{
					errmsg ("No write since last change");
				}
				else search (fid, zp);
				break;

			case '-':				/* toggle options			*/
				zp = getch ();
				stay = 1;
				change = 0;
				switch (zp)
				{
					case 'a': /* ascii */
						if (ebcdic)
						{
							dbg_msg ("toggle to ascii");
							change = 1;
						}
						ebcdic = 0;
						break;

					case 'e': /* ebcdic */
						if (ebcdic == 0)
						{
							dbg_msg ("toggle to ebcdic");
							change = 1;
						}
						ebcdic = 1;
						break;

					case 's': /* swab */
						dbg_msg ("toggle byte swap");
						change = 1;
						schwapp (record, 256);
						swab = !swab;
						break;
				}
				if (change)
				{
					show (bytes, record, filename, size,
							recno, position, m,reclen, dump,
							ebcdic, swab, block, honly);
				}

				break;

			case '\022':			/* redraw screen (^R)		*/
				clear ();
				show (bytes, record, filename, size, recno,
					  position, m, reclen, dump, ebcdic,
					  swab, block, honly);
				stay = 1;
				break;
			
			case	'\030':			/* toggle debug (^X)		*/
				debug = !debug;
				break;

			case	'\006':			/* new file (^F)			*/
				close (fid);
				fid = ckfile (cp = instr (), &size);
				if (fid < 0)
				{
					fid = ckfile (filename, &size);
				}
				else
				{
					strncpy (filename, cp, sizeof filename);
					stay = 0;
					recno = 0;
				}
				break;

			case	'\016':			/* next file (^N)			*/
				if (mod)
				{
					errmsg ("No write since last change");
					stay = 1;
				}
				else if (optind == (argc - 1))
				{
					errmsg ("No more files");
					stay = 1;
				}
				else
				{
					close (fid);
					for (fid = -1, optind++; fid < 0 && optind < argc;)
					{
						fid = ckfile (argv[optind], &size);
						if (fid < 0) optind++;
					}
					if (fid < 0)
					{
						errmsg ("could not handle the file list");
						reset (0);
					}
					strncpy (filename, argv[optind], sizeof filename);
					stay = 0;
					recno = 0;
				}
				break;

			case	'\020':			/* prev file (^P)			*/
				if (mod)
				{
					errmsg ("No write since last change");
					stay = 1;
				}
				else if (optind == firstfile)
				{
					errmsg ("No previous file");
					stay = 1;
				}
				else
				{
					close (fid);
					for (fid = -1, optind--; fid < 0 && optind >= firstfile;)
					{
						fid = ckfile (argv[optind], &size);
						if (fid < 0) optind--;
					}
					if (fid < 0)
					{
						errmsg ("could not handle the file list");
						reset (0);
					}
					strncpy (filename, argv[optind], sizeof filename);
					stay = 0;
					recno = 0;
				}
				break;

			case	'\021':		/* quit absolutely (^Q)		*/
				status = EOF;
				break;

			case DEL:			/* quit with check			*/
			case 'Q': /* quit */
				if (mod)
				{
					errmsg ("No write since last change!");
					stay = 1;
				}
				else
				{
					status = EOF;
				}
				break;

			case '\\': /* back up 1 record */
				if (mod)
				{
					errmsg ("No write since last change");
					stay = 1;
				}
				else
				{
					if (recno > 0)
					{
						--recno;
						stay = 0;

						status = lseek (fid, (long)recno*256, 0);
						if (status < 0)
						{
							move (22, 0);
							clrtoeol ();
							perror (filename);
							errmsg ("error positioning in file");
							beep ();
							++recno;
							stay = 1;
						}
					}
					else
					{
						errmsg ("No previous records");
						beep ();
						stay = 1;
					}
				}
				break; 

			case 'F': /* go to first record */
				if (mod)
				{
					errmsg ("No write since last change");
					stay = 1;
				}
				else
				{
					status = lseek (fid, (long)0, 0);
					recno = 0;
				}
				break;

			case 'L': /* go to last record */
				if (mod)
				{
					errmsg ("No write since last change");
					stay = 1;
				}
				else
				{
					position = lseek (fid, (long)0, 2);
					recno = position / 256;
					j = position % 256;
					if (j == 0) --recno;
					status = lseek (fid, (long)(recno*256), 0);
				}
				break;

			case 'U':			/* undo changes					*/
				stay = 1;
				mod = 0;
				copyrec (unch_rec, record, sizeof record);
				show (bytes, record, filename, size, recno,
						position, m,reclen, dump, ebcdic, swab,
						block, honly);
				break;

			case 'R': /* re-read record */
				status = lseek (fid, (long)recno*256, 0);
				break;

			case '0': /* go to some address */
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (mod)
				{
					errmsg ("No write since last change");
					stay = 1;
				}
				else
				{
					byteaddr = getnum (zp, FALSE);
					stay = 1;
					errmsg ("Position to byte %ld", byteaddr);
					if (!rawfile && byteaddr > size)
					{
						errmsg ("Address outside file");
						beep ();
					}
					else if (byteaddr / 256 != recno)
					{
						recno = byteaddr / 256;
						status = lseek (fid, (long)recno * 256, 0);
						stay = 0;
					}
				}
				break;

			case 'A': /* alpha modify */
				stay = 1;
				r = c = 0;
				dbg_msg ("edit ascii");
				if (bytes == 0) break;
				touchwin (stdscr);
				refresh ();
				wmove (alphawin, r, c);
				wrefresh (alphawin);

				while ((z = wgetch (alphawin)) != DEL)
				{
					if (!arrow (z, &r, &c))
					{
						if (isascii (z))
						{
							if (isprint (z)) waddch (alphawin, z);
							else             waddch (alphawin, '.');

							if (ebcdic) icc (&z, 1,"AE");

							record[r][c] = z;
							mod = 1;

							hexc = c * 3;
							wmove (hexwin, r, hexc);
							if (record[r][c] < '\0')
							{
								wprintw (hexwin, "%x%x",
									(record[r][c] >> 4) & m,
									record[r][c] & m);
							}
							else
							{
								wprintw (hexwin, "%02x", record[r][c]);
							}
							wrefresh (hexwin);

						}
						else
						{
							beep ();
						}

						if (c == 15)
						{
							if (r == 15) beep ();
							else
							{
								c = 0;
								++r;
							}
						}
						else
						{
							c++;
						}
					}
					if (r * 16 + c >= bytes)
					{
						beep ();
						r = (bytes - 1) / 16;
						c = (bytes - 1) % 16;
					}

					wmove (alphawin, r, c);
					wrefresh (alphawin);
				}

				break;

			case 'H': /* hex modify */
				dbg_msg ("edit hex");
				stay = 1;
				r = c = hexc = 0;
				if (bytes == 0) break;
				touchwin (stdscr);
				refresh ();
				wmove (hexwin, r, hexc);
				wrefresh (hexwin);

				while ((z = wgetch (hexwin)) != DEL)
				{
					if (!arrow (z, &r, &c))
					{
						hexc = c * 3;
						z = toupper (z);
						if (!isxdigit (z))
						{
							beep ();
						}
						else
						{
							waddch (hexwin, tolower (z));
							wrefresh (hexwin);

							if (z > '9') z -= 7;

							h = (z & m) << 4;

							while (2)
							{
								z = EOF;
								z = getch ();
								if (z == EOF)
								{
									pbrk = 0;
									h = -1;
									break;
								}
								z = toupper (z);
								if (!isxdigit (z))
								{
									beep ();
								}
								else
								{
									waddch (hexwin, tolower (z));
									wrefresh (hexwin);
									if (z > '9') z -= 7;

									h |= z & m;
									break;
								}
							}

							if (h < 0)
							{
								wmove (hexwin, r, hexc);
								if (record[r][c] < '\0')
								{
									wprintw (hexwin, "%x%x",
										(record[r][c] >> 4) & m,
										record[r][c] & m);
								}
								else
								{
									wprintw (hexwin, "%02x", record[r][c]);
								}
								wrefresh (hexwin);
								break;
							}

							record[r][c] = z = h;
							mod = 1;

							if (ebcdic) icc (&z, 1,"EA");

							wmove (alphawin, r, c);
							if (isascii (z) && isprint (z))
								waddch (alphawin, z);
							else waddch (alphawin, '.');
							wrefresh (alphawin);

							if (c == 15)
							{
								if (r == 15) beep ();
								else
								{
									c = 0;
									++r;
								}
							}
							else
							{
								c++;
							}
						}
					}

					if (r * 16 + c >= bytes)
					{
						beep ();
						r = (bytes - 1) / 16;
						c = (bytes - 1) % 16;
					}

					hexc = c * 3;
					wmove (hexwin, r, hexc);
					wrefresh (hexwin);
				}
				break;

			case 'W': /* write record */
				stay = 1;
				status = lseek (fid, position, 0);
				if (status != position)
				{
					move (22, 0);
					clrtoeol ();
					perror (filename);
					errmsg ("error positioning in file");
					beep ();
				}
				if (swab) schwapp (record, 256);
				if (write (fid, record, bytes) != bytes)
				{
					errmsg ("error writing to file");
					sleep (1);
					reset (0);
					exit (0);
				}
				if (swab) schwapp (record, 256);
				mod = 0;
				errmsg ("Record written");
				break;

			case	'\n':		/* newline - next page			*/
			case	'\r':
				if (mod)
				{
					errmsg ("No write since last change");
					stay = 1;
				}
				else
				{
					++recno;
					if (!rawfile && (recno * 256) >= size)
					{
						recno--;
						beep ();
						errmsg ("No more records in file");
						stay = 1;
					}
					else stay = 0;
				}
				break;

			default:
				if (isascii (zp) && isprint (zp))
					errmsg ("Unknown command: %d", zp);
				else
					errmsg ("Unknown command: %d", zp + '@');
				beep ();
				stay = 1;
				break;
		} /* end switch zp */
		refresh ();
	}

	if (windowed)
	{
		reset (0);
	}
	status = close (fid);

	exit (status);
}
/*S show - display a record on the terminal */
/*H show */
/*E*/
show (bytes, record, filename, size, recno, position,
	  m,reclen, dump, ebcdic, swab, block, honly)
int bytes;
int size;
int recno;
int position;
int m;
int reclen;
int dump;
int ebcdic;
int swab;
int honly;
char record[16][16];
char *filename;
{
	int		i;
	int		j;
	char	s;
	char	temp[16];
	char	*look = NULL;

	int		row = 0;
	int		col = 0;

	if (dump) printf ("\n\n");

	if (debug)
	{
		getyx (stdscr, row, col);
		move (23,0);
		printw ("show: %d|%d|%s|%d|%d|%d|%#x|%d|%d|%d|%d|%d|%d",
			bytes, record, filename, size, recno, position,
			m, reclen, dump, ebcdic, swab, block, honly);
		move (row, col);
		row = col = 0;
	}

	if (!dump) move (0, 0);
	outstr ("FILE: %s ", filename);
	if (block_spec) outstr ("(block special)");
	else if (char_spec) outstr ("(character special)");
	else if (fifo_spec) outstr ("(fifo (named pipe))");
	else if (dir_spec) outstr ("(directory - %ld)", size);
	else outstr ("(%ld)", size);

	if (ebcdic) outstr (" - EBCDIC");
	else outstr (" - ASCII");
	if (swab) outstr (" - SWAP");
	if (block) outstr (" - BLOCK (%d)", block);
	if (reclen) outstr (" - RECORD (%d)", reclen);

	if (!dump)
	{
		clrtoeol ();
		move (1,0);
		printw ("PAGE: %d (%d)", recno, position);
		clrtoeol ();
		row = 2;
	}
	else
	{
		printf ("\nPAGE: %d (%d)\n", recno, position);
	}

	if (honly)
	{
		look = (char *) record;
		for (j=0;j<256;++j)
		{
			if (*look++ != '\0')
			{
				look = NULL;
				break;
			}
		}
	}

	if (!dump) move (row, col);

	outstr ("    x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf");
	outstr ("      0123456789abcdef");

	if (!dump)
	{
		row += 2;
		move (row, col);
	}
	else
	{
		printf ("\n\n");
	}
	for (i=0; i<=bytes/16; ++i)
	{
		if (honly && look != NULL)
		{
			i = 16;
			continue;
		}
		if (i*16+1 > bytes) continue;

		outstr ("%02x: ", i);

		for (j=0; j<16; ++j)
		{
			if (i*16+j < bytes)
			{
				if (record[i][j] < '\0')
					outstr ("%x%x ",
						(record[i][j] >> 4) & m, record[i][j] & m);
				else
					outstr ("%02x ", record[i][j]);

				s = ' ';
				if (reclen > 0 && (position+i*16+j+1)%reclen == 0)
					s = ':';
				if (block > 0 && (position+i*16+j+1)%block == 0)
				{
					if (s == ' ')
						s = '/';
					else
						s = '%';
				}
				if (s != ' ')
					outstr ("\b%c", s);
			}
			else
			{
				outstr ("   ");
			}
		}

		outstr ("     ");

		copyrec (record[i], temp, 16);

		if (ebcdic) icc (temp, 16, "EA");

		for (j = 0; j < 16 && i*16+j < bytes; ++j)
		{
			if (temp[j] < ' ') outch ('.');
			else outstr ("%c", temp[j]);
		}

		if (!dump)
		{
			move (++row, col);
		}
		else
		{
			printf ("\n");
		}
	}

	if (!dump)
	{
		clrtobot ();
		refresh ();
	}

	return;
}
/*S breakp - set pbrk on interrupt */
/*H breakp */
/*E*/
int breakp (i)
int i;
{
	int s;
	extern int pbrk;
	s = (int) signal (SIGINT, breakp);
	pbrk = i;
}
/*S bread - buffered read */
/*H bread */
/*E*/
int bread (fid, record, want, block)
int fid, want, block;
char *record;
{
	int i, j, k;
	int what, bytes, orig;
	static char buffer[10240];
	static int left, arrow;
	static int flag = 1;

	if (flag)
	{
		left = 0;
		arrow = 0;
		flag = 0;
	}

	if (block == 0)
		return (read (fid, record, want));

	if (block & 1) ++block;

	orig = what = want;
	while (1)
	{
		if (left < want)
		{
			if (left)
			{
				copyrec (&buffer[arrow], record, left);
				record += left;
				want -= left;
			}

			arrow = 0;
			left = 0;

			if ((bytes = read (fid, buffer, block)) < 0)
			{
				what = bytes;
				break;
			}

			if (bytes == 0)
			{
				what = orig - want;
				break;
			}

			left = bytes;
		}
		else
		{
			copyrec (&buffer[arrow], record, want);
			arrow += want;
			left -= want;
			break;
		}
	}

	return (what);
}
/*S schwapp - swap bytes in place */
/*H schwapp */
/*E*/
void
schwapp (ptr, nch)
register char *ptr;
register int nch;
{
	register int i;
	register char c;
	register char *ptra = ptr + 1;

	if (nch & 1) --nch;

	for (i = 0; i < nch; i += 2, ptr += 2, ptra += 2)
	{
		c = *ptr;
		*ptr = *ptra;
		*ptra = c;
	}
	return;
}
/*S copyrec - transfer bytes from f to t for nbytes bytes */
/*H copyrec */
/*E*/
void
copyrec (f, t, nbytes)
register char *f;
register char *t;
register int nbytes;
{
	register int i;

	for (i = 0; i < nbytes; i++, f++, t++) *t = *f;

	return;
}
/*S ebcdic codes corresponding to ascii - translation table */
/*Page Eject*/
char ebcdic_codes[] = {
0, 0x1, 0x2, 0x3, 0x37, 0x2d, 0x2e, 0x2f, 0x16,
0x5, 0x25, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11,
0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, 0x18, 0x19, 0x3f,
0x27, 0x1c, 0x1d, 0x1e, 0x1f, 0x40, 0x5a, 0x7f, 0x7b,
0x5b, 0x6c, 0x50, 0x7d, 0x4d, 0x5d, 0x5c, 0x4e, 0x6b,
0x60, 0x4b, 0x61, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
0xf6, 0xf7, 0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e,
0x6f, 0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8,
0xe9, 0xad, 0xe0, 0xbd, 0x9a, 0x6d, 0x79, 0x81, 0x82,
0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92,
0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0xa2, 0xa3,
0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0,
0x5f, 0x7 };
/*S icc - internal code conversion */
/*H icc */
/*E*/
void
icc (buf, nch, type)
register char *buf;
register int nch;
char *type;
{
	register int i;
	register int j;

	if (!strcmp (type, "AE"))
	{
		for (i = 0; i < nch; i++, buf++)
		{
			*buf = ebcdic_codes[*buf];
		}
	}
	else if (!strcmp (type, "EA"))
	{
		for (i = 0; i < nch; i++, buf++)
		{
			for (j = 0; j < 128; j++)
			{
				if (*buf == ebcdic_codes[j])
				{
					*buf = j;
					break;
				}
			}
		}
	}

	return;
}
/*S ckfile - check on existence, accessibility, and type of file */
/*H ckfile */
/*E*/
ckfile (filename, sizep)
char	*filename;
long	*sizep;
{
	register int fid = 0;

	if (access (filename, 0) < 0)
	{
		errmsg ("file not found (%s)", filename);
		fid = -1;
	}

	if (block || pause || dump)
	{
		fid = open (filename, O_RDONLY, 0);
	}
	else
	{
		fid = open (filename, O_RDWR, 0);
	}

	if (fid < 0)
	{
		errmsg ("error opening file %s", filename);
		perror (filename);
		fid = -1;
	}
	else
	{
		errmsg ("File %s opened successfully ", filename);
		if (fstat (fid, &sb) == -1)
		{
			fprintf (stderr, "Can't stat\n");
			perror (filename);
			fid = -1;
		}
		else
		{
			block_spec = (sb.st_mode & S_IFMT) == S_IFBLK;
			char_spec = (sb.st_mode & S_IFMT) == S_IFCHR;
			fifo_spec = (sb.st_mode & S_IFMT) == S_IFIFO;
			dir_spec = (sb.st_mode & S_IFMT) == S_IFDIR;
			rawfile = block_spec || char_spec || fifo_spec;

			if (rawfile) *sizep = -1;
			else
			{
				if (sb.st_size == 0)
				{
					fprintf (stderr,
						"file %s is empty (zero bytes)\n",
						filename);
					fid = -1;
				}
				*sizep = sb.st_size;
			}
		}
	}

	return fid;
}
/*S dohelp - display help text */
/*H dohelp */
/*E*/
dohelp ()
{
	static char *helptxt[] = {
	"!   - execute command in the shell",	"a   - edit ascii portion",
	"-x   - toggle command line option",	"f   - display first page of file",
	"<cr> - display next page",	NULL,
	"?    - display this help text",	"h   - edit hexadecimal portion",
	"DEL  - quit the program",		"l   - display last page of file",
	"\\    - display previous page",		"nnn - direct addressing",
	"/    - search for ASCII string",		"q   - quit the program",
	"^f   - select named file",		"r   - reread the current page",
	"^n   - select next file",		"u   - undo all changes to page",
	"^p   - select previous file",		"w   - write out changed page",
	"^q   - quit without writing changes",	NULL,
	"^r   - redraw the screen",	NULL,
	"^x   - turn on debug",	NULL,
	"----------------------------------------------------------------", NULL,
	"direct addressing: nnnS, where nnn = some number, and", NULL,
	"                      S = type suffix", "b = block (512)",
	NULL, "k = kilobyte (1024)",
	NULL, "l = long word (4)",
	NULL, "p = page (256)",
	NULL, "w = word (2)",
	NULL, "<cr> = byte",
	};

	static int nmsg = sizeof helptxt / sizeof (char *);
	register int row = 0;
	register int i;

	register WINDOW *helpwin;
	extern WINDOW *newwin ();

	helpwin = newwin (LINES, COLS, 0, 0);
	werase (helpwin);
	wrefresh (helpwin);

	wmove (helpwin, 0, 1);
	waddstr (helpwin,
"---------------------------------- HELP ----------------------------------");

	for (row = 1, i = 0; i < nmsg; i+=2)
	{
		if (helptxt[i])
		{
			wmove (helpwin, row, 1);
			waddstr (helpwin, helptxt[i]);
		}
		if (i+1 <= nmsg && helptxt[i+1])
		{
			wmove (helpwin, row, 41);
			waddstr (helpwin, helptxt[i+1]);
		}
		row++;
	}

	wmove (helpwin, 23, 0);
	wstandout (helpwin);
	waddstr (helpwin, " <Press any key> ");
	wstandend (helpwin);
	wclrtoeol (helpwin);
	wrefresh (helpwin);
	wgetch (helpwin);
	werase (helpwin);
	wrefresh (helpwin);
	delwin (helpwin);

	return;
}
/*S reset - reset terminal to original state */
/*H reset */
/*E*/
reset (sig)
int sig;
{
	move (23, 0);
	refresh ();
	endwin ();
	if (sig) fprintf (stderr, "killed with signal %d\n", sig);
	exit (sig);
}
/*S arrow - determine if current character is a cursor control key */
/*H arrow */
/*E*/
arrow (k, r, c, type)
register int k;
register int *r;
register int *c;
register int type;
{
	register ret = 1;

	if (k == KEY_UP)
	{
		if (*r == 0) beep ();
		else (*r)--;
	}
	else if (k == KEY_DOWN || k == '\n')
	{
		if (*r == 15) beep ();
		else (*r)++;
	}
	else if (k == KEY_LEFT || k == '\b' )
	{
		if (*c == 0)
		{
			if (*r == 0) beep ();
			else
			{
				*c = 15;
				(*r)--;
			}
		}
		else (*c)--;
	}
	else if (k == KEY_RIGHT)
	{
		if (*c == 15)
		{
			if (*r == 15) beep ();
			else
			{
				*c = 0;
				(*r)++;
			}
		}
		else (*c)++;
	}
	else if (k == KEY_HOME)
	{
		*r = *c = 0;
	}
	else
	{
		ret = 0;
	}

	return ret;
}
/*S dbg_msg - print a debug message */
/*H dbg_msg */
/*E*/
dbg_msg (msg)
register char *msg;
{
	if (debug && !dump)
	{
		errmsg (msg);
	}

	return;
}
/*S instr - get a character string from the terminal */
/*H instr */
/*E*/
char *
instr ()
{
	static char buf[512];

	register int c;
	register char *p = buf;
	register int col = 0;

	move (22, 0);
	clrtoeol ();
	refresh ();

	while ((c = getch ()) != '\r')
	{
		if (isascii (c) && isprint (c))
		{
			move (22, col);
			addch (c);
			*p++ = c;
			col++;
		}
		else if (c == '\b')
		{
			p--;
			col--;
			move (22, col);
			addch (' ');
			move (22, col);
		}
		refresh ();
	}

	refresh ();

	*p = '\0';

	return buf;
}
/*S getnum - retrieve a number from the terminal */
/*H getnum */
/*E*/
long
getnum (frst_char, hex)
register int frst_char;
register int hex;
{
	static char buf[64];

	register int c;
	register char *p = buf;
	register int col = 0;

	register long retval = 0;

	move (22, 0);
	clrtoeol ();
	if (frst_char)
	{
		addch (frst_char);
		*p++ = frst_char;
		col++;
		refresh ();
	}

	while ((c = getch()) != '\r')
	{
		if (isascii (c))
		{
			if ((hex && isxdigit (c)) || isdigit (c))
			{
				move (22, col);
				addch (c);
				*p++ = c;
				col++;
			}
			else if (c == '\b')
			{
				p--;
				col--;
				move (22, col);
				addch (' ');
				move (22, col);
			}
			else
			{
				break;			/* some character typing the value	*/
			}
			refresh ();
		}
	}

	*p = '\0';

	retval = atol (buf);

	mvprintw (22, 0, "%ld", retval);
	switch (c)
	{
		case	'b':			/* block - 512 bytes				*/
			retval *= 512;
			break;
		case	'k':			/* 1024 bytes						*/
			retval *= 1024;
			break;
		case	'l':			/* long word - 4 bytes				*/
			retval *= 4;
			break;
		case	'p':			/* page - 256 bytes					*/
			retval *= 256;
			break;
		case	'w':			/* word - 2 bytes					*/
			retval *= 2;
			break;
		case	'\r':			/* just clear it for display		*/
			c = '\0';
			break;
	}

	printw ("%c -> %ld byte offset", c, retval);
	clrtoeol ();

	refresh ();

	return retval;
}
/*S search - look for an ascii string in the file */
/*H search */
/*E*/
search (fid)
register int fid;
{
	long	curpos = position;
	long	currec = recno;

	char	lrecord[sizeof record + 1];

	register int i;
	register int matched = 0;
	register int srch_len;

	register char *cp = instr ();
	register char *rp;

	int row, col;

	srch_len = strlen (cp);
	copyrec (record, lrecord, sizeof record);
	lrecord[256] = '\0';

	pbrk = 0;

	wmove (errwin, 0, 0);
	wstandout (errwin);
	wmove (errwin, 0, 40);
	wstandend (errwin);
	wmove (errwin, 0, 1);
	waddstr (errwin, "..searching record ");
	getyx (errwin, row, col);

	do
	{
		mvwprintw (errwin, row, col, "%ld", currec);
		wrefresh (errwin);

		for (i = 0, rp = lrecord, matched = 0;
			 !matched && i < 256;
			 rp++, i++)
		{
			if (*cp == *rp && !strncmp (cp, rp, srch_len))
			{
				matched = 1;
				break;
			}
		}

		if (!matched)
		{
			bytes = bread (fid, lrecord, 256, block);
			currec++;
			lrecord[256] = '\0';
		}
	}	while (!pbrk && bytes && !matched);

	if (matched)
	{
		recno = currec;
		stay = 0;
		copyrec (record, unch_rec, sizeof record);
		werase (errwin);
		wrefresh (errwin);
	}
	else
	{
		errmsg ("String %s not found", cp);
		stay = 1;
	}

	return;
}
errmsg (fmt, arg1, arg2, arg3, arg4)
register char *fmt;
register char *arg1;
register char *arg2;
register char *arg3;
register char *arg4;
{
	int y, x;

	if (!dump)
	{
		getyx (stdscr, y, x);
		wmove (errwin, 0, 0);
		wstandout (errwin);
		wprintw (errwin, fmt, arg1, arg2, arg3, arg4);
		wstandend (errwin);
		wclrtoeol (errwin);
		wrefresh (errwin);
		move (y, x);
	}
	else
	{
		fprintf (stderr, fmt, arg1, arg2, arg3, arg4);
	}
	return;
}
outstr (fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
register char *fmt;
register char *arg1;
register char *arg2;
register char *arg3;
register char *arg4;
register char *arg5;
register char *arg6;
register char *arg7;
{
	if (dump) printf (fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	else printw (fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7);

	return;
}
outch (ch)
register char ch;
{
	if (dump) putchar (ch);
	else addch (ch);

	return;
}
