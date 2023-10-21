#include <stdio.h>
#include "config.h"
#include "ng.h"

char *index() ;


nginit() {
      int c ;

      fseek(ngfp, 0L, 0) ;
      while ((c = getc(ngfp)) != '\n' && c != EOF) ;
}


ngread(g)
      register struct ngrec *g ;
      {
      char line[100] ;
      register char *p ;

      if (fgets(line, 100, ngfp) == NULL)
            return 0 ;
      if ((p = index(line, ' ')) == NULL)
            xerror("corrupted newsgroup file") ;
      *p++ = '\0' ;
      scopyn(line, g->g_name, sizeof(g->g_name)) ;
      g->g_num = atoi(p) ;
      g->g_flags = 0 ;
      if (index(p, 'm'))
            g->g_flags |= G_MOD ;
      return 1 ;
}
