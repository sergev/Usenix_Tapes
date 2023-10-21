#include        <stdio.h>
#include        "ed.h"

#define 	NMETA_CMDS  52


int	replace_string();		/* replace string global	*/
int 	query_replace();		/* query replace 		*/

extern  BUFFER *bfind();		/* buffer find			*/

extern  int	forwsearch();		/* forward search		*/
extern  int 	backsearch();		/* backward search		*/
extern  int     ctlxlp();               /* Begin macro                  */
extern  int     ctlxrp();               /* End macro                    */
extern  int     ctlxe();                /* Execute macro                */
extern  int     fileread();             /* Get a file, read only        */
extern  int     filevisit();            /* Get a file, read write       */
extern  int     filewrite();            /* Write a file                 */
extern  int     filesave();             /* Save current file            */
extern  int     quit();                 /* Quit                         */
extern  int     gotobol();              /* Move to start of line        */
extern  int     forwchar();             /* Move forward by characters   */
extern  int     gotoeol();              /* Move to end of line          */
extern  int     backchar();             /* Move backward by characters  */
extern  int     forwline();             /* Move forward by lines        */
extern  int     backline();             /* Move backward by lines       */
extern  int     forwpage();             /* Move forward by pages        */
extern  int     backpage();             /* Move backward by pages       */
extern  int     gotobob();              /* Move to start of buffer      */
extern  int     gotoeob();              /* Move to end of buffer        */
extern  int     setmark();              /* Set mark                     */
extern  int     swapmark();             /* Swap "." and mark            */
extern  int     nextwind();             /* Move to the next window      */
extern  int     prevwind();             /* Move to the previous window  */
extern  int     onlywind();             /* Make current window only one */
extern  int     splitwind();            /* Split current window         */
extern  int     mvdnwind();             /* Move window down             */
extern  int     mvupwind();             /* Move window up               */
extern  int     enlargewind();          /* Enlarge display window.      */
extern  int     shrinkwind();           /* Shrink window.               */
extern  int     listbuffers();          /* Display list of buffers      */
extern  int     usebuffer();            /* Switch a window to a buffer  */
extern  int     killbuffer();           /* Make a buffer go away.       */
extern  int     reposition();           /* Reposition window            */
extern  int     refresh();              /* Refresh the screen           */
extern  int     deblank();              /* Delete blank lines           */
extern  int     quote();                /* Insert literal               */
extern  int     backword();             /* Backup by words              */
extern  int     forwword();             /* Advance by words             */
extern  int     forwdel();              /* Forward delete               */
extern  int     backdel();              /* Backward delete              */
extern  int     kill();                 /* Kill forward                 */
extern  int     yank();                 /* Yank back from killbuffer.   */
extern  int     upperword();            /* Upper case word.             */
extern  int     lowerword();            /* Lower case word.             */
extern  int     upperregion();          /* Upper case region.           */
extern  int     lowerregion();          /* Lower case region.           */
extern  int     capword();              /* Initial capitalize word.     */
extern  int     delfword();             /* Delete forward word.         */
extern  int     delbword();             /* Delete backward word.        */
extern  int     killregion();           /* Kill region.                 */
extern  int     copyregion();           /* Copy region to kill buffer.  */

