# include       "sps.h"

/* PRSUMMARY - Print the summarising information */
prsummary ()
{
	extern struct summary   Summary ;

	printf(
"%D (%Dk) processes, %D (%Dk) busy, %D (%Dk) loaded, %D (%Dk) swapped\n",
		Summary.sm_ntotal, KBYTES( Summary.sm_ktotal ),
		Summary.sm_nbusy, KBYTES( Summary.sm_kbusy ),
		Summary.sm_nloaded, KBYTES( Summary.sm_kloaded ),
		Summary.sm_nswapped, KBYTES( Summary.sm_kswapped ) ) ;
	Summary.sm_ntotal = 0L ;
	Summary.sm_ktotal = 0L ;
	Summary.sm_nbusy = 0L ;
	Summary.sm_kbusy = 0L ;
	Summary.sm_nloaded = 0L ;
	Summary.sm_kloaded = 0L ;
	Summary.sm_nswapped = 0L ;
	Summary.sm_kswapped = 0L ;
}
