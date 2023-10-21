/*
   scan_tree: Joseph T. Buck, Entropic Processing, Inc.
	      New-style version

   scan_tree applies a function to each file in a directory tree
   (excluding directories), and returns the number of files processed.
   This version uses the 4.2bsd directory access library; another version
   that reads directories directly is available.
*/
#include <sys/types.h>
#include <sys/dir.h>
#define MAXL 256
scan_tree (path, func)
char *path;
int (*func)();
{
    char name[MAXL], *strcpy(), *pos;
    DIR *dirp = opendir (path);
    struct direct *de;
    int nf = 0, len;

    if (dirp == NULL) return 0;
    (void) strcpy (name, path);
    len = strlen (path);
    name[len] = '/';
    pos = name + len + 1;
    while ((de = readdir (dirp)) != NULL) {
	if (strcmp (de->d_name, ".") == 0 || strcmp (de->d_name, "..") == 0)
	    continue;
	(void) strcpy (pos, de->d_name); 
	if (is_dir (name)) nf += scan_tree (name, func);
	else {
	    (func) (name,de->d_ino);
	    nf += 1;
	}
    }
    (void) closedir (dirp);
    return nf;
}
