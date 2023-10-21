/*
 * Program to determine whether file2 needs to be updated from file1.
 * that another one.  Usage:  newer file1 file2
 * Returns:
 *	1  file2 is newer than file1.
 *	0  file2 is older than file1 or one of the files does not exist.
 *	8  arg count.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


main(argc, argv)
      char **argv ;
      {
      struct stat st1, st2;

      if (argc != 3) {
            fputs("Usage: newer file1 file2\n", stderr);
            exit(8);
      }
      if (stat(argv[1], &st1) < 0) {
            fputs("newer: can't stat ", stderr);
            fputs(argv[1], stderr);
            fputs("\n", stderr);
            exit(0);			/* force update attempt */
      }
      if (stat(argv[2], &st2) < 0 || st2.st_mtime < st1.st_mtime)
            exit(0);
      exit(1);				/* up to date */
}
