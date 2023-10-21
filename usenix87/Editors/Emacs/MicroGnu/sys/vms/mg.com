$	Verify = 'F$Verify(0)
$!
$! MG.COM
$!
$! Usage:
$! @MG [file1 [file2 [...]]]		! To start up MG
$! @MG [file]				! To reattach to MG after ^Z
$!
$! MG.COM implements a "kept-fork" capability for MG, allowing you to pop
$! in and out of the editor without reloading it all the time.  If a
$! process called user_MG (where user is your username) exists, this
$! command file attempts to attach to it.  If not, it silently spawns a
$! subjob to run Emacs for you. 
$!
$! To `keep' MG around once you get into it, use "suspend-emacs" (bound
$! to C-z by default) to suspend MG and attach back to the process
$! pointed to by MG$AttachTo. 
$!
$! To get back into MG from DCL enter @MG again.  You may optionally
$! specify *one* new file name, in which case MG will attempt to
$! visit that file when you re-attach. 
$!
$!----------------------------------------------------------------
$!
$! Set things up.  Change the definition of MG_Name to whatever you like.
$! You'll *have* to redefine MG_PROG, of course...
$!
$	MG_Name = F$Edit(F$Getjpi("","USERNAME"),"TRIM") + "_MG"
$	MG_Prog = "Disk$Staff:[Ccep001.Proj.Mg3]MG.Exe"
$	MG_Base = MG_Name			! Used for additions
$	If F$Length(MG_Base) .GT. 13 Then -	! Truncate base for _1,_2...
$		MG_Base = F$Extract(0,13,MG_Base)
$	Proc = F$GetJpi("","PRCNAM")
$	Master_Pid = F$Getjpi("","MASTER_PID")
$!
$! Define logical names used for communicating with MG
$!
$	Define/Nolog/Job MG$AttachTo	"''Proc'"
$	Define/Nolog/Job MG$File	" "	! No file by default
$	If P1 .Nes. "" Then -
		Define/Nolog/Job MG$File "''P1'"
$!
$! Attempt to find MG subprocess in current tree.  If found, attach
$! to it, else spawn a new MG process
$!
$	Save_Priv = F$SetPrv("NOWORLD,NOGROUP")	! Only look in job tree
$	Try_Count = 1
$Search:
$	Context = ""			! Set up process search context
$ProcLoop:
$	Pid = F$Pid(Context)		! Get next PID
$	If Pid .Eqs. "" Then -
		 Goto Spawn		! No MG_Name found; spawn a process
$	If F$GetJpi(Pid,"PRCNAM") .Nes. MG_Name Then -
		Goto Procloop		! Try next process
$! Process name matches; see if it's in our job
$	If F$GetJpi(Pid,"MASTER_PID") .Eqs. Master_Pid Then -
		Goto Attach		! Found process in our job!
$! Process name matches, but isn't in our job.  Re-start search
$	MG_Name = MG_Base + "_" + F$String(Try_Count)
$	Try_Count = Try_Count + 1
$	Goto Search
$!
$! Here to attach to a process in our tree. Set message to
$! turn off the "Attaching to..." message
$!
$Attach:
$	Message = F$Environment("MESSAGE")
$	Set Proc/Priv=('Save_Priv')		! Restore privileges
$	Set Message/NoFacility/NoIdentification/NoSeverity/NoText
$	Attach "''MG_Name'"
$	Set Message/Facility/Identification/Severity/Text
$	Goto Done
$!
$! Here if can't attach.  Spawn a new MG process
$!
$Spawn:
$	Set Process/Priv=('Save_Priv')		! Restore privileges
$	MG$MG :== $'MG_Prog'			! Avoid recursion
$	Spawn/NoLog/Proc="''MG_Name'" MG$MG 'P1' 'P2' 'P3' 'P4' 'P5' 'P6' 'P7' 'P8'
$	Delete/Symbol/Global MG$MG		! Get rid of it 
$Done:
$!
$! Here once we reconnect from MG, whether we detached or exited.
$!
$	Deassign/Job MG$File
$	Deassign/Job MG$AttachTo
$	If Verify Then -
		Set Verify
