!--> file ECHERR.COM ---------- ---------- ---------- --------->
arg_string     = "''p1' ''p2' ''p3' ''p4' ''p5' ''p6' ''p7' ''p8'"
argument_line := 'arg_string
LOOP_ON_BLANK:
  length         = f$length(argument_line)
  position       = f$locate("\B", argument_line)
  exists_blank   = position .ne. length
  if .not. exists_blank then goto EXIT_LOOP_ON_BLANK
  argument_line  = f$extract(0, position, argument_line) + " " + -
                   f$extract(2 + position, length, argument_line)
END_LOOP_ON_BLANK: goto LOOP_ON_BLANK
EXIT_LOOP_ON_BLANK:

LOOP_ON_NEWLINE:
  length         = f$length(argument_line)
  position       = f$locate("\N", argument_line)
  exists_newline = position .ne. length
  first_line     = f$extract(0, position, argument_line)
  argument_line  = f$extract(2 + position, length, argument_line)
  write sys$error first_line
  if .not. exists_newline then exit
END_LOOP_ON_NEWLINE: goto LOOP_ON_NEWLINE
!<-- end  ---------- ---------- ---------- ---------- <---------
