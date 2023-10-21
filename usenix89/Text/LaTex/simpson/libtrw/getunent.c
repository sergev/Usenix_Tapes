/* Scott Simpson, TRW */
#include <stdio.h>
#include <local/universe.h>

static char UNIVERSE[]	= "/etc/u_universe";
static char EMPTY[] = "";
static FILE *uf = NULL;
static char line[BUFSIZ+1];
static char name[80];
static char univ[80];
static struct universe universe;

setunent()
{
	if( uf == NULL )
		uf = fopen(UNIVERSE, "r" );
	else
		rewind( uf );
}

endunent()
{
	if( uf != NULL ){
		fclose( uf );
		uf = NULL;
	}
}

struct universe *
getunent()
{
	register char *p;
	if (uf == NULL) {
		if( (uf = fopen( UNIVERSE, "r" )) == NULL )
			return(0);
	}
	while (1) {
		p = fgets(line, BUFSIZ, uf);
		if (p==NULL)
			return(0);
		if (line[strlen(line)-1]=='\n')line[strlen(line)-1]='\0';
		if (strlen(line) == 0 || line[0] == '#' || line[0] == ':')
		    continue;
		if (sscanf(line, "%[^:]:%s", name, univ) != 2) {
		    return(0);
}		else {
		    universe.un_name = &name[0];
		    universe.un_universe = &univ[0];
		    break;
		}
	}
	return(&universe);
}

getunnam(name)
char *name;
{
    register struct universe *u;

    setunent();
    while ( (u = getunent()) && strcmp(name,u->un_name) );
    endpwent();
    return(u);
}
