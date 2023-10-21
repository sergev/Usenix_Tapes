/* Generate prompt string
 * Ben Cranston 3/14/87
 *
 * designed to be called as:
 *
 * alias cd 'cd \!*; set prompt="`dirs|genpro`"'
 *
 * builds and outputs string:  <hostname> [!] <workdir>
 *
 * where <hostname> is the name of the current host sans trailing domains
 *       <workdir>  is the current directory, with all but the last two
 *                  directory names replaced by ... for brevity.
 *
 * I had all this working with "awk" scripts, but it was taking over
 * four SECONDS to switch directories.  This was considered wasteful.
 *
 * SYSTEM V version by J.R. Stoner (asgard@cpro.UUCP) 03/24/87
 */

#define BUFFSIZE 512

#include <sys/utsname.h>

extern char *getcwd();
extern char *strchr();
extern char *strrchr();

main(argc,argv)
int argc;
char **argv;
{
	struct utsname u;
	char dirs[BUFFSIZE], buff[BUFFSIZE];
	char *cp;
	char *p1, *p2;
	uname(&u);
	getcwd(dirs,(sizeof dirs) - 2);
	if((cp=strchr(dirs,' ')) != (char *)0)
		*cp = 0;
		/* truncate dir string at first space (if any) */

/* search backwards for slash.  if found, temporarily make null and
 * search backwards for slash again.  if found again, replace string
 * to the left of the second slash with "..."
 *
 * the additional test of (p1!=dirs) is for a special case, a two-string
 * explicitly rooted.  that is, "cd /etc/ns" will display as /etc/ns
 * rather than ...etc/ns
 */

buff[0] = 0;
cp = dirs;
if ( 0 != (p2 = strrchr(dirs,'/')) ) {
	*p2 = 0;
	if ( (0 != (p1 = strrchr(dirs,'/'))) && (p1 != dirs) ) {
		strcpy(buff,"...");
		cp = p1 + 1;
	}
	*p2 = '/';
}

	strcat(buff,cp);
/* 	printf("%.8s[!] %s %% ",u.nodename,buff);	*/
	printf("%.8s %s %% ",u.nodename,buff);
}
