#	defines and common block for argument decoding
#	routines

define ARGL	48	#argument length
define DEFINED	1	#argument status values
define VALUED	2
define UNDEFINED	0
define MAXARGS	32	#maximum number of arguments

common /rgxlst/ usedup(MAXARGS), kurrnt
logical usedup
integer*2 kurrnt
save /rgxlst/
