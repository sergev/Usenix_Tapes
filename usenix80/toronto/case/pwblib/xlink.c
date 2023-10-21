char xlink__[] "~|^`xlink.c 1.3";

/*
	Interface to link(II) which handles all error conditions.
*/

xlink(f1,f2)
{
	extern errno;

	if (link(f1,f2))
		fatal(stringf("error %d linking `%s' to `%s' (330)",errno,f2,f1));
}


/*
	Interface to unlink(II) which handles all error conditions.
*/

xunlink(f)
{
	extern errno;

	if (unlink(f))
		fatal(stringf("error %d unlinking `%s' (331)",errno,f));
}

