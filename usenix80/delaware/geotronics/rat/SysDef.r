### SysDef.r - system-wide symbol definitions
# UNIX V6 version  03-Jun-1980  D A Gwyn

 # standard Ratfor language extensions -

 # define(assert,#)			# production mode: activate
define(boolean,logical*1)		# ANSI: logical
define(byte,integer*1)			# ANSI: integer
define(character,integer*1)		# ANSI: integer
 # define(program,#)			# ANSI: activate
 # define(reference,#)			# for declaring calls

 # standard symbolic constants -

define(ARB,1)				# indeterminate array subscript
define(BLANK,32)			# ASCII space character
define(DIGIT,48)			# character class
define(EOF,26)				# Ctrl/Z, DEC standard
define(EOS,0)				# ASCIZ, DEC standard
define(LETTER,97)			# character class
define(MAXLINE,82)			# input line size + NL + EOS
define(NEWLINE,10)			# ASCII line-feed
define(NEWPAGE,12)			# ASCII form-feed
define(NO,.false.)			# boolean constant
define(NULLFD,-1)			# null file descriptor
define(READONLY,0)			# i/o access mode
define(READWRITE,2)			# i/o access mode
define(WRITEONLY,1)			# i/o access mode
define(YES,.true.)			# boolean constant

 # system-specific values -

define(BLOCKSIZE,512)			# random-access record size
define(EPSILON,6.0e-08)			# floating-point tolerance
define(INFINITY,2147483647)		# largest integer value
define(MAXEXPO,38)			# 10.0**(MAXEXPO+1) overflows
define(MAXNAME,40)			# maximum filespec size + 1
define(PROTECT,416)			# default file protection
