#define maxmap 32
/*#define cmdline 23	/** temp */
#include "edit.h"

struct map{
	int	ma_lgt;
	char*	ma_from;
	char*	ma_to;
	};
struct map map[maxmap];
struct map *nextmapslot=map;

mapc()
	{
	register c;
	register struct map *mp;
	char *tp;
	extern char *globp;
	static char *mapp=0;
	static char mapbuf[82];
	int mcount,equal;

	if(globp){
		if(c = *globp++){
			return(c);
			}
		globp=0;
		}
	if(mapp){
		if(c = *mapp++){
#ifdef debugging
debug(90,"\007mapc return1 0%o",c);
#endif
			return(c);
			}
		mapp=0;
		}

	c=rarec();
	if(c >= ' ')
		return(c);
	tp=mapp=mapbuf;
	mcount=1;
loop:
	*tp++ = c;
	equal=0;
/*
	for(mp=map; mp->ma_lgt; mp++){
*/	for(mp=nextmapslot-1; mp>=map; mp--){	/* search from bottom up */
		if(mp->ma_lgt<mcount)
			continue;
		if(strncmp(mapbuf,mp->ma_from,mcount)==0){
			equal++;
			if(mcount==mp->ma_lgt){
				mapp=mp->ma_to;
				goto found;
				}
			}
		}
	if(equal){
		c=rarec();
		mcount++;
		goto loop;
		}
found:
#ifdef debugging
debug(89,"mapc: mapp=0%o mapbuf=0%o mcount=%d equal=%d c=0%o",
mapp,mapbuf,mcount,equal,c);
#endif
	*tp='\0';
#ifdef debugging
debug(90,"\007mapc return2: 0%o",*mapp);
#endif
	return(*mapp++);
	}

struct map *
findmapslot()
	{
	if(nextmapslot >= &map[maxmap])
		errfunc("Map table full");
	return(nextmapslot++);
	}

setmap(string)
	char *string;
	{
	char *src, *tgt, del;
	register struct map *mp;

#ifdef debugging
debug(87,"setmap: string=%s",string);
#endif
	if(string==0) return;
	del = *string++;
	src=tgt=string;
	do{
		if(*src == '\0') break;
		mp = findmapslot();
		mp->ma_from = tgt;
		if(transmap(&src, &tgt, del)==0)
			errfunc("Bad map format");
		mp->ma_lgt = strlen(mp->ma_from);
		mp->ma_to = tgt;
		} while(transmap(&src, &tgt, del));
	}

transmap(src,tgt,del)
	register char **src, **tgt;
	char del;
	{
	register c;

    loop:
	c = *(*src)++;
#ifdef debugging
debug(88,"transmap: c=0%o (%c)",c,c>=' '?c:'?');
#endif
	if(c=='\\')
		switch(c = *(*src)++){
			case 'b': c='\b'; break;
			case 'n': c='\n'; break;
			case 't': c='\t'; break;
			case 'E': c='\033'; break;
			}
	else if(c=='^')
		c = *(*src)++ & 037;
	**tgt = (c==del)?0:c;
	if(*(*tgt)++ == 0)
		return(c);
	goto loop;
	}

mapcomd()
	{
	extern char _sibuf[];
#	define string _sibuf
	static char *cp = string;
	char *start;

	start=cp;
	*cp++ = 0177;
	message("Enter source sequence followed by SPACE");
	/** this code is used by loading and gnv and should be a subroutine */
	/** it could be an option of message */
	clearsline(cmdline);
	printf("Mapping: ");
	while((*cp = rarec())!=' ')
		echo(*cp++);
	*cp++ = 0177;
	message("Enter output sequence followed by RETURN");
	clearsline(cmdline);
	printf("To: ");
	while((*cp = rarec())!='\n')
		echo(*cp++);
	*cp++ = '\0';
	setmap(start);
	message("");
	}
