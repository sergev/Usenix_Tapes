!--> file HELP.COM   ---------- ---------- ---------- --------->
reply = p1
if reply .nes. "" then goto HAS_REPLY
ty vmx_help_dir:help.hlp
$
LOOP:
ty vmx_help_dir:topics.hlp
$
inquire/nopunctuation reply "Topic? "
if reply .eqs. "" then exit

HAS_REPLY:
help_file = "vmx_help_dir:" + reply + ".hlp"
if f$search(help_file) -
  .nes. "" then ty 'help_file'
$
if f$search(help_file) -
  .eqs. "" then vmx_put_line "  Sorry, no documentation on ''reply'"
$
END_LOOP: goto LOOP
!<-- end  ---------- ---------- ---------- ---------- <---------
