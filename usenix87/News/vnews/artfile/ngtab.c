#include <stdio.h>
#include "ng.h"


static char MODGROUPS[] = "all.announce,mod.all,all.mod,fa.all,control,junk" ;
struct ngrec *ngroup ;
extern int maxng ;



/*
 * Read in the newsgroup file.
 */

getngtab() {
      struct ngrec ng ;
      char *savestr(), *calloc(), *realloc() ;

      gfopen() ;
      if ((ngroup = calloc(sizeof(*ngroup), maxng)) == NULL)
            xerror("out of space") ;
      ALL_GROUPS(ng) {
            ngroup[ng.g_num - 1] = ng ;
      }
      gfclose() ;
}



/*
 * Initialize without reading in the newsgroup file.
 */

ngtinit() {
      char *ckmalloc() ;

      maxng = 0 ;
      ngroup = ckmalloc(sizeof(struct ngrec)) ;
}



/*
 * Convert a newsgroup number to a newsgroup name
 */

char *
ngname(num) {
      return ngroup[num - 1].g_name ;
}



/*
 * Convert a newsgroup name to a number
 */

int
ngnum(name)
      char *name ;
      {
      register i ;
      register struct ngrec *gp ;
      char n[MAXNGNAME] ;

      scopyn(name, n, MAXNGNAME) ;
      for (gp = ngroup, i = maxng ; --i >= 0 ; gp++)
            if (strcmp(gp->g_name, n) == 0)
                  return gp - ngroup + 1 ;
      printf("ngnum: %s not found\n", name) ;		/*DEBUG*/
      return -1 ;
}



newng(name)
      char *name ;
      {
      register int i ;

      for (i = 0 ; i < maxng ; i++) {
            if (ngroup[i].g_name[0] == '\0')
                  break ;
      }
      printf("newng %s assigns %d", name, i + 1) ;	/*DEBUG*/
      if (i == maxng) {
            if ((ngroup = realloc((char *)ngroup, (i + 1) * sizeof ngroup[0])) == NULL)
                  xerror("out of space") ;
            maxng++ ;
            printf(", maxng upped") ;			/*DEBUG*/
      }
      printf("\n") ;					/*DEBUG*/
      scopyn(name, ngroup[i].g_name, MAXNGNAME) ;
      ngroup[i].g_flags = ngmatch(name, MODGROUPS)? G_MOD : 0 ;
      return ngroup[i].g_num = i + 1 ;
}



writengfile() {
      char newname[64], curname[64], oldname[64] ;
      FILE *fp ;
      int i ;
      register struct ngrec *gp ;
      extern char LIB[] ;

      printf("writeng called\n") ;		/*DEBUG*/
      sprintf(curname, "%s/groupfile", LIB) ;
      sprintf(newname, "%s.tmp", curname) ;
      sprintf(oldname, "%s.bak", curname) ;
      unlink(newname) ;
      if ((fp = fopen(newname, "w")) == NULL)
            xerror("Can't create %s", newname) ;
      fprintf(fp, "%d\n", maxng) ;
      for (i = maxng, gp = ngroup ; --i >= 0 ; gp++) {
            fprintf(fp, "%s %d", gp->g_name, gp->g_num) ;
            if (gp->g_flags & G_MOD)
                  fputs(" -m", fp) ;
            fputc('\n', fp) ;
      }
      if (ferror(fp) || fclose(fp) == EOF)
            xerror("Write error on %s", newname) ;
      rename(curname, oldname) ;
      if (rename(newname, curname) < 0)
            xerror("rename %s failed", newname) ;
}
