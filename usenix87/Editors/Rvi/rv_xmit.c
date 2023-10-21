#include "rv.h"

void
xmit_curline()
/*
 * Transmit the current line, if modified
 */
{
	register struct sc_screen *sc;
	register struct li_line  *line;

	sc = &screen;
	if (sc->sc_origline.li_text != NULL) {
		free(sc->sc_origline.li_text);
		sc->sc_origline.li_text = NULL;
		line = sc->sc_curline;
		xmit_ed("%dc\n", sc->sc_lineno);
		if (strcmp(line->li_text, ".") == 0)
			fputs("\\.", file.fi_fpout);
		else
			fputs(line->li_text, file.fi_fpout);
		fputs("\n.\n", file.fi_fpout);
		file.fi_modified = TRUE;
		if (set_debug > 1)
			fputs("\007", stderr);
	}
}


/*VARARGS1*/
void
xmit_ed(txt, arg1, arg2, arg3, arg4, arg5)
/*
 * Printf the string to ed
 */
char *txt, *arg1, *arg2, *arg3, *arg4, *arg5;
{
	char buf[512];

	sprintf(buf, txt, arg1, arg2, arg3, arg4, arg5);
	fputs(buf, file.fi_fpout);
}
