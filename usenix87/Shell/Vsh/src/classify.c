#include "hd.h"
#include "classify.h"
#include "mydir.h"
#include "strings.h"

#define aout1	0407
#define aout2	0410
#define aout3	0411
#define aout4	0405
#define	aout5	0413
#define	cpio	070707
#define ar	0177545
#define	packold	017437
#define	pack	017037
#define	compact	017777
#define compress 0116437

/* Classify return the file type of parameter fname */

classify (fname) char * fname; 
{
    register unsigned mode;
    register int fdesc, rdlen;
    unsigned short word;
    int code;
    unsigned short wbuf[5];
    char *cp;
    char *lastfn();

    if (stat (fname, &scr_stb)) return CL_NULL;

    mode = scr_stb.st_mode & S_IFMT;
    if (mode == S_IFDIR) return CL_DIR;
    if (mode != S_IFREG) return CL_SPCL;

    fdesc = open (fname, 0);
    if (fdesc < 0) return CL_PROTPLN;

    rdlen = read (fdesc, wbuf, sizeof wbuf);
    if (rdlen < sizeof word)
	code = CL_TEXT;
    else {
	word = wbuf[0];
	cp = (char *)wbuf;
	cp[8] = 0;
	/* Berkeley archive */
	if (rdlen >= 8 && !strcmp(cp, "!<arch>\n"))
		word = ar;
	/* System 5.0 archive (where will it end ?) */
	else {
		cp[4] = 0;
		if (rdlen >= 4 && !strcmp(cp, "<ar>"))
			word = ar;
	}
	switch (word) {
	case aout1:
	case aout2:
	case aout3:
	case aout4:
	case aout5:
		code = CL_AOUT;
		break;
	case ar:
		code = CL_AR;
		break;
	case cpio:
		code = CL_CPIO;
		break;
	case pack:
	case packold:
		code = CL_PACK;
		break;
	case compact:
		code = CL_COMPACT;
		break;
	case compress:
		code = CL_COMPRESS;
		break;
	default:
		if (compe (lastfn (fname), "core"))
			code = CL_CORE;
		else {
			/* Attempt to protect user from binary files */
			if (istext(fdesc))
				code = CL_TEXT;
			else
				code = CL_UNKPLN;
		}
	}
    }

    close (fdesc);
    return code;
}

/*
 * istext(fd) - test if file contents is text-like
 */
istext(fd)
int fd;
{
	register int i;
	register char *s;
	register int cc;
	char buf[500];

	i = read(fd, buf, 500);
	cc = 0;
	for (s = buf; i > 0; i--) {
		/* Test for NUL, parity bit, control codes */
		if (*s == 0 || (*s&0200) || (ascii[*s] == UD && *s != '\b'))
			cc++;
		s++;
		if (cc > 50)
			break;
	}
	return (i ? 0 : 1);
}

/* Lastfn returns a pointer to the last file name in path.  */

char *
    lastfn (path) register char *path; 
{
    register char *cp;

    for (cp=path; *cp++;);

    cp--;
    while (*--cp != '/' && cp >= path);

    return ++cp;
}
