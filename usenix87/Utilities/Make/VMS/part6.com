:
>One more patch is needed:
>Turns out, VAX11C uses a non-standard format for picking
>up #include files from SYS$LIBRARY:CSYSDEF.TLB. Thus, if
>you want to be compatible, you should set up your header
>#include's like this:
>	#ifdef VAX11C
>	#include stdio.h
>	#else
>	#include <stdio.h>
>	#endif
>The bracketed version will pick up files out in
>SYS$LIBRARY itself; at our site, there are a set of files
>out there belonging to Whitesmith's C, but certainly not
>all the ones called out in the program were there, and
>I don't think that Whitesmith's <stdio.h> is compatible
>with DEC's runtime library.

	It is true that VAX11C uses a weird format, but such use is entirely
optional, and I never use it.  All of my #included files are either distributed
with MAKE, or are provided with VAX11C.  No such patch is necessary.

	It has been remarked that the routine IFERR is missing.  Funny, I could
swear I generated the entire thing from the distribution copy before submitting
it, but I guess weird things happen.  Let me give you my version:




iferr (status)
int *status;
/*
 * Check the given VMS status value, and abort if an error is indicated.
 */
{
	if ((*status & 0x1) == 0)
		lib$signal (*status);
}



>PS: I am slightly troubled by the use of an established name
>    for an incompatible program (cf the discussion about the
>    rights to the name 'rn' for a newsreader). We have a VMS
>    and a UNIX system side by side, and we run EUNICE on our
>    VMS system also. Thus we already have a "make" on our
>    VMS system. Well, at least the 1MAKEFILE has a different
>    file name.

	I guess I see your point.  I just know that I needed a tool to give
me the same sort of functionality under VMS, so I wrote this thing for myself.
If you have spent any amount of time switching between systems, you know how
nice it is to be able to type the same thing in every place; thus I called it
make.  If you run EUNICE, just call MAKE something else...

	I am sorry to have distributed something that didn't work perfectly
the first time -- this is my first try at network software distribution.  If
you have any other troubles do let me know.

	However, do note that I will be at DECUS starting this weekend, and
I will not be back until june 6, so I won't be too responsive until then.

jon

-- 
Jonathan Corbet
National Center for Atmospheric Research, Field Observing Facility
{seismo|hplabs}!hao!boulder!jon		(Thanks to CU CS department)
