!--------------------------------------------------------------------------
!	FILE: POP.COM - Sets default to next higher directory level and
!			set the prompt accordingly to the directory name.
!--------------------------------------------------------------------------

$Main:
$	Set Def [-]
$	This_Dir = F$Directory()
$	EOString = F$Length (This_Dir) - 1
$	Temp_Dir = F$Extract (0, EOString, This_Dir)
$	Found_Period = 0
$	Goto Loop_Over_Period

$Loop_Over_Period:
$	Store_String = Temp_Dir
$	Found_Period = F$locate (".", Temp_Dir)
$	Temp_Dir = F$Extract (Found_Period + 1, EOString, Temp_Dir)
$	If Temp_dir .Eqs. "" Then Goto Found_Last_Period
$	Goto Loop_Over_Period

$Found_Last_Period:
$	Set Prompt = "[7m''Store_String'>[m "
$	Exit


!------------------------------- End of POP.COM ---------------------------!
