$	Verify = F$Verify(0)
$!
$! CCOM.COM
$!
$!	Run the C compiler on P1, but only if the .c file
$!	is newer than the corresponding .obj file.
$!
$! Usage:
$!	@CCOM [file [qualifiers]]
$!
$	If P1 .Eqs. "" Then -
		Inquire P1 "C Source File"
$	Name = P1 - ".C"
$	Source = Name + ".C"
$	Object = Name + ".OBJ"
$!
$! See if both files exist.  If both exist, only compile the
$! source if the revision date is greater than or equal to
$! that of the object file.
$!
$	If F$Search(Source) .Eqs. "" Then -
		Goto NoSource
$	If F$Search(Object) .Eqs. "" Then -
$		Goto Compile
$	SDate = F$File_Attributes(Source, "RDT")
$	ODate = F$File_Attributes(Object, "RDT") 
$	If SDate .Lts. ODate Then -
		Goto Bye
$!
$! Compile the program
$!
$Compile:
$	On Error Then Goto Fail
$	Write Sys$Output "Compiling " + Source
$	CC 'P2' 'Source
$	If F$Search(Object) .Eqs. "" Then -
		Goto Fail
$!
$! Done.
$!
$Bye:
$	If Verify Then -
		Set Verify
$	Exit
$!
$NoSource:
$	Write Sys$Output "%CCOM-F-NOTFOUND, file not found"
$	Goto Bye
$!
$Fail:
$	Write Sys$Output "%CCOM-F-FAIL, compile failed"
$	Goto Bye

