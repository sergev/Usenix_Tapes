65c
		if (suibyte(on, c) < 0)
			u.u_error = ENXIO;
.
34c
		if ((c = fuibyte(on)) < 0)
			u.u_error = ENXIO;
.
9a
 *
 *	modified by D A Gwyn on 25-Apr-1980:
 *	1) catch error returns from fuibyte, suibyte.
.
3,6c
 * mem.c - memory special file
.
w
q