struct  Meta_Disc  {
		     char *meta_str;
		     int  (*meta_fctn)();
		     
		   } Meta_Commands[NMETA_CMDS]   = {  
			{ "replace-string",replace_string },
			{ "query-replace",query_replace },
			{ "forward-search",forwsearch },
			{ "backward-search",backsearch },
			{ "begin-macro",ctlxlp },
			{ "end-macro",ctlxrp },
			{ "execute-macro",ctlxe },
			{ "read-file",fileread },
			{ "visit-file",filevisit },
			{ "write-file",filewrite },
			{ "save-file",filesave },
			{ "begin-of-line",gotobol },
			{ "end-of-line",gotoeol },
			{ "top-of-buffer",gotobob },
			{ "bottom-of-buffer",gotoeob },
			{ "set-mark",setmark },
			{ "swap-mark",swapmark },
			{ "next-window",nextwind },
			{ "previous-window",prevwind },
			{ "only-window",onlywind },
			{ "split-window",splitwind },
			{ "move-window-down",mvdnwind },  
			{ "move-window-up",mvupwind },    
			{ "enlarge-window",enlargewind }, 
			{ "shrink-window",shrinkwind },   
			{ "list-buffers",listbuffers },
			{ "use-buffer",usebuffer },
			{ "kill-buffer",killbuffer },
			{ "reposition-window",reposition },
			{ "refresh-screen",refresh },
			{ "delete-blank-lines",deblank },
			{ "insert-literial",quote }, 	  
			{ "backward-word",backword },	  
			{ "forward-word",forwword },      
			{ "kill-to-end-of-line",kill },
			{ "yank-from-killbuffer",yank },  
			{ "case-word-upper",upperword },
			{ "case-word-lower",lowerword },
			{ "case-region-upper",upperregion },
			{ "case-region-lower",lowerregion },
			{ "case-word-capitalize",capword },
			{ "delete-next-word",delfword },
			{ "delete-previous-word",delbword },
			{ "kill-region",killregion },
			{ "copy-region",copyregion },
			{ "quit",quit },
			{ "backward-character",backchar },
			{ "forward-character",forwchar },
			{ "forward-line",forwline },
			{ "backward-line",backline },
			{ "forward-page",forwpage },
			{ "backward-page",backpage }
		     };
										
	
/* test routine */

metacmds() {

  int  s;
  char buff[64];


  s = mlreply("Meta-X: ",buff,sizeof(buff));

  if (s == ABORT)
    return(ABORT);

  if ((s = findpos(buff)) < 0) {
	mlwrite("Meta-X command does not exist");
	createhelpbuffer();
	return(ABORT);
  }
  else {
    (*(Meta_Commands[s].meta_fctn))(NULL,1);
  }
}

/*	Find Position()
 *		
 * 	This procedure matches the string typed in at the command line 
 * 	with one of the functions in the META-X list.  If none is found
 *	then it returns with a -1, else it returns with the index into
 *	the Meta_Commands (list).
 */

findpos(buff) 

  char *buff;
{
  int i;

  for (i=0;i<NMETA_CMDS;i++){
	if (strcmp(Meta_Commands[i].meta_str,buff)==0)
		return(i);
  }
  return(-1);
}

/*	Replace String ()
 *	
 * 	This routine reads in an old and new string and does a global
 *	replacement of all those occurances.  Most of this routine is
 *	a copy of the Search command ('forwardsearch');
 */

replace_string() {
    register LINE *clp;
    register int cbo;
    register LINE *tlp;
    register int tbo;
    register int c;
    register char *pp;
    register int s;
    register int repl_cnt;
    register char old_str[64];
    register char new_str[64];
  
    repl_cnt = 0;
    s = mlreply("Old String: ",old_str,sizeof(old_str));

    if ((s == ABORT) || (s != TRUE))
        return (s);

    if ((s = mlreply("New String: ",new_str,sizeof(new_str))) == ABORT)
        return (s);

    clp = curwp->w_dotp;
    cbo = curwp->w_doto;

    while (clp != curbp->b_linep)
        {
        if (cbo == llength(clp))
            {
            clp = lforw(clp);
            cbo = 0;
            c = '\n';
            }
        else
            c = lgetc(clp, cbo++);

        if (eq(c, old_str[0]) != FALSE)
            {
            tlp = clp;
            tbo = cbo;
            pp  = &old_str[1];

            while (*pp != 0)
                {
                if (tlp == curbp->b_linep)
                    goto fail;

                if (tbo == llength(tlp))
                    {
                    tlp = lforw(tlp);
                    tbo = 0;
                    c = '\n';
                    }
                else
                    c = lgetc(tlp, tbo++);

                if (eq(c, *pp++) == FALSE)
                    goto fail;
                }

            curwp->w_dotp  = tlp;
            curwp->w_doto  = tbo;
            curwp->w_flag |= WFMOVE;
	    repl_cnt++;
 	    replace_occ(strlen(old_str),new_str);
            }
fail:;
        }

    mlwrite(" %d Strings Replaced ",repl_cnt);
    return (FALSE);
}

