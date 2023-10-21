!----------------------------------------------------------------------------
!	FILE:  HOME.COM  - 	Sets Default to the home directory and
!				changes prompt to user name in inverse video.
!----------------------------------------------------------------------------

$Main:
$	Set Def Sys$Login:
$ 	I_Am = F$User()
$	SOString = F$Locate (",", I_Am) + 1
$	EOString = F$Length (I_Am) - 1 - SOString
$	Prompt_String = F$Extract (SOString, EOString, I_Am)
$	Set Prompt = "[7m''Prompt_String'>[m "
$	Exit


!---------------------------- End of HOME.COM -------------------------------
