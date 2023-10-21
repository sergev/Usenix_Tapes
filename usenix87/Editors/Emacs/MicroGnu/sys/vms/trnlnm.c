/*
 * Name:	MicroEmacs
 *		VAX/VMS translate logical name routine
 * Version:	Gnu30
 * Last Edit:	10-Jul-86
 * By:		...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *
 */

/*
 *
 * Trnlnm()
 *
 * Description:
 *	Attempt to translate the logical name logname into an equivalence
 *	string, using the standard VMS routine LIB$SYS_TRNLOG().
 *	If a translation exists, return a pointer to the static area
 *	that contains the null-terminated translation string.  If not,
 *	return 0.
 *
 *  Bugs:
 *	Returns a pointer to static data that is modified each time
 *	the routine successfully translates a logical name.
 */

#include <ssdef.h>
#include <descrip.h>
#include <stdio.h>

static char _equiv_buf[256];
static struct dsc$descriptor_s
_equiv = {
	sizeof(_equiv_buf), DSC$K_DTYPE_T, DSC$K_CLASS_S, _equiv_buf
},
_name = {
	0, DSC$K_DTYPE_T, DSC$K_CLASS_S, NULL
};

char *trnlnm(logname)
char *logname;
{
	short eqlen;
	int status;

	if (logname == NULL)
		return (NULL);

	_name.dsc$a_pointer = logname;
	_name.dsc$w_length = strlen(logname);

	status = lib$sys_trnlog(&_name, &eqlen, &_equiv);
	if (status != SS$_NORMAL)
		return (NULL);

	_equiv_buf[eqlen] = '\0';
	return (_equiv_buf);
}