/*	Query Replace()
 *	
 * 	This routine reads in an old and new string and does a query
 *	replacement of all those occurances.  Most of this routine is
 *	a copy of the Search command ('forwardsearch');
 */

query_replace() {
    register LINE *clp;
    register int cbo;
    register LINE *tlp;
    register int tbo;
    register int c;
    register int ch;
    register char *pp;
    register int s;
    register int repl_cnt;
    register char old_str[64];
    register char new_str[64];
  
    repl_cnt = 0;
    s = mlreply("Old String: ",old_str,sizeof(old_str));

    if ((s == ABORT) || (s != TRUE))
        return (s);

    if ((s = mlreply("New String: ",new_str,sizeof(new_str))) == ABORT)
        return (s);

    mlwrite("<ESC>,n,N : <SP>,y,Y : ^G");

    clp = curwp->w_dotp;
    cbo = curwp->w_doto;

    while (clp != curbp->b_linep) {

        if (cbo == llength(clp)) {
            clp = lforw(clp);
            cbo = 0;
            c = '\n';

        } /* if */
        else
            c = lgetc(clp, cbo++);

        if (eq(c, old_str[0]) != FALSE) { 
            tlp = clp;
            tbo = cbo;
            pp  = &old_str[1];

            while (*pp != 0) {
                if (tlp == curbp->b_linep)
                    goto fail;

                if (tbo == llength(tlp)) {
                    tlp = lforw(tlp);
                    tbo = 0;
                    c = '\n';
		    
                } /* if */
                else
                    c = lgetc(tlp, tbo++);

                if (eq(c, *pp++) == FALSE)
                    goto fail;

            } /* while */

            curwp->w_dotp  = tlp;
            curwp->w_doto  = tbo;
            curwp->w_flag |= WFMOVE;
	    update();
tryagain:;
            ch = (*term.t_getchar)();

	    switch (ch) {	    
	    
		case  0x07 : {
	        	        ctrlg(FALSE, 0);
				mlwrite("Aborted");
		                (*term.t_flush)();
				return(0);
				goto fail;
			      }
			      break;
		case 0x020 :
		case 0x079 : 
		case 0x059 :  {
			        repl_cnt++;
		 	    	replace_occ(strlen(old_str),new_str);
		  	      }
			      break;
	        case 0x01b :
		case 0x06e :
		case 0x04E :  {
		              }
			      break;
		default	   :  {
		                ctrlg(FALSE, 0);
		                (*term.t_flush)();
				mlwrite(" TRY AGAIN! <ESC>,n,N : <SP>,y,Y : ^G");
				update();
				goto tryagain;
			      }

		} /* switch */
 	} /* if */
fail:;
    } /* while */

    mlwrite(" %d Strings Replaced ",repl_cnt);
    return (FALSE);
}
/*  This routine takes the current cursor location, deletes the old
 *  string and replaces it with the new string
 */

replace_occ(old_length,new_str) 

  int	old_length;
  char	*new_str;

{
  register int i;
  register int len;

  backdel(TRUE,old_length);
  len = strlen(new_str);
  for (i=0;i<len;i++) {
	linsert(1,*new_str);
	new_str++;
  }
}

/*
 *
 *	
 *
 */

createhelpbuffer() {

  register BUFFER *bp;
  register int	  i;


  bp = bfind("HELP",TRUE,0);

  if (nextwind(NULL,NULL)==FALSE)
	  splitwind();

  useb("HELP");
  if (bclear(bp) != TRUE)
	return(FALSE);

  for (i=0;i<NMETA_CMDS;i++) 
	if (addbufftext(Meta_Commands[i].meta_str,bp) != TRUE)
		return(FALSE);

  gotobob(NULL,NULL);
  prevwind(NULL,NULL);
}  

