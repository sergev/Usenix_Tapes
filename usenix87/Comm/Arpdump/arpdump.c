#define CCI	/* CCI needs this because of nlist() problems */
#define DPB	/* new personal mods added to the original */

#ifdef DPB
/*
 *	arpdump.c
 *	Barry Shein, Boston University original 10 21 85
 *	Dennis Bednar, CCI Reston VA, added options, Timer, Flags, 10 25 85
 *
 *	usage: arpdump [-a] [-v] [seconds]
 *		-a = print old timed-out slots too
 *			ie slots containing non-zero etheraddr && IP addr == 0
 *		-v = verbose, prints every entry in the table
 *		seconds = decimal seconds to print again
 *
 */
#else
/*
	usage: arpdump [seconds]
Newsgroups: net.unix-wizards
Subject: Re: Re: Ethernet problems


Re: Hedrick's suggestion that the problem may be with your ARP tables:

Here's a hack I use to examine the arp tables on my systems, I think
half of it or more is from something else, but it seems to do the job
(4.2bsd):

*/
#endif

#include <stdio.h>
#include <nlist.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char *fcore = "/dev/kmem", *fnlist = "/vmunix" ;
struct nlist nl[] = {
#define ARPTAB	0
	{ "_arptab" },
	{ "" },
} ;
#ifdef DPB
/*	The arptab structure is defined in netinet/if_ether.h		*/
#endif
struct	arptab {
	struct	in_addr at_iaddr;	/* internet address */
	u_char	at_enaddr[6];		/* ethernet address */
	struct	mbuf *at_hold;		/* last packet until resolved/timeout */
	u_char	at_timer;		/* minutes since last reference */
	u_char	at_flags;		/* flags */
};
/* at_flags field values */
#ifdef DPB
/*	The at_flags are defined in net/if.h		*/
#endif
#define	ATF_INUSE	1		/* entry in use */
#define ATF_COM		2		/* completed entry (enaddr valid) */
#ifdef DPB
#define	ATF_PERM	4	/* permanent entry */
#define	ATF_PUBL	8	/* publish entry (respond for other host) */
#endif

#ifdef DPB
/*	bucket stuff defined in netinet/if_ether.c	*/
#endif
#define	ARPTAB_BSIZ	5		/* bucket size */
#define	ARPTAB_NB	19		/* number of buckets */
#define	ARPTAB_SIZE	(ARPTAB_BSIZ * ARPTAB_NB)

struct	arptab *arptab, atab ;

#ifdef DPB
int	verbose;	/* -v verbose flag */
int	allflag;	/* -a to print all timed out ones */
#endif

main(argc,argv) int argc ; char **argv ;
{
	int fc ;
	int i,j ;
	int secs ;
	char *ap ;
	struct hostent *gethostbyaddr(), *hp ;

	if((fc = open(fcore,0)) < 0)
	{
		perror(fcore) ;
		exit(1) ;
	}
#ifdef CCI
	if (nlist(fnlist, nl) == -1)
		{
		fprintf(stderr, "arptab: nlist on %s returned error\n", fnlist);
		exit(1);
		}
#else
	nlist(fnlist,nl) ;
#endif
	arptab = (struct arptab *) nl[ARPTAB].n_value ;
#ifdef CCI
	if (nl[0].n_value == 0)
#else
	if(nl[0].n_type == 0)
#endif
	{
		fprintf(stderr,"No name list %s\n",fnlist) ;
		exit(1) ;
	}
#ifdef DPB
	secs = 0;
	/* primitive error checking on arguments */
	for (i = 1; i < argc; ++i)
		if (strcmp(argv[i], "-a") == 0)
			++allflag;
		else if (strcmp(argv[i], "-v") == 0)
			++verbose;
		else
			secs = atoi(argv[i]);
#else
	if(argc > 1) secs = atoi(argv[1]) ;
	else secs = 0 ;
#endif
	printf("\t\tACTIVE ARP TABLE DUMP\n") ;
loop:
#ifdef DPB
	printf("Index\tIP Addr\t\tEthernet Interface\tHostName\tTimer\tFlags\n") ;
#else
	printf("Index\tAddr\t\tEthernet Interface\tHostName\n") ;
#endif
	lseek(fc,(int) arptab,0) ;
#ifdef DPB
	ap = (char *) &atab;
#else
	/* old way requires that at_iaddr be first */
	ap = (char *) &atab.at_iaddr ;
#endif
	for(i=0 ; i < ARPTAB_SIZE ; i++)
	{
		if(read(fc,&atab,sizeof atab) != (sizeof atab))
		{
			fprintf(stderr,"Error reading arp table\n") ;
			exit(1) ;
		}
#ifdef DPB
		if (verbose)
			goto printslot;	/* print every slot it */

		if (allflag)	/* print old timed out "half-filled" slots */
			if ( inet_netof(atab.at_iaddr) != 0 ||
				atab.at_flags != 0 ||
				ge_addr(&atab.at_enaddr[0]) != 0)
				;	/* print slot below */
			else
				continue;
		else		/* print only entries with IP addresses */
			if (inet_netof(atab.at_iaddr) != 0)
				;	/* print slot below */
			else
				continue;
printslot:
#else
		if(inet_netof(atab.at_iaddr) == 0) continue ;
#endif
		printf("%2d:\t",i) ;
		printf("%s\t",ap = (char *) inet_ntoa(atab.at_iaddr)) ;
#ifdef DPB
		/* kludgy way to align columns because IP addr is var len */
		if (strlen(ap)<8) printf("\t");
#endif
		for(j=0 ; j < 6 ; j++)
			printf("%02x",atab.at_enaddr[j]) ;
		if((hp = gethostbyaddr(&atab.at_iaddr.s_addr,
					sizeof atab.at_iaddr.s_addr,
					AF_INET)) == NULL)
#ifdef DPB
		/* save ptr to host name printed for strlen later */
			printf(ap = "\t\t(unknown)") ;
		else printf("\t\t%s",ap = hp->h_name) ;
#else
			printf("\t\t(unknown)") ;
		else printf("\t\t%s",hp->h_name) ;
#endif
#ifdef DPB
		/* again kludgy way to align columns because hostname var len */
		if (strlen(ap)<8) printf("\t");
		printf("\t%3d", atab.at_timer);
		printf("\t");
		if (atab.at_flags & ATF_INUSE) printf("U");
		if (atab.at_flags & ATF_COM)   printf("C");
		if (atab.at_flags & ATF_PERM)  printf("P");
		if (atab.at_flags & ATF_PUBL)  printf("R"); /* responding */
#endif
		printf("\n") ;
	}
	if(secs <= 0) exit(0) ;
	sleep(secs) ;
	goto loop ;
}
/*
	-Barry Shein, Boston University
*/

#ifdef DPB
/*
 * get ethernet address
 * returns 0 iff 6 byte ethernet address is all zeros
 */
ge_addr(addr)
	register	char	*addr;
{
	register	int	i;

	for (i = 0; i < 6; ++i, ++addr)
		if (*addr)
			return	1;	/* found a non-zero byte */

	return 0;			/* all bytes were zero */
}
#endif
