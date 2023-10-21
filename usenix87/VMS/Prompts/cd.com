!----------------------------------------------------------------------------
!	FILE:  CD.COM:    Changes Directory and sets prompt in inverse video
!----------------------------------------------------------------------------

$MAIN:
$	W :== Write Sys$output
$	If P1 .eqs. "" Then Inquire/NoPun/Local P1 "Directory: "
$	Check_Dir = P1 + ".Dir"					
$	Check = F$Search (Check_Dir)
$	If Check .Eqs. "" Then Goto NOT_FOUND			
$	Def_String = "[." + P1 + "]"	
$	Set Def 'Def_String'					
$	Set Prompt = "[7m''P1'>[m "		 	
$	Exit
$
$NOT_FOUND:
$	W  "Invalid Directory name or Directory not found:  ", P1
$	Exit

!----------------------------- End of CD.COM ----------------------------!
