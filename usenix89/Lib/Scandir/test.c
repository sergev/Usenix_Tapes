/*
**	Author J.L Portman III (root@transys.UUCP)
**	This is just included as a test driver for the scandir()
**	Function. Kind of a crude ls
**	Pretty brain damaged, in that it does not free() any of the
**	allocated structures.
*/

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ndir.h>

main(argc,argv,envp)
int argc;
char **argv;
char **envp;
{
	struct direct **filst;
	int	nfils, x, y;
	extern int scandir();
	int	compare(), select();
	char *strcpy(), *strncat();

	if (argc<2) {
		printf("Usage: scandir 'dirname'\n");
		exit(1);
	}

	for(x=1;x<argc;x++) {
	     if ((nfils = scandir(argv[x], &filst, select, compare)) == -1) {
		     printf("scandir: scandir() failed on directory %s\n", 
			  argv[x]);
		     exit(1);
	     }
		for (y = 0; y < nfils; y++) 
			printf("nfile[%d] = %s\n",y, filst[y]->d_name);
	}

	exit(0);

}


/*
**	Dummy selection routine, always returns true
*/

select(d)
	struct direct *d;
{
	return(1);

}

/*
**	For the qsort() in scandir. 
*/

compare(a, b)
	struct direct **a, **b;
{
	return(strcmp((*a)->d_name,(*b)->d_name)); 
}
