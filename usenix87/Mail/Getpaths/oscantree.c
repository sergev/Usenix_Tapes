/*
   scan_tree: Joseph T. Buck, Entropic Processing, Inc.
	      Old-style version

   scan_tree applies a function to each file in a directory tree
   (excluding directories), and returns the number of files processed.
   This version reads directories directly and will not work with 4.2bsd.
*/
#include <sys/types.h>
#include <sys/dir.h>
#define MAXL 256


scan_tree (path, func)
char *path;
int (*func)();
{
    char name[MAXL], *strcpy(), *strncpy(), *pos;
    struct direct dirent;
    int fd = open (path), nf = 0, len;

    if (fd < 0) return 0;
    
    (void) strcpy (name, path);
    len = strlen (path);
    name[len] = '/';
    pos = name + len + 1;
    pos[DIRSIZ] = '\0'; /* req'd in case dirent has length DIRSIZ */
    while (read (fd, (char *) &dirent, sizeof dirent) == sizeof dirent) {
	if (dirent.d_ino == 0) continue;
	(void) strncpy (pos, dirent.d_name, DIRSIZ);
	if (strcmp (pos, ".") == 0 || strcmp (pos, "..") == 0) continue;
	if (is_dir (name)) nf += scan_tree (name, func);
	else {
	    (func) (name, dirent.d_ino);
	    nf += 1;
	}
    }
    (void) close (fd);
    return nf;
}
