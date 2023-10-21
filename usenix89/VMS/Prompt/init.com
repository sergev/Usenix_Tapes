
! ------------------------- INITIAL PROMPT SETTING -----------------------
! Put this in your "LOGIN.COM" file.  Substitue the {Username} string with
! your user-id.

$ Set prompt = "[7m{Username}> [m"

! ------------------------- END OF INITIAL DCL COMMAND ------------------
