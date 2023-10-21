
*** defs.h.old	Oct  3 20:37:48 1986
--- defs.h	Oct  3 20:37:48 1986
***************
*** 56,61
  /* # define HOSTDOMAIN "host.dom"	/* replacement for HOSTNAME.MYDOM */
  
  /*
  **  Locations of files:
  **	PATHS is where the pathalias output is.  This is mandatory.
  **	Define LOG if you want a log of mail.  This can be handy for

--- 58,73 -----
  /* # define HOSTDOMAIN "host.dom"	/* replacement for HOSTNAME.MYDOM */
  
  /*
+  * FORWARD is set to the site that we wish to send mail that we can't
+  * address to.  Does not need to be a direct connection.
+  *
+  * This hack local to National Semiconductor.
+  */
+ /* #ifndef FORWARD
+  * # define FORWARD    "forwarding-relay"
+  * #endif */
+ 
+ /*
  **  Locations of files:
  **	PATHS is where the pathalias output is.  This is mandatory.
  **	Define LOG if you want a log of mail.  This can be handy for
*** resolve.c.old	Fri Oct  3 20:46:45 1986
--- resolve.c		Fri Oct  3 20:46:46 1986
***************
*** 68,74
  	char *partv[MAXPATH];		/* "  "      "		*/
  	char temp[SMLBUF];		/* "  "      "		*/
  	int i;
! 		
  
  /*
  **  If we set REROUTE and are prepared to deliver UUCP mail, we split the 

--- 68,74 -----
  	char *partv[MAXPATH];		/* "  "      "		*/
  	char temp[SMLBUF];		/* "  "      "		*/
  	int i;
! 	int route_done = 0;		/* set if everything routed */
  
  /*
  **  If we set REROUTE and are prepared to deliver UUCP mail, we split the 
***************
*** 100,106
  **  we are set to route ALWAYS or REROUTE) or a ROUTE form.
  */
  		if ( rsvp( form ) != ROUTE && 
! 		    ( rsvp( form ) != UUCP || routing == JUSTDOMAIN ) )
  			break;
  /*
  **  Apply router.  If BULLYing and routing failed, try next larger substring.

--- 100,107 -----
  **  we are set to route ALWAYS or REROUTE) or a ROUTE form.
  */
  		if ( rsvp( form ) != ROUTE && 
! 		    ( rsvp( form ) != UUCP || routing == JUSTDOMAIN ) ) {
! 			route_done++;
  			break;
  		}
  /*
***************
*** 102,107
  		if ( rsvp( form ) != ROUTE && 
  		    ( rsvp( form ) != UUCP || routing == JUSTDOMAIN ) )
  			break;
  /*
  **  Apply router.  If BULLYing and routing failed, try next larger substring.
  */

--- 103,109 -----
  		    ( rsvp( form ) != UUCP || routing == JUSTDOMAIN ) ) {
  			route_done++;
  			break;
+ 		}
  /*
  **  Apply router.  If BULLYing and routing failed, try next larger substring.
  */
***************
*** 112,117
  */
  		form = parse( temp, domain, user );
  	DEBUG("parse route '%s' = %s @ %s (%d)\n",temp,user,domain,form);
  		break;
  	}
  /*

--- 114,120 -----
  */
  		form = parse( temp, domain, user );
  	DEBUG("parse route '%s' = %s @ %s (%d)\n",temp,user,domain,form);
+ 		route_done++;
  		break;
  	}
  /*
***************
*** 125,130
  		(void) strcpy( domain, "" );
  		form = LOCAL;
  	}
  /*
  **  If we were supposed to route and address but failed (form == ERROR), 
  **  or after routing once we are left with an address that still needs to

--- 128,168 -----
  		(void) strcpy( domain, "" );
  		form = LOCAL;
  	}
+ 
+ /*
+ **  Local NSC hack to forward unrouted messages
+ */
+ #ifdef FORWARD
+ 	if ( !route_done )
+ 	{
+ 		char forward[SMLBUF];
+ 
+ 		/* get the path to the forwarding machine */
+ 		if ( getpath( FORWARD, forward) == EX_OK ) {
+ 			char *p;
+ 
+ 			/* splice in the address */
+ 			sprintf( temp, forward, address );
+ 
+ 			/* split the address at the first ! */
+ 			p = index( temp, '!' );
+ 
+ 			if ( p ) {
+ 				*p = '\0';
+ 				(void) strcpy( user, p+1 );
+ 				(void) strcpy( domain, temp );
+ 				ADVISE( "forward '%s' = %s @ %s (%d)\n",
+ 					address, user, domain, UUCP );
+ 				return ( UUCP );
+ 			}
+ 		}
+ 		exitstat = EX_NOHOST;
+ 		printf( "%s...couldn't forward to %s.\n", address, FORWARD );
+ 		form = ERROR;
+ 		return( form );
+ 	}
+ #endif FORWARD
+ 
  /*
  **  If we were supposed to route and address but failed (form == ERROR), 
  **  or after routing once we are left with an address that still needs to

