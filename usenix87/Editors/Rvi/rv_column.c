#include "rv.h"

/*
 * Logical to/from physical column conversion functions
 */

INT
file_column(s, maxscreen_col)
/*
 * Convert a screen column number to a file column number.
 */
register char	*s;
register INT maxscreen_col;
{
	register INT	c;
	register INT	file_col, screen_col;

	if (s == NULL) {
		errflag = 1;
		botprint(TRUE, "file_column - text is null.\n");
		return 0;
	}

	file_col = 0;
	screen_col = 0;
	while (c = *s++) {
		if (c < ' ' || c > '~') /* control character */
			if (c == '\t' && !set_list) {
				screen_col += set_tabstops -
					(screen_col % set_tabstops) - 1;
			} else
				++screen_col;
		++file_col;
		++screen_col;
		if (screen_col > maxscreen_col)
			break;
	}

	return file_col <= 0 ? 0 : file_col-1;
}



INT
screen_column(s, maxfile_col)
/*
 * Convert a file column number to a screen column number.
 */
register char	*s;
register INT maxfile_col;
{
	register INT	c;
	register INT	file_col, screen_col;

	if (s == NULL) {
		errflag = 1;
		botprint(TRUE, "screen_column - text is null.\n");
		return 0;
	}

	file_col = 0;
	screen_col = 0;
	while (c = *s++) {
		if (c < ' ' || c > '~') /* control character */
			if (c == '\t' && !set_list) {
				if (input_mode && file_col >= maxfile_col) {
					++screen_col;
					break;
				}
				screen_col += set_tabstops - 
					(screen_col % set_tabstops) - 1;
			} else
				++screen_col;
		++file_col;
		++screen_col;
		if (file_col > maxfile_col)
			break;
	}

	return screen_col <= 0 ? 0 : screen_col-1;
}
