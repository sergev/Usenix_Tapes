# include       "sps.h"
# include       "flags.h"

/* PRHEADER - Print a header according to the switches */
prheader ()
{
	extern struct flags     Flg ;

	printf( "Ty User    %s Proc#", Flg.flg_v ?
		" Status Fl Nice Virtual Resident %M  Time Child %C" :
		Flg.flg_d ?
		"  Files    PageFaults Swap BlockI/O Kbytsecs" : "" ) ;
	if ( Flg.flg_f )
		printf( " Ppid#" ) ;
	if ( Flg.flg_g )
		printf( " Pgrp#" ) ;
	printf( " Command\n" ) ;
}
