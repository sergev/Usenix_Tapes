
#! /bin/sh
# test script for pman options
E="echo ============ "
F='mv rm cp ln find'
pman=./pman
$E ILLEGAL OPTION: pman -z $F
	$pman -z $F
$E MISSING NAMES: pman
	$pman
$E UNFOUND NAME: pman nomanual foobaz
	$pman nomanual foobaz
$E UNFOUND NAME IN SECTION: pman 5 ls
	$pman 5 ls
$E SIMPLE TEST SHOULD PRINT NAME SECTION AND EXAMPLES: pman $F
	$pman $F
$E NAME: pman -n $F
	$pman -n $F
$E COMPACT SYNTAX: pman -cs $F
	$pman -cs $F
$E PROCESSED MAN OUTPUT: pman -cnwv - '<' pman.nr
	$pman -cnwv - < pman.nr
$E HEADER WITH BLANK LINES WITH WARNINGS: pman -wbh $F
	$pman -wbh $F
$E COMPACT LISTING OF SEE ALSO FOR FILES: pman -c -+ '"SEE ALSO"' $F
	$pman -c -+ "SEE ALSO" $F
$E COMPACT VARIABLES FOR SECTION 7 MAN MACROS: pman -cv 7 man
	$pman -cv 7 man
$E REQUESTS FOR SECTION 7 MAN MACROS: pman -+ REQ 7 man
	$pman -+ REQ 7 man
$E EXAMPLES FOR SECT 3 VARARGS W/OUT SECT SUPPLIED: pman -e varargs
	$pman -e varargs
$E ABOVE, WITH -+ OPTION AND SECT SUPPLIED: pman -+ EXAMPLE 3 varargs
	$pman -+ EXAMPLE 3 varargs
$E OPTIONS AND NAME FOR FTP: pman -on ftp
	$pman -on ftp
$E FILES AND VARIABLES FOR MAIL: pman -v mail
	$pman -v mail
$E COMPACT HEADER AND WARNINGS FOR UUCP: pman -cwh uucp
	$pman -cwh uucp
$E HEADER AND XREFS WITH BLANKS FOR SH AND CSH: pman -hbx sh csh
	$pman -hbx sh csh
$E ALL VERSIONS OF MKDIR: pman -a mkdir
	$pman -a mkdir
