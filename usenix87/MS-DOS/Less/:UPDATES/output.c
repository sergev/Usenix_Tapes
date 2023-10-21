/*
 * High level routines dealing with the output to the screen.
 */

#include "less.h"
#ifdef MSDOS
#include "scrn.h"
#endif

extern int sigs;
extern int sc_width, sc_height;
extern int ul_width, ue_width;
extern int so_width, se_width;
extern int tabstop;
extern int twiddle;
extern char *line;
extern char *first_cmd;
#ifdef MSDOS
extern int scrn_in_color;
#endif

/*
 * Display the line which is in the line buffer.
 */
	public void
put_line()
{
	register char *p;
	register int c;
	register int column;
	extern int auto_wrap, ignaw;

	if (sigs)
		/*
		 * Don't output if a signal is pending.
		 */
		return;

	if (line == NULL)
		line = (twiddle) ? "~" : "";

	column = 0;
	for (p = line;  *p != '\0';  p++)
	{
		switch (c = *p)
		{
		case UL_CHAR:
			ul_enter();
			column += ul_width;
			break;
		case UE_CHAR:
			ul_exit();
			column += ue_width;
			break;
		case '\t':
			do
			{
				putc(' ');
				column++;
			} while ((column % tabstop) != 0);
			break;
		case '\b':
			putbs();
			column--;
			break;
		default:
			if (c & 0200)
			{
#ifndef MSDOS
				putc('^');
				putc(c & 0177);
				column += 2;
#endif
			} else
			{
				putc(c);
				column++;
			}
		}
	}
	if (column < sc_width || !auto_wrap || ignaw)
		putc('\n');
}

/*
 * Is a given character a "control" character?
 * {{ ASCII DEPENDENT }}
 */
	public int
control_char(c)
	int c;
{
	return (c < ' ' || c == '\177');
}

/*
 * Return the printable character used to identify a control character
 * (printed after a carat; e.g. '\3' => "^C").
 * {{ ASCII DEPENDENT }}
 */
	public int
carat_char(c)
	int c;
{
	return ((c == '\177') ? '?' : (c | 0100));
}


static char obuf[1024];
static char *ob = obuf;

/*
 * Flush buffered output.
 */
	public void
flush()
{
#ifndef MSDOS
	write(1, obuf, ob-obuf);
	ob = obuf;
#else
	int chr_cnt;
	int i;

	chr_cnt = ob-obuf;	
	i = 0;
	do
	{
		if (scrn_in_color == 1)
			chr_put(*(obuf + i++), WHITE_ON_BLUE);
		else
			chr_put(*(obuf + i++), BW);
		--chr_cnt;
	}
	while (chr_cnt > 0);
	ob = obuf;
#endif
}

/*
 * Discard buffered output.
 */
	public void
dropout()
{
	ob = obuf;
}

/*
 * Output a character.
 */
	public void
putc(c)
	int c;
{
	if (ob >= &obuf[sizeof(obuf)])
		flush();
	*ob++ = c;
#ifdef MSDOS
	flush();
#endif
}

/*
 * Output a string.
 */
	public void
puts(s)
	register char *s;
{
#ifndef MSDOS
	while (*s != '\0')
		putc(*s++);
#else
	str_put(s, WHITE_ON_RED);
#endif
}

/*
 * Output a message in the lower left corner of the screen
 * and wait for carriage return.
 */

static char return_to_continue[] = "  (press RETURN)";

	public void
error(s)
	char *s;
{
	register int c;
	static char buf[2];

	lower_left();
	clear_eol();
	so_enter();
#ifndef MSDOS
	puts(s);
	puts(return_to_continue);
#else
	str_put(s, WHITE_ON_RED);
	str_put(return_to_continue, WHITE_ON_RED);
#endif	
	so_exit();

#if ONLY_RETURN
	while ((c = getc()) != '\n' && c != '\r')
		bell();
#else
	c = getc();
	if (c != '\n' && c != '\r' && c != ' ')
	{
		buf[0] = c;
		first_cmd = buf;
	}
#endif

	if (strlen(s) > sc_width)
		repaint();
}

	public int
error_width()
{
	/*
	 * Don't use the last position, because some terminals
	 * will scroll if you write in the last char of the last line.
	 */
	return (sc_width - 
		(sizeof(return_to_continue) + so_width + se_width + 1));
}

