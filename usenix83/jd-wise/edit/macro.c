#include "ed.h"
#include "window.h"

#define maxargs 9

macro(n)
	{
	char mbuf[GBSIZE];
	int *addr, *a1, *a2;
	char *saveglobp;
	char savepeek;
	struct window *wp;

	saveglobp = globp;
	savepeek = peekc;
	peekc = 0;
	expmac(n, mbuf, sizeof(mbuf));
	a1 = addr1.ad_addr;
	a2 = addr2.ad_addr;
	wp = addr1.ad_window;
	for (addr=a1; addr<=a2; addr++) {
		globp = mbuf;
		wp->wi_dot = addr;
		commands();
		}
	globp = saveglobp;
	peekc = savepeek;
	}

expmac(n, buf, size)
	char *buf;
	{
	char argbuf[GBSIZE];
	char *args[maxargs];
	char del;
	char *cp, *ap, *lp;
	char c;
	int nargs;

	nargs=0;
	del=getchr();
#ifdef debugging
debug(86,"expmac: del=0%o",del);
#endif
	if(del != '\n')
		for(ap=cp=argbuf; ;cp++){
			if(cp >= &argbuf[GBSIZE])
				errfunc("Argument list too long");
			c = *cp = getchr();	
			if(c==del || c=='\n'){
				*cp = '\0';
#ifdef debugging
debug(86,"expmac: arg %d=%s (lgt=%d)",nargs,ap,cp-ap);
#endif
				args[nargs++]=ap;
				ap=cp+1;
				if(nargs>=maxargs)
					errfunc("Too many arguments");
				if(c=='\n')
					break;
				}
			}
#ifdef debugging
debug(86,"expmac: nargs=%d",nargs);
#endif

	primebuf(n);
#ifdef old
	for(cp=buf; getbuf()!=EOF; cp++){
		for(lp=linebuf; c = *lp; lp++, cp++){
			if(c=='\\'){
				*cp = *++lp;
				continue;
				}
			if(c=='$'){
				n = *++lp - '1';
				if(n>=maxargs)
					errfunc("Bad parameter $%c",n+'1');
				/* unspecified parameters are empty */
				if(n<nargs){
					for(ap=args[n]; *ap; ap++, cp++)
						*cp = *ap;
					cp--;
					}
				continue;
				}
			*cp = c;
			}
		*cp = '\n';
		}
	*--cp = '\0';
#else
	for(cp=buf; getbuf()!=EOF; ){
		for(lp=linebuf; c = *lp; lp++){
			if(c=='\\'){
				*cp++ = *++lp;
				continue;
				}
			if(c=='$'){
				n = *++lp - '1';
				if(n>=maxargs)
					errfunc("Bad parameter $%c",n+'1');
				/* unspecified parameters are empty */
				if(n<nargs){
					for(ap=args[n]; *ap; ap++)
						*cp++ = *ap;
					}
				continue;
				}
			*cp++ = c;
			}
		*cp++ = '\n';
		}
	*--cp = '\0';
#endif
#ifdef debugging
debug(86,"expmac: buf=%s *cp=0%o",buf,*cp);
#endif
	}
