/*
 *  print2stat - compare and print 2 stat structures side-by-side
 */

char   *ctime ();
char   *strcpy ();

print2stat (old, new)
struct stat *old;
struct stat *new;

{
    printf("OLD: ");
    printstat("", old);
    printf("NEW: ");
    printstat("", new);
    putchar('\n');
}
