#include "rv.h"

boolean input_mode;
INT  set_debug = 0;
boolean set_list;
boolean set_autoindent = FALSE;
boolean set_fortran;
INT  set_scroll;
INT  set_tabstops = 8;
#ifdef CRAY
boolean set_timeout = FALSE;
#else
boolean set_timeout = TRUE;
#endif
INT  set_shiftwidth = 8;
boolean set_wrapscan = TRUE;

void
cleanup()
{
	char buf[NUM_YANK_BUFS*20+8], *s;
	INT i;

	xmit_curline();
	refresh();

	/*
	 * Remove all yank buffers
	 */
	strcpy(buf, "!rm -f");
	s = &buf[strlen(buf)];
	yank_array[NUM_YANK_BUFS-1].ya_madefile = TRUE;
	for (i=0; i < NUM_YANK_BUFS; ++i)
		if (yank_array[i].ya_madefile) {
			sprintf(s, " /tmp/yk%d.%d", getpid(), i);
			s += strlen(s);
		}
	*s++ = '\n';
	*s = '\0';
	fputs(buf, file.fi_fpout);
}


void
quit()
{
	cleanup();
	xmit_sync();
	(void) recv_sync(TRUE);
	move(LINES-1,0);
	refresh();
	endwin();
	exit(0);
}


void
Quit()
{
	cleanup();
	xmit_sync();
	(void) recv_sync(TRUE);
	xmit_ed("Q\n");
	fflush(file.fi_fpout);
	move(LINES-1,0);
	refresh();
	endwin();
	exit(0);
}
