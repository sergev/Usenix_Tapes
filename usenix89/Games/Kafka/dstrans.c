
/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 * This routine takes the data structures which have been created by the
 * parser and creates a file called "kaf.spec.c", with all the appropriate
 * source in it. Each node in the structure is given a unique name, and 
 * all links between these are made explicit here. Also all nodes are
 * linked together in a list, for the purpose of searching for names.
 */

#include "kafgraf.h"
#include "defs.h"
#include <stdio.h>

extern struct knode *hashtab[];
extern FILE *specp;		/* The file to output to. */
int errp = 0;

dstrans() 
{
	struct knode *nt, *wk, *lf;
	struct karc *tarc;
	int a;
	char *it;			/* Temp char *. */
	unsigned char b, phash();	/* For lookup of node names. */
	char lastnode[32];

	/* dumpdata(); Diagnostic. */

	/* First we have to resolve all references to nodes. Match
	 * ka_toname with the apropriate lists of nodes.
	 */
	
	for (a = 0; a < HASHSIZE; a++) {
	    for (nt = hashtab[a]; nt != NULL; nt = nt->kn_nnt) {
		for (wk = nt; wk != NULL; wk = wk->kn_next) {
		    for (tarc = wk->kn_arc; tarc != NULL; tarc = tarc->ka_narc) {
			if (tarc->ka_type != KTNTERM)
			    tarc->ka_to = NULL;	
			else {
			    it = tarc->ka_toname;
			    b = phash(it);
			    lf = NULL;
			    for (lf = hashtab[b]; lf != NULL; lf = lf->kn_nnt)
				if (!strcmp(it, lf->kn_nodename)) {
				    tarc->ka_to = lf;
				    break;
				}
			    if (lf == NULL) {
				fprintf(stderr, "Error: no such node: %s\n", it);
				errp = 1;
			    }
			}
		    }
		}
	    }
	}
	if (errp) {
		fprintf(stderr, "kafka aborted due to errors\n");
		exit(1);
	}

	/* All references are cool now. The working data structure is now
	 * to be built, with the difference that there is no hashing involved
	 * as everything is held together by the arcs. 
	 */

	strcpy(lastnode, "0");
	
	for (a = 0; a < HASHSIZE; a++) {
	    for (nt = hashtab[a]; nt != NULL; nt = nt->kn_nnt) {
		for (wk = nt; wk != NULL; wk = wk->kn_next) {
		    /* Write this node out. This is ugly... */
		    if (wk->kn_arc)
			fprintf(specp, "extern struct kcarc _kka%d;\n", 
			    wk->kn_arc->ka_arcnumber);
		    if (wk->kn_next)
			fprintf(specp, "extern struct kknode _kkn%d;\n",
			    wk->kn_next->kn_nodenumber);
		    if (wk->kn_fnum)
			fprintf(specp, "extern int _kkFunc%d();\n",
			    wk->kn_fnum);
		    fprintf(specp, "struct kknode _kkn%d = { ", wk->kn_nodenumber);
		    fprintf(specp, "%d, ", wk->kn_type);
		    if (wk->kn_nodename)
			fprintf(specp, "\"%s\", ",
			    wk->kn_nodename);
		    else
			fprintf(specp, "0, ");
		    if (wk->kn_arc)
		    	fprintf(specp, "&_kka%d, ", wk->kn_arc->ka_arcnumber);
		    else
			fprintf(specp, "0, ");
		    if (wk->kn_next)
		    	fprintf(specp, "&_kkn%d, ", wk->kn_next->kn_nodenumber);
		    else
			fprintf(specp, "0, ");
		    if (wk->kn_fnum)
		        fprintf(specp, "_kkFunc%d, ",
			    wk->kn_fnum);
		    else
			fprintf(specp, "0, ");
		    fprintf(specp, "%s };\n", lastnode);
		    sprintf(lastnode, "&_kkn%d", wk->kn_nodenumber);
		    if (wk->kn_arc) {
			for (tarc = wk->kn_arc; tarc != NULL; tarc = tarc->ka_narc) {
			    if (tarc->ka_to)
				fprintf(specp, "extern struct kknode _kkn%d;\n",
				    tarc->ka_to->kn_nodenumber);
			    if (tarc->ka_narc)
				fprintf(specp, "extern struct kcarc _kka%d;\n",
				    tarc->ka_narc->ka_arcnumber);
			    fprintf(specp, "struct kcarc _kka%d = { ", 
				tarc->ka_arcnumber);
			    fprintf(specp, "\"%s\", ", tarc->ka_toname);
			    if (tarc->ka_type == KTNTERM)
			    	fprintf(specp, "&_kkn%d, ",
				    tarc->ka_to->kn_nodenumber);
			    else
				fprintf(specp, "0, ");
			    if (tarc->ka_narc)
			    	fprintf(specp, "&_kka%d };\n", 
				    tarc->ka_narc->ka_arcnumber);
			    else	/* Oops { */
				fprintf(specp, "0 };\n");
			}
		    }
		}
	    }
	}
	fprintf(specp, "struct kknode *nodelist = %s;\n\n", lastnode);

	/* All the necessary information for the structure is now in place. */
}

/* Diagnostic routine. */

dumpdata()
{

	int hpos;
	struct knode *nont, *rule;
	struct karc *arc;

	for (hpos = 0; hpos < HASHSIZE; hpos++) {
		printf("Hashtab entry %d:\n", hpos);
		if (hashtab[hpos] == NULL) 
			printf("\t(empty)\n");
		else
			for (nont = hashtab[hpos]; nont != NULL; 
			    nont = nont->kn_nnt) {
				printf("\tNonterminal: %s\n", nont->kn_nodename);
				for (rule = nont; rule != NULL; 
				    rule = rule->kn_next) {
					printf("Rule: ");
					for (arc = rule->kn_arc; arc != NULL; 
					    arc = arc->ka_narc)
						printf("%s ", arc->ka_toname);
					putchar('\n');
				}
			}
	}
}

