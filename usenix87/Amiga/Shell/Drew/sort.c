
/*
 * sort.c
 *
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 * sort has been modified via addition of slashcmp which provides correct
 * sorting of file names in tree/alphabetical order. This was needed
 * due to the recursive wild carding support I added in expand(). /Steve
 */

QuickSort(av, n)
char *av[];
int n;
{
   int b;

   if (n > 0) {
      b = QSplit(av, n);
      QuickSort(av, b);
      QuickSort(av+b+1, n - b - 1);
   }
}


/*
 * QSplit called as a second routine so I don't waste stack on QuickSort's
 * recursivness.
 */

QSplit(av, n)
char *av[];
int n;
{
   int i, b;
   char *element, *scr;

   element = av[0];
   for (b = 0, i = 1; i < n; ++i) {
      if (slashcmp(av[i],element) < 0) {
	 ++b;
	 scr = av[i]; av[i] = av[b]; av[b] = scr;
      }
   }
   scr = av[0]; av[0] = av[b]; av[b] = scr;
   return (b);
}

slashcmp(a,b)
char *a,*b;
{
   char *ap,*bp;
   int b_slash=0,a_slash=0,c;
   char *index();

   if (ap = index(a,'/')) ++a_slash;
   if (bp = index(b,'/')) ++b_slash;

   if (a_slash && b_slash) { /* both contain slashes */
       *bp = *ap = '\0';
       c = strcmp(a,b); /* just compare before slash */
       *bp = *ap = '/';
       if (c == 0) c = slashcmp(ap+1,bp+1); /* recursive */
       return(c);
   }
   if (!a_slash && !b_slash)  /* neither have slashes so just cmp */
       return(strcmp(a,b));
   return(a_slash - b_slash);
}

