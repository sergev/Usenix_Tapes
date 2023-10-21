# include       "sps.h"
# include       "flags.h"

/* FLAGDECODE - Looks at the argument list and sets various internal switches */
flagdecode ( argc, argv )

register int                    argc ;
register char                   **argv ;

{
	register char           *chp ;
	union flaglist          *plist ;
	union flaglist          *tlist ;
	union flaglist          *ulist ;
	static char             usage[] =
	"sps - Unknown option %s\nUsage - sps [ -defgijkoqrsvwyABFNPSTUWZ ][ process|tty|user ] ...\n";
	union flaglist          *getflgsp() ;
	extern struct flags     Flg ;

	plist = tlist = ulist = (union flaglist*)0 ;
	for ( argv++ ; --argc ; argv++ )
	{
		chp = *argv ;
		while ( *chp )
			switch ( *chp++ )
			{
				case '-' :
					/* Separation character */
					continue ;
				case 'd' :
				case 'D' :
					/* List disc orientated information */
					Flg.flg_d = 1 ;
					Flg.flg_v = 0 ;
					continue ;
				case 'e' :
				case 'E' :
					/* List environment strings */
					Flg.flg_e = 1 ;
					continue ;
				case 'f' :
					/* List the father's process id */
					Flg.flg_f = 1 ;
					continue ;
				case 'g' :
				case 'G' :
					/* List the process group id */
					Flg.flg_g = 1 ;
					continue ;
				case 'i' :
				case 'I' :
					/* Initialise (super-user only) */
					Flg.flg_i = 1 ;
					continue ;
				case 'j' :
				case 'J' :
					/* The next argument specifies the
					   name of the information file */
					if ( argc <= 1 )
						prexit(
	      "sps - Name of an information file expected after `-j' flag\n" ) ;
					argc-- ;
					Flg.flg_j = *++argv ;
					continue ;
				case 'k' :
				case 'K' :
					/* Use a disc file such as /vmcore
					   rather than /dev/{k}mem for
					   accessing kernel data. The next
					   argument specifies the file name. */
					if ( argc <= 1 )
						prexit(
	       "sps - Name of a memory dump file expected after `-k' flag\n" ) ;
					argc-- ;
					Flg.flg_k = *++argv ;
					Flg.flg_o = 1 ;
					continue ;
				case 'l' :
				case 'v' :
				case 'L' :
				case 'V' :
					/* Verbose output */
					Flg.flg_d = 0 ;
					Flg.flg_v = 1 ;
					continue ;
				case 'o' :
				case 'O' :
					/* Avoid looking at the swap device */
					Flg.flg_o = 1 ;
					continue ;
				case 'q' :
				case 'Q' :
					/* Show only the user time, not the
					   user + system times together. */
					Flg.flg_q = 1 ;
					continue ;
				case 'r' :
				case 'R' :
					/* Repeat output every n seconds.
					   The next argument specifies n which
					   defaults to 5 if omitted. */
					Flg.flg_r = 1 ;
					if ( argc > 1 )
					{
						if ( **++argv >= '0'
						&& **argv <= '9' )
						{
							argc-- ;
							Flg.flg_rdelay
							       = atoi( *argv ) ;
							continue ;
						}
						argv-- ;
					}
					Flg.flg_rdelay = 0 ;
					continue ;
				case 's' :
					/* Next argument specifies a symbol
					   file rather than the default
					   /vmunix. */
					if ( argc <= 1 )
						prexit(
		    "sps - Name of a symbol file expected after `-s' flag\n" ) ;
					argc-- ;
					Flg.flg_s = *++argv ;
					continue ;
				case 'w' :
					/* Wide output, exceeding 79 columns */
					Flg.flg_w = 1 ;
					continue ;
				case 'y' :
				case 'Y' :
					/* List current tty information */
					Flg.flg_y = 1 ;
					continue ;
				case 'a' :
				case 'A' :
					/* List all processes */
					Flg.flg_AZ = 1 ;
					Flg.flg_A = 1 ;
					continue ;
				case 'b' :
				case 'B' :
					/* List only busy processes */
					Flg.flg_AZ = 1 ;
					Flg.flg_B = 1 ;
					continue ;
				case 'F' :
					/* List only foreground processes */
					Flg.flg_AZ = 1 ;
					Flg.flg_F = 1 ;
					continue ;
				case 'n' :
				case 'N' :
					/* No processes, just the summary line*/
					Flg.flg_AZ = 1 ;
					Flg.flg_N = 1 ;
					continue ;
				case 'p' :
				case 'P' :
					/* List only the given process ids */
					Flg.flg_AZ = 1 ;
					Flg.flg_P = 1 ;
					if ( !plist )
					   plist=Flg.flg_Plist=getflgsp( argc );
					while ( argc > 1 )
					{
						if ( **++argv == '-' )
						{
							--argv ;
							break ;
						}
						--argc ;
						plist->f_chp = *argv ;
						(++plist)->f_chp = (char*)0 ;
					}
					continue ;
				case 'S' :
					/* List only stopped processes */
					Flg.flg_AZ = 1 ;
					Flg.flg_S = 1 ;
					continue ;
				case 't' :
				case 'T' :
					/* List only processes attached to the
					   specified terminals */
					Flg.flg_AZ = 1 ;
					Flg.flg_T = 1 ;
					if ( !tlist )
					   tlist=Flg.flg_Tlist=getflgsp( argc );
					while ( argc > 1 )
					{
						if ( **++argv == '-' )
						{
							--argv ;
							break ;
						}
						--argc ;
						tlist->f_chp = *argv ;
						(++tlist)->f_chp = (char*)0 ;
					}
					continue ;
				case 'u' :
				case 'U' :
					/* List only processes belonging to the
					   specified users */
					Flg.flg_AZ = 1 ;
					Flg.flg_U = 1 ;
					if ( !ulist )
					   ulist=Flg.flg_Ulist=getflgsp( argc );
					while ( argc > 1 )
					{
						if ( **++argv == '-' )
						{
							--argv ;
							break ;
						}
						--argc ;
						ulist->f_chp = *argv ;
						(++ulist)->f_chp = (char*)0 ;
					}
					continue ;
				case 'W' :
					/* List only waiting processes */
					Flg.flg_AZ = 1 ;
					Flg.flg_W = 1 ;
					continue ;
				case 'z' :
				case 'Z' :
					/* List only zombie processes */
					Flg.flg_AZ = 1 ;
					Flg.flg_Z = 1 ;
					continue ;
				default :
					prexit( usage, *argv ) ;
					/* NOTREACHED */
			}
	}
}
