\" .tr `'
.c2 ,
.ll 6.5i
.pr 1.25i
\" .nr si 3n
.nr pi 0
.nr bs 0
.de be	\" begin enummerate
.ba +3n
.nr zz \n(ps
.nr ps 0
..
.de ee	\" end enummerate
.ba -3n
.pp
.nr ps \n(zz
..
.ce
Edit Users Manual
.he 'EDIT USERS MANUAL''%'
.fo ''\n(mo/\n(dy/\n(yr''
.sh 1 Introduction
.pp
.ul
Edit
is a screen-oriented text editor
derived from
the standard UNIX editor
.ul
ed.
In addition to the obvious advantages of
being able to edit in two dimensions,
.ul
edit
offers a number of special features, bells, and whistles,
including:
.nr zz \n(ps
.nr ps 0
.nf
.in +4n
on-line help
crash recovery
automatic spell checking of text being entered
automatic line breaking at word boundaries
macros
editing of multiple files
shell interaction while editing
.in -4n
.pp
.nr ps \n(zz
.fi
.pp
This document is intended to serve as
a reference manual
and as
an introduction to the "novel" (i.e. different from
.i ed )
features of
.i edit.
For a tutorial description of those features shared with
.i ed
(i.e. much of "Commands", "Addressing", and "Regular Expressions")
see
"A Tutorial Introduction to the UNIX Text Editor"
and
"Advanced Editing on UNIX"
in the
.i "UNIX Programmer's Manual."
.sh 1 "The Screen"
.pp
The screen 
is divided
into two parts:
the command area
and the window area.
The last two lines of the screen
are the command area
and the remainder is the window area.
.sh 2 "The Window Area"
.pp
The text of each file being edited is displayed
in a separate window.
These windows are named after letters of the alphabet.
A window may also contain
shell commands and output, or help information.
Windows are created with the "e", "!", or help commands,
and deleted by the "q", "noshell", and "nohelp" commands.
A maximum of 3 windows may be on the screen at one time.
.(b F
.pp
The bottom line of each window is its
.i "status line."
It displays:
.ba +3n
.nr zz \n(ps
.nr ps 0
.np
The name of the window in the form "File\c
.i X ":""
where
.i X
is the letter corresponding to the window name.
.np
The name of the file displayed in the window.
.np
The current line number (".").
.np
The number of the last line in the file ("$").
.np
The page number on which the current line would be found in a
.i pr
listing.
.ba -3n
.)b
.pp
.nr ps \n(zz
All of the spaces in the status line which are not displaying
information are filled with carets ('^') as a mnemonic
reminder that the line is associated with the text above it.
.(b F
.pp
The remainder of the window contains a portion of the text of the
file being edited.
For each text line the following is displayed:
.ba +3n
.nr zz \n(ps
.nr ps 0
.np
the line number
.np
an upper case letter denoting the mark, if any
.np
a '.' if the line is the current line or a '$' if it is the
last line
.np
the text itself
.ba -3n
.)b
.pp
.nr ps \n(zz
If the text line is longer than the screen line, it is continued
on the next screen line, but without a line number.
The continued portion of the line is normally right justified.
.pp
Each window has a
.i "default line"
which is a screen line about 2/3 of the way down from
the top of the window.
.i Edit
tries to keep the current text line as close as possible
to this screen line.
.sh 2 "The Command Area"
.pp
The bottom line of the screen is the
.ul
command line.
It begins with a prompt which
varies as a function of what is being prompted for.
The usual prompt is of the form "\c
.i W :\ "
where
.i W
is the name of the current window.
This indicates that
.ul
edit
is expecting a command
(if the cursor is on the command line)
and specifies which window that command will affect.
Other prompts are described below.
The remainder of the command line is used to assemble
and display the current command
(or
option value,
buffer line,
or any other input
other than that
being entered into
one of the files being edited).
.pp
The line above the command line is the
.ul
message line
and is used to display error messages
and to prompt for additional input.
It is cleared after the successful completion of each command.
.sh 1 Modes
.pp
Edit may be considered to be in one of three modes:
command mode, cursor mode, or insert mode.
.sh 2 "Command Mode"
.pp
When the cursor is on the bottom line of the screen,
edit is in
.i command
mode
and will accept and execute commands
similar to those of
.ul
ed
(see "Commands").
The results of these commands are immediately visible
in the window area of the screen.
.sh 2 "Cursor Mode"
.pp
When the cursor is in the window area,
edit is in
.ul
cursor
mode
and changes and additions may be made to the text of the file.
Hitting RETURN returns to the point from which cursor mode was entered
(i.e. either command mode or insert mode).
Typing CONTROL-D returns to command mode.
.sh 2 "Insert Mode"
.pp
.i Insert
mode is a modification of cursor mode which is intended for
entering new line of text into the file.
It differs from cursor mode in that hitting RETURN creates
a new blank text line rather than returning to the point
from which the mode was entered,
and that the kill character will delete the entire
current line.
.pp
The
insert
mode is entered via the
.i a,
.i c,
or
.i i
commands.
It is the same as the insert mode in
.i ed
except that
lines of text before and after
the line being entered
are also
displayed.
Also,
character and line deleting are handled differently
from their usual UNIX treatment.
Entering the erase character causes the preceding character
to disappear and moves the cursor to take its place.
If the erased character was a tab,
intervening spaces are skipped over and the cursor ends up in
the correct location.
Entering the kill character causes the entire line to
disappear and positions the cursor at the
beginning of the resulting empty line.
If either of the erase or kill characters is a printing
character it may be entered into the text by preceding it
with a '\\'.
No non-printing characters except tab and newline
may be entered into the text.
If typed, they are ignored.
.pp
Insert mode is exited by entering a line consisting of a
single '.' (as in
.i ed )
or by typing CONTROL-D or CONTROL-Z.
.sh 1 "Command mode functions"
.pp
Commands to
.i edit
have a simple and regular structure: zero or
more
.i addresses
followed by a single character
.i command,
possibly
followed by parameters to the command.
These addresses specify one or more lines in the buffer.
Missing addresses are supplied by default.
.pp
Commands may require zero, one, or two addresses.
Commands which require no addresses regard the presence
of an address as an error.
Commands which accept one or two addresses
assume default addresses when insufficient are given.
For example, the 'd' command deletes lines from the file.
"3,7d" deletes lines 3 through 7, "3d" deletes line 3, and
"d" deletes the current line (the default).
.sh 2 Addresses
.pp
To understand addressing in
.i edit
it is necessary to know that at any time there is a
.i "current line."
Generally speaking, the current line is
the last line affected by a command; however,
the exact effect on the current line
is discussed under the description of
the command.
In any event, the current line is always on the screen
and is indicated by a '.' after the line number.
Addresses are constructed as follows.
.np
The character '\fB.\fR' addresses the current line.
.np
The character '$' addresses the last line of the file.
.np
A decimal number
.i n
addresses the
.i n \c
-th
line of the file.
.np
"'\fIx\fR" addresses the line marked with the name
.i x
which must be a lower-case letter.
Lines are marked with the
.i k
command described below.
.np
A regular expression enclosed in slashes '/' addresses
the line found by searching forward from the current line
and stopping at the first line containing a
string that matches the regular expression.
If necessary the search wraps around to the beginning of the
file.
.np
A regular expression enclosed in question marks '?' addresses
the line found by searching backward from the current line
and stopping at the first line containing
a string that matches the regular expression.
If necessary
the search wraps around to the end of the file.
.np
An address followed by a plus sign '+'
or a minus sign '\-' followed by a decimal number specifies that address plus
(resp. minus)\| the indicated number of lines.
The plus sign may be omitted.
.np
If an address begins with '+' or '\-'
the addition or subtraction is taken with respect to the current line;
e.g. '\-5' is understood to mean '\fB.\fR\-5'.
.np
If an address ends with '+' or '\-',
then 1 is added (resp. subtracted).
As a consequence of this rule and rule 8,
the address '\-' refers to the line before the current line.
Moreover,
trailing
'+' and '\-' characters
have cumulative effect, so '\-\-' refers to the current
line less 2.
.np
Prefixing any address construct with "\fIw\fR:" specifies that
the address is to be sought in window
.i w .
Otherwise the address automatically refers to the current window.
.np
The character '%' refers to the line in the shell window
containing the most recently submitted shell command.
.pp
Addresses are separated from each other typically by a comma
'\fB,\fR'.
They may also be separated by a semicolon
'\fB;\fR'.
In this case the current line '\fB.\fR' is set to
the previous address before the next address is interpreted.
This feature can be used to determine the starting
line for forward and backward searches ('/', '?')\|.
The second address of any two-address sequence
must correspond to a line following the line corresponding to the first address.
If more addresses are given than such a command requires,
the last one or two (depending on what is accepted)\| are used.
.sh 2 "The Commands"
.pp
In the following list of
.i edit
commands, the default addresses
are shown in parentheses.
The parentheses are not part of
the address, but are used to show that the given addresses are
the default.
Square brackets ([]) denote optional parameters
(addresses are almost always optional).
.if t Italic
.if n Underlined
characters indicate that a single character name is expected.
.(b F
.ip "(.)a"
.nr zz \n(ps
.nr ps 0
.ip "<text>"
.ip "."
.br
.nr ps \n(zz
The
.i append
command reads the given text and appends it after the
addressed line.
'.' is left at the last line input.
.)b
.(b F
.ip "(.)b<regular expression>"
.br
The
.i break
command splits the addressed line into two lines such
that the first character of the regular expression is the first
character of the second line.
.)b
.(b F
.ip "(.,.)c"
.nr zz \n(ps
.nr ps 0
.ip "<text>"
.ip "."
.br
.nr ps \n(zz
The
.i change
command deletes the addressed lines, then accepts
input text which replaces these lines. '.' is left at the last
line input; if there were none, it is left at the line preceding
the deleted lines.
The deleted lines are saved in the unnamed text buffer.
.)b
.(b F
.ip "(.,.)C"
.br
The
.i center
command centers the addressed lines
between the left and right margins.
.)b
.(b F
.ip "(.,.)d"
.br
The
.i delete
command deletes the addressed lines from the file.
The line originally after the last line deleted becomes the current
line; if the lines deleted were originally at the end, the new
last line becomes the current line.
The most recently deleted lines are saved in the unnamed text
buffer.
.)b
.(b F
.ip "e[\fIw\fP] filename"
.br
The
.i edit
command causes the entire contents of the temporary file to be
deleted, and then the named file to be read in. 'filename' is
remembered for possible use as a default file name in a subsequent
r or w command.
If
.i w
is given,
a window of that name is created if it does not exist
and subsequent editing takes place in that window.
The command form "e\fIw\fP", without a filename, is used
to change from one window to another.
.)b
.(b F
.ip "(.)f<regular expression>"
.br
The
.i find
command searches the file beginning at the addressed
line for the first occurrence of a string matching the regular
expression, and positions the cursor at the beginning of this
string. The search wraps around to the beginning of the file
if necessary. Note that there is no space between the 'f' and
the beginning of the regular expression.
.)b
.(b F
.ip "(1,$)g/regular expression/command"
.br
In the
.i global
command, the first step is to mark every line which
matches the given regular expression. Then for every such line,
the given command is executed with '.' initially set to that line.
The a, c, i, g, and v commands are not permitted.
.)b
.(b F
.ip "(.)h"
.br
The
.i head
command causes the addressed line to be moved to the
top of the window and the remainder of the window to be filled.
.)b
.(b F
.ip "help [subject]"
.br
The
.i help
command enables the help function which displays help
information in a window in the bottom half of the screen.
If subject is given, information on that subject is displayed
if available.
Otherwise a general help message is displayed.
.)b
.(b F
.ip "(.)i"
.nr zz \n(ps
.nr ps 0
.ip "<text>"
.ip "."
.br
.nr ps \n(zz
The
.i insert
command inserts the given text before the addressed
line. '.' is left at the last line input, or if there were none,
at the line before the addressed line. This command differs from
the a command only in the placement of the text.
.)b
.(b F
.ip "(.,.+1)j"
.br
The
.i join
command joins the addressed lines into a single line.
If the 'j' option is set (see options) a space is inserted
between all joined lines.
.)b
.(b F
.ip "(.,.+1)J"
.br
The
.i justify
command joins the addressed lines
one at a time until a line extending past the right margin is formed.
This line is split at the beginning of the word crossing the margin
and the second portion left justified at the right margin.
This process is continued until the end of the address range.
Note that the left adjustment of the first line in the range
is not changed.
.)b
.(b F
.ip "(.)k\fIx\fP"
.br
The
.i mark
command marks the addressed line with the name
.i x,
which must be a lower-case letter. The address form "'\fIx\fP" then
addresses this line.
.)b
.(b F
.ip "(.)l"
.br
The
.i list
command performs the same function as the "p" command
except that all non-printing characters (including tabs) are
displayed as "\e\fIoo\fP" where
.i oo
is the octal value of the character.
.)b
.(b F
.ip "(.,.) m <addr>"
.br
The
.i move
command repositions the addressed lines after the line
addressed by <addr>. The last of the moved lines becomes the
current line.
.)b
.(b F
.ip "M"
.nr zz \n(ps
.nr ps 0
.ip "<source sequence><SPACE><output sequence><RETURN>"
.br
.nr ps \n(zz
The
.i map
command provides for modifying or augmenting the default
special function mapping (see "Mapping"). The source sequence,
which must begin with a control character and contain no spaces,
is replaced by the output sequence whenever
the former is entered on the
keyboard.
.)b
.(b F
.ip "nohelp"
.br
The
.i nohelp
command disables the help function and removes the
help window from the screen.
.)b
.(b F
.ip "noshell"
.br
The
.i noshell
command removes the shell window from the screen
and terminates any processes still running.
If a shell process is still active, a warning is given.
.)b
.(b F
.ip "o<option><new value>"
.br
The
.i "set option"
command allows options to be changed.
<option> is a single character code for the desired option.
<new value> is either a numeric value for numeric options, or
one of the words "on" or "off" for boolean options.
If either <option> or <new value> is omitted, it will be prompted
for.
For more information and a list of options see "Options".
.)b
.(b F
.ip "(.)p[<buffername>]"
.br
The
.i put
command places the contents of the named text buffer
after the addressed line. If no buffer name is given,
the unnamed delete buffer is used.
.)b
.(b F
.ip "(.)P"
.br
The
.i print
command clears the screen and displays the addressed
line at the default position. The addressed line becomes the
current line. Unless the x option is set, no other lines are
displayed.
.)b
.(b F
.ip "q[\fIw\fP]"
.br
The
.i quit
command is used to exit the editor or to delete a
window from the screen. If a window name is given, that window
is removed from the screen and the remaining windows are expanded.
If no window name is given, the editor exits. No automatic write
of a file is done. However, if the file has been modified, a
message is printed and a confirmation (i.e. reentry of the quit
command) is required.
.)b
.(b F
.ip "(.)r filename"
.br
The
.i read
command reads in the given file after the addressed
line. If no file name is given, the remembered file name, if any,
is used (see "e" command). The file name is remembered if there
was no remembered file name already. Address '0' is legal for "r"
and causes the file to be read at the beginning of the edited file.
If the read is successful, '.' is left at the last line read from
the file.
.)b
.(b F
.ip "S<shell command>"
.br
The screen is cleared and the cursor positioned to the upper left
corner of the screen.
The remainder of the line after the 'S' is sent to UNIX to be
interpreted as a command.
Any text produced by the UNIX command is left on the screen after
the command returns,
but the edit windows are redrawn before the next
.i edit
command is executed.
.pp
This command differs from the '!' command in that the program
being run communicates directly with the terminal,
rather than through pipes to the shell window.
It is useful for running programs which do cursor positioning
on the screen.
.)b
.(b F
.ip "(.,.)s/regular expression/replacement/[g]"
.br
The
.i substitute
command searches each addressed line for an
occurrence of the specified regular expression. On each line on
which a match is found, all matched strings are replaced by the
replacement specified, if the global replacement indicator 'g'
appears after the command. If the global indicator does not appear
only the first occurrence of the matched string is replaced.
Any character other than space or newline may be used instead of '/'
to delimit the regular expression and replacement.
An ampersand '&' appearing in the replacement is replaced by the
string matching the regular expression. The special meaning of '&'
in this context may be suppressed by preceding it by '\e'.
The characters '\em' where m is a digit, are replaced by the text
matched by the m-th regular subexpression enclosed between '\e('
and '\e)'. When nested, parenthesized subexpressions are present,
m is determined by counting occurrences of '\e(' starting from the
left.
Lines may be split by substituting newline characters into them.
The newline in the replacement string must be escaped by preceding
it by '\e'. Lines may also be split with the "b" command or
with the split key in cursor mode.
.)b
.(b F
.ip "(.,.)t<addr>"
.br
The
.i copy
command acts just like the "m" command except that a copy
of the addressed lines is placed after address <addr> (which may
be 0). '.' is left on the last line of the copy.
.)b
.(b F
.ip "(.)u"
.br
The
.i update
command positions the cursor at the end of the addressed
line and enters the insert mode.
.)b
.(b F
.ip "(1,$)v/regular expression/command"
.br
This command is the same as the global command "g" except that
the command is executed with '.' initially set to every line
.i except
those matching the regular expression.
.)b
.(b F
.ip "(1,$)w filename"
.br
The
.i write
command writes the addressed lines onto the given
file. If the file does not exist, it is created with mode 660.
The file name is remembered if there is no remembered file name
already. If no file name is given, the remembered file name, if
any, is used (see "e" command). '.' is unchanged. If the read-only
option is set an error is indicated and no write is performed.
.)b
.(b F
.ip "(1,$)W filename"
.br
This command is the same as
"w",
except that the addressed lines are appended to the file,
rather than replacing it.
.)b
.(b F
.ip "(.)x"
.br
The
.i expand
command places the addressed line on the screen, if
necessary, and causes any empty window lines to be filled.
If the expand size is less than the window size
(which is the case for low baud rates)
only a portion of the window may be filled
(see option 'z').
Repeated execution of the 'x' command will eventually fill
the entire window.
.)b
.(b F
.ip "X filename"
.br
The
.i execute
command reads the specified file and executes
its contents as editor commands. Execute commands may be nested.
.)b
.(b F
.ip "(.,.)y[<buffername>]"
.br
The
.i yank
command places a copy of the addressed lines into the
specified text buffer where they may be used in a subsequent
put command or in a macro. If no buffer name is given, the
unnamed delete buffer is used.
.)b
.(b F
.ip <newline>
.br
An address alone on a line causes the addressed line to be
displayed.
A blank line alone is equivalent to '.h' and is used to scroll
forward through the text.
.)b
.(b F
.ip -
.br
A minus sign alone on a line moves the first line on the screen
down to the default position and fills the screen.
It is used to scroll backwards through the text.
.)b
.(b F
.ip ^
.br
A caret performs the same function as a lone minus sign.
.)b
.(b F
.ip "!<shell command>"
.br
The remainder of the line after the '!' is sent to sh(1) to be
executed. If not present, a shell window is created at the top
of the screen. All input to and output from the shell and any
processes it executes appears in this window. The command is
executed in the background and editing may continue while it is
running.
.br
.sp
If an explicit address range is given, and the shell command is
null, the addressed lines are sent to the shell, or, if a
process is currently running, to its standard input.
If any of the addressed lines begins with the current
shell prompt string, it is removed before the line is sent.
.)b
.(b F
.ip ">[<buffer name>]"
.nr zz \n(ps
.nr ps 0
.ip "<text>"
.ip "."
.br
.nr ps \n(zz
Text is entered into the named buffer until a line consisting
only of '.' in encountered. While this text is being entered,
a message appears on the message line, and the prompt is changed
to "\c
.i x >"
where
.i x
is the buffer name. This command is intended
mainly for use in initializing macros, both on-line and in
a file specified in the X command or option.
.)b
.(b F
.ip "(.,.)@<buffer name>[<delim><string>[<delim><string>...]]"
.br
For each line in the specified address range, the lines in the	
named buffer are executed as editor commands. If any of the
command lines contains a string of the form "$\fIn\fR" where
.i n
is a
digit between 1 and 9, it is replaced with the n\fIth\fR parameter
string before the command is executed. If a string of the form
"\ec" is encountered, where c is any character, it is replaced with
c. Thus to execute the command "s/$/\e$/" the macro would have to
read "s/\e$/\e\e\e$/". Macros may be nested.
.sp
Note that the command group is executed once per line in the address
range with the default for each command being used.
Thus if buffer 'b' contains "W xfile"
then the command "1,5@b" will append 5 copies of the entire current
file to xfile, whereas if buffer 'b' is ".W xfile" then the
same command will append lines 1 through 5 of the current file
to xfile.
.)b
.(b F
.ip <ctl-R>
.br
Pressing this key causes the entire screen to be redrawn
as
.i edit
thinks it looks.
This is not really a command in that it may be done at any
time in any mode.
.)b
.sh 2 "Regular Expressions"
.pp
.i Edit
supports a limited form of
.i "regular expression"
notation.
A regular expression specifies
a set of strings of characters.
A member of this set of strings is said to be
.i matched
by the regular expression.
.pp
Regular expressions are used in addresses to specify
lines and in one command
(see
.i s
above)\|
to specify a portion of a line which is to be replaced.
If it is desired to use one of
the regular expression metacharacters as an ordinary
character, that character may be preceded by '\e'.
This also applies to the character bounding the regular
expression (often '/')\| and to '\e' itself.
.pp
In the following specification for regular expressions
the word 'character' means any character but newline.
.np
Any character except a special character
matches itself.
Special characters are
the regular expression delimiter plus
\e\|[\|\fB.\fP
and sometimes ^\|*\|$.
.np
A
.b .
matches any character.
.np
A \e followed by any character except a digit or (\|) matches that character.
.np
A nonempty string
.i s
bracketed
[\c
.i \|s\| \c
]
(or
[^\c
.i \|s\| \c
])
matches any character in (or not in)
.i s.
In 
.i s,
\e has no special meaning, and ] may only appear as
the first letter.
A substring 
.i a\-b,
with
.i a
and
.i b
in ascending ASCII order, stands for the inclusive
range of ASCII characters.
.np
A regular expression of form 1-4 followed by * matches a sequence of
0 or more matches of the regular expression.
.np
A regular expression
.i x
of form 1-8, bracketed
\e( \c
.i \|x\| \c
\e)
matches what
.i x
matches.
.np
A \e followed by a digit 
.i n
matches a copy of the string that the
bracketed regular expression beginning with the
.i n \c
th
\e( matched.
.np
A regular expression
.i x
of form 1-8
followed by a regular expression
.i y
of form 1-7
matches a match for
.i x
followed by a match for
.i y,
with the
.i x
match being as long as possible while still permitting a
.i y
match.
.np
A regular expression of form 1-8 preceded by ^
(or followed by $), is constrained to matches that
begin at the left (or end at the right) end of a line.
.np
A regular expression of form 1-9 picks out the
longest among the leftmost matches in a line.
.np
An empty regular expression stands for a copy of the
last regular expression encountered.
.sh 1 "Cursor Mode Functions"
.pp
In the
.i cursor
mode text already displayed on the screen may be
changed by moving the cursor
to the desired position in the text,
then depressing keys to effect the desired change.
.pp
The
cursor
mode is entered either by depressing a
cursor motion or modify function key
while in the command or insert mode
or via the 'f'
command.
Typing RETURN leaves the
cursor
mode and returns to the previous mode.
.pp
The keys may be divided into four groups:
cursor motion,
modify function,
printing characters,
and control function.
In the description of cursor mode functions,
the term "current line" refers to the line on which
the cursor is located, not '.'.
However, some of these functions result in '.' being set to
the current line.
These are noted by "Sets dot" in the description.
.pp
The heading for each key gives
the name of the function,
the corresponding escape sequence,
and for each supported terminal type
the key (if any) which generates that sequence
in the default mapping.
Thus the line:
.in +5
end-of-line (esc-K; i100: EL, 2621: f3, h19: f5)
.in -5
indicates that the escape sequence esc-K performs the
end of line function and that this sequence is generated
by the EL key on the Infoton, the f3 key on the HP 2621,
and the f5 key on the Heath H19.
Note that on the 2621 terminal the shifted function keys
must be used as the unshifted keys transmit no characters.
.sh 2 "Cursor motion keys"
.(b F
.ip "left (esc-D; left arrow)"
.nr zz \n(ps
.nr ps 0
.ip "right (esc-C; right arrow)"
.br
.nr ps \n(zz
The left and right arrows move the cursor one position to the left
or right respectively, handling tabs correctly and refusing to
move past the end of a line.
.)b
.(b F
.ip "fast left (esc-L; 2621: f1, others: IL)"
.nr zz \n(ps
.nr ps 0
.ip "fast right (esc-M; 2621: f2, others: DL)"
.br
.nr ps \n(zz
The fast left and right keys move the cursor
to every eighth position on the line.
Attempting to move past the end (beginning) of a line
wraps around to the beginning (end) of the next (previous) line.
.)b
.(b F
.ip "up (esc-A; up arrow)"
.nr zz \n(ps
.nr ps 0
.ip "down (esc-B; down arrow)"
.br
.nr ps \n(zz
The up and down arrows move the cursor one line up or down
respectively.
Attempting to move off the top or bottom of the window causes
the window to scroll
but leaves the cursor on the same line.
Sets dot if window scrolls.
.)b
.(b F
.ip "home (esc-H; HOME)"
.br
The
.i home
key positions the cursor at the beginning of the current line
('\fB.\fR').
.)b
.(b F
.ip "end-of-line (esc-K; i100: EL, 2621: f3, h19: f5)"
.br
The end-of-line key positions the cursor at the null character
at the end of the current line.
.)b
.(b F
.ip "match bracket (esc-m)"
.br
If the cursor is positioned at a bracketing character
(one of "()[]{}")
it will be moved to the matching bracketing character,
taking into account nesting and direction.
If not on a bracketing character, the next such character
to the right on the current line will be matched.
Sets dot.
.)b
.(b F
.ip "misspelled word (esc-s)"
.br
The cursor is positioned at the beginning of the
previous occurrence
of the most recently misspelled word.
.)b
.(b F
.ip "scroll up (esc-h)"
.br
The current line is scrolled to the top of the window
and the cursor is moved to the default line.
Changes dot.
.)b
.(b F
.ip "scroll down (esc-^ or esc--)"
.br
The top line of the window is scrolled down to the default line.
Changes dot.
.)b
.sh 2 "Modify function keys"
.(b F
.ip "delete character (esc-P; 2621: f4, others: DC)"
.br
The delete character key deletes the character at the current
cursor position and shifts the remainder of
the line to the left to take its place.
.)b
.(b F
.ip "delete line (esc-d)"
.br
The current line is deleted.
Sets dot.
.)b
.(b F
.ip "append line (esc-a)"
.br
Enter insert mode after the current line.
Sets dot.
.)b
.(b F
.ip "insert line (esc-i)"
.br
Enter insert mode before the current line.
Sets dot.
.)b
.ip "split (esc-1; i100: ST, h19: f2)"
.br
The split key splits the current line
into two lines such that the current
cursor position becomes the first character of
the second line.
Sets dot.
.ip "join (esc-j)
The join key combines the current line and the following
text line into a single text line.
Sets dot.
.sh 2 "Printing characters"
.pp
Typing any printing character causes that character to be
inserted to the left of the current cursor position
and the remainder of the line shifted right to make
room for it
if the current treatment of printing characters is
set to insert.
If the current treatment is set to overwrite, the character at
the current cursor position is replaced by the typed character.
Typing the erase character causes the character to
the left of the cursor to be deleted.
The kill function is not active in
cursor
mode.
.sh 2 "Control functions"
.ip "set-insert-mode (esc-@; IC)"
.br
The set-insert-mode key sets the current treatment of printing characters
to insert for the duration of the current invocation
of cursor mode.
.ip "set-overwrite-mode (esc-2; i100: CT, h19: f1)"
.br
The set-overwrite-mode key
sets the current treatment of printing characters
to overwrite for the duration of the current invocation
of cursor mode.
.(b F
.ip "accept word (esc-o)"
.br
The most recently misspelled word is accepted as correct for the
remainder of the editing session.
.)b
.(b F
.ip "left margin (esc-l)"
.br
The left margin is set to the current cursor position.
.)b
.(b F
.ip "right margin (esc-r)"
.br
The right margin is set to the current cursor position.
.)b
.sh 1 Options
.pp
Options may be set in three ways: via the EDIT environment variable,
at invocation as flags, and while running with the option command.
There are three types of options: boolean, numeric, and string.
.nr zi \n(ii
.nr ii 3n
.ip "a"
Accept all characters (boolean)
.br
Normally all control characters, except tab, newline, escape, and
mapped characters, are ignored. If this option is set,
control characters will be
accepted as printing characters. In order to enter the erase or kill
character, it is necessary that this option be set and the character
be escaped with a '\e'.
.ip "b"
Shell beep (boolean)
.br
With this option set, the bell is sounded when the command running
in the shell window completes.
.ip c
Crash recovery (numeric)
.br
.ul
Edit
periodically saves the state of each file being edited in a pair of
temporary files:
/tmp/exxxxxYa and /tmp/exxxxxYb,
where xxxxx is the process id of the editor,
and Y is the name of the window in which the file is being edited.
If
.ul
edit
terminates normally, these files are removed.
However, if a system crash or other abnormal termination occurs during
editing, these files are preserved
(for a short time).
To recover from the point of the last update type:
.in +5
edit -cxxxxx
.in -5
where xxxxx is the process id mentioned above.
This may be discovered by typing:
.in +5
l /tmp
.in -5
and looking for an appropriate pair of files having your user id.
This "option" may only be specified at the time the editor
is invoked.
.ip "i"
Auto-indent (boolean)
.br
With this option in effect, a line of text being inserted is automatically
indented to the same level as the previous line by copying initial white
space (tabs and spaces). The level of indentation may be reduced by
backspacing. A single '.' as the first character entered terminates
insert mode, regardless of any indenting.
.ip j
Join spacing (boolean)
.br
When this option is set a space is inserted between lines joined
by the 'j' command
and any initial white space is deleted from the beginning of
the second line created by the 'b' and split commands.
Note that the default is set.
.ip l
Left break (boolean)
.br
Normally, text lines which are too long to fit on a single screen line
are broken and the last part right justified on the following screen
line.
If this option is set, the remainder will be left justified.
.ip m
Set margins (numeric)
.br
The left and right margins for the current window are set via
this option.
This option is different from others in that two values are required;
each is prompted for separately.
Setting either margin to zero has the effect of restoring it
to its default value.
.ip o
Overwrite mode (boolean)
.br
When on the default treatment of printing characters in cursor mode
is set to overwrite. The default is insert.
.ip p
Page threshold (numeric)
.br
Normally,
.ul
edit
scrolls in order to move a line from one part of the screen
to another.
If the number of screen lines to be moved exceeds the page threshold,
the entire window will be cleared and rewritten instead.
A value of zero disables paging.
.ip r
Read-only (boolean)
.br
When set the execution of all write commands is prohibited. Useful
for browsing through a file without inadvertently changing it.
.ip s
Spell checking (boolean)
.br
When this option is set a background process is started to which
all words entered into the file are passed.
This process, which is a modified version of the
.ul
spell
program, checks each word and sends back a message for each
one which it thinks is misspelled.
Pressing esc-s will position the cursor at the word in question.
Pressing esc-o will cause the spell checker to accept the word
as correctly spelled henceforth.
.ip t
Tab spacing (numeric)
.br
Sets the tab spacing to the specified value.
Only uniformly spaced tabs are supported.
The default is 8.
.ip u
Upper case text (boolean)
.br
When set, this option causes all text entered into the file
to be forced into upper case.
Note that text entered into the command line remains in
whatever case it was typed in.
.ip w
Word break (boolean)
.br
With this option in effect, whenever a text line becomes longer
than the screen line size, a new text line is started beginning
with the faulting word.
Useful for entering text without looking.
.ip x
Auto expand (boolean)
.br
When set causes automatic expansion after a 'P' command.
.ip X
Startup file (string)
.br
This option, which may only be specified at invocation,
causes the specified file to be read and executed as editor commands,
as in the 'X' command.
Useful for setting up a standard set of macros.
.ip z
Expand size (numeric)
.br
When running at low baud rates,
.ul
edit
reduces the number of lines added to the screen by an
expand command or by scrolling
so as to reduce delays.
This number is a function of the baud rate, with
fewer lines at lower baud rates.
This option allows the default to be overridden.
.nr ii \n(zi
.sh 1 "Miscellaneous Topics"
.sh 2 Buffers
.pp
Edit provides 27 text buffers
where lines may be saved for insertion into the text,
or for use as macros.
There are 26 named buffers,
each specified by a single letter,
and one unnamed buffer.
The unnamed buffer is automatically filled
with the lines to be removed
immediately before each delete or change command.
The named buffers are filled via the yank command
or the fill buffer command.
The contents of a buffer may be inserted into the text
with the put command.
.sh 2 Mapping
.pp
Edit provides a capability for translating certain input characters or
character sequences into different sequences before acting on them. This
facility is provided to compensate for discrepancies between the special
function keys on a particular terminal and the escape sequences used to
perform cursor mode functions. For example, if your terminal has a group
of keys which transmit escape sequences but these are not the same as the
default sequences used by edit (see "escape sequences"), you can use the
mapping function to cause these keys to perform the desired function.
For each supported terminal type there is a default mapping. This may be
augmented or modified in two different ways: environment variables or the
map command. During initialization, edit searches for the environment
variables MAP and MAP\c
.i term ,
where
.i term
is the code for the current terminal.
If present, the mappings defined are merged into the map with any existing
mappings being overridden.
The format for these strings is:
.nf
	<delim><source><delim><target>...
.br
.fi
where <delim> is some character not appearing in any source or target.
The string "^x", where x is any letter (or '[' for ESC) is replaced by
the corresponding control character wherever it appears in either the
source or target.
.(b F
Some restrictions:
.ba +3n
.nr zz \n(ps
.nr ps 0
.np
All source strings must begin with a control character.
.np
No source string may be a left substring of any other source string.
.ba -3n
.)b
.nr ps \n(zz
.pp
.sh 2 Margins
.pp
.i Edit
has the ability to perform limited text formating automatically:
lines may be centered or justified between adjustable left and right
margins.
Normally the left margin is column 1 and the right margin column 72.
These may be changed with the 'm' option.
.pp
When the left margin is greater than 1, white space is automatically
added to each new line which is inserted to left justify it.
If auto-indent is set, it is overridden.
When the right margin is less than 72, the word break is automatically
enabled and a new line is started whenever a word crosses the right
margin.
.pp
There is nothing rigid about either margin.
The white space at the beginning of the line may be deleted,
and an existing line may be extended past the right margin.
When this occurs as a result of corrections and modifications,
a paragraph may be reformated with the 'J' command.
Since the first line of a paragraph may be indented,
the 'J' command does not adjust the left hand boundary of the
first line of its range.
.pp
A warning:
to reduce the size of the file,
the white space at the beginning of adjusted lines is composed
of both tabs and spaces.
This means that the file will be correctly formated only
when the tab spacing used to display it is the same as that used
to create it.
.sh 1 "Using Edit"
.pp
.sh 2 "Invoking Edit"
.pp
Before
.i edit
can run, it must know what type of terminal it is talking to.
This is done by setting the TERM environment variable to
a code specifying the the terminal.
The safest way to do this is with the program
.i termtype ,
which looks in the system configuration file to find
out what kind of terminal you are (supposed to be)
logged in on.
This can all be done automatically by putting the lines:
.nf
.in +5n
TERM=`termtype`
export TERM
.in -5n
.fi
in your .profile file.
If you are using a terminal other than the one the system
thinks you are using you will have to explicitly set TERM
to the code for this terminal.
.pp
Once this detail is attended to you can begin editing by typing
.in +5
edit [-<options>] filename
.in -5
where filename is the name of the file to be edited,
and <options> is a string specifying the options you
wish set.
For example, if you want tabs to be expanded to 4 spaces
instead of 8
and no spaces placed between joined lines, type
.in +5n
edit -jt4 filename
.in -5
If you always want the same options to be set
you can save typing by setting the EDIT environment
variable to be the options string.
To have the same options as above in effect for all
editing sessions, put the lines
.in +5n
.nf
EDIT=jt4
export EDIT
.in -5n
.fi
in your .profile file.
There are several other environment variables used by
.i edit.
These will be discussed below.
.sh 2 "Basic editing"
.pp
A few seconds after typing "edit ..." your screen should
clear and fill with text from the specified file as described
above in "The screen".
At this point you can begin typing commands just
as in
.i ed
with the difference that the results of the commands are
immediately visible on the screen.
Since each line is numbered, it is easy to specify the
address of any line on the screen.
The current line has a '.' between the line number and the
text of the line,
and if any marked line is on the screen,
an upper case letter corresponding to the mark name is
displayed to the right of the line number.
.pp
To scroll forward through the file, hit RETURN;
to scroll backward type '-' or '^' followed by RETURN.
.pp
Typing one of the commands for entering new text into the file
('a', 'c', or 'i')
opens up a line on the screen and places the cursor at the
beginning of it.
Subsequent characters will be entered into the file
until a line consisting of a single '.' in the first column
is entered,
again as in
.i ed
except that the new lines are seen in context.
Some other differences:
There is no ambiguity about backspacing over tabs.
If a non-printing control character is accidently hit, it
is ignored; it is impossible (or at least difficult, see option 'a')
to enter a garbage character into a file.
If a non-printing character is already present in the file,
it is made visible in a manner which is dependent on terminal type:
on the Infoton all control characters are displayed as a
cent sign (\o'c/'),
on the 2621 they are represented by underlined letters,
and on the H19 they are displayed as letters in reverse video.
.sh 2 "Cursor mode"
.pp
Pressing one of the arrow keys
(remember to shift on the HP terminal)
moves the cursor one position in the indicated direction and
enters the
.i "cursor mode" .
Any subsequent characters which are typed will be inserted
into the text immediately to the left of the cursor and any
text to the right will be moved over to make room.
If you wish the new characters to replace the existing ones,
hit the enter-overwrite-mode key before typing any characters.
If you always want to overwrite rather than insert, set the
'o' option.
.pp
Pressing the delete-character key deletes the character at the
cursor position and shifts any characters to the right
over.
.pp
There are several line oriented operations which may be performed
from cursor mode.
In particular most command mode commands which would make
sense (eg. 'a', 'd', 'i', 'j') may be performed by prefixing
them with ESCAPE.
For example, esc-d deletes the line the cursor is on
and esc-j joins the current line to its successor.
.pp
This makes it possible to scroll the screen forward by typing
esc-h and backwards by typing esc-^.
The screen will also scroll if you attempt to move past the top
or bottom with the arrow keys.
.pp
It is possible to get into cursor mode directly from command mode.
Pressing any of the cursor mode keys or escape sequences
causes cursor mode to be entered on the first character of '.'
and then the particular function of the key to be performed.
The find ('f') command searches out the next occurrence of the
specified regular expression and enters cursor mode at the
beginning of the matched string.
.pp
To get out of cursor mode, type RETURN.
This does just what the name implies: if you had been in command
mode you will return to command mode, if you had been inserting
you will return to the point where you left off.
Thus, if you are entering text and notice an error on a previous
line, just move to it with the arrows, correct it, hit RETURN
and continue where you left off.
.sh 2 "Editing Text"
.pp
.i Edit
has several features which are oriented towards entering text
intended for human consumption (documents as opposed to programs).
With the word break ('w') option set you can enter continuous
text without having to keep looking at the screen to see if you
are getting near the end of the line.
If a word is entered which would extend past the right margin,
a new line is started and as much of the word which had
already been typed is removed from the end of the previous line
and placed at the beginning of the new line.
.pp
If you suffer from poor orthography,
you can have words checked for proper spelling as you enter them
by setting the 's' option.
Then, whenever a misspelled word is entered,
.i edit
beeps and displays a message on the message line.
Typing esc-s positions the cursor at the offending word.
If the word in question is really the name of a rare
Transylvanian neurological dysfunction
and you get tired of being beeped at every time you type it,
typing esc-o will cause
.i edit
to accept the spelling as correct.
.pp
If you are afraid of
.i nroff
or are just in a hurry,
.i edit
can do your simple text formating for you while you wait.
The 'w' option provides a crude sort of formating if you
always want your lines to be 72 characters long.
To get shorter lines or indented blocks of text,
use the 'm' option to set the margins where you want them.
With margins set text is automatically formated as it is entered.
To reformat a group of lines already in the file, use the 'J' command.
To center a title between the current margins, use the 'C' command.
.sh 2 "Editing Programs"
.pp
.i Edit
also has some features to make programmers' lives easier.
If you like to display the structure of your program
by indenting, setting the auto-indent ('i') option
will reduce the wear and tear on the tab key.
With this option set each new line entered is automatically
indented the same amount as the line above it.
To increase the level of indenting, add another tab;
to decrease, backspace over the excess.
.pp
.i Edit
has the ability to find the proper mate for the
bracketing characters ([], (), {}).
To find a matching parenthesis in a complicated expression,
put the cursor at the parenthesis in question and hit esc-m.
The cursor will move to the matching parenthesis.
If it moves to someplace unexpected there is probably an error
in the expression.
This feature is also useful for skipping over a
long procedure.
Place the cursor at the '{' at the beginning of the procedure,
hit esc-m, and the cursor will move to the end of the procedure.
.pp
If you have a
.i pr
listing of the program and wish to see a larger context than
can be shown on the screen, the page number on the status line
tells approximately what page of the listing to look on.
.sh 2 "Windows"
.pp
.i Edit
allows up to three files to be displayed and edited at the same time.
Each file is displayed in a horizontal strip of the screen
called a
.i window.
When edit is first invoked, it creates a single window,
window A,
which fills the entire window area.
To create another window, type "eb" (or "ec").
To have a file read into this window after it is created type
"eb filename".
To change back and forth between windows, type "ea" or "eb",
depending on the window you want to be in.
The name of the current window is displayed as the first
character of the command line prompt.
To delete the extra window from the screen type "qb".
.pp
It is possible to address lines in one window while working
in another by prefixing the line number by the window name
and a colon.
For example, to delete line 7 in window b, type "b:7d".
A handy phrase is "b:.h" which scrolls window b.
Lines from one window can be copied into another with the 't'
command.
"b:3,b:7t." copies lines 3 through 7 from window b and
places them after the current line in the current window.
.pp
Be warned that although it is a perfectly reasonable thing to want
to do,
editing the same file in two different windows will not work.
Since each window has its own temporary copy of the file,
changes made in one window will not be made in the other.
The changes made in whichever window is written last will determine
the contents of the file, not the union of all changes made.
It is perfectly all right to look at the same file in two different
windows as long as all changes are made in one window.
.sh 2 "The shell window"
.pp
A unique feature of
.i edit
is that shell commands and their output may be
placed in
a window.
Although the contents of this window may be edited just like
any other, the major utility of this feature is that
it allows a crt terminal to emulate the venerable teletype
by providing a sort of soft hard copy.
There is no need to scribble down cryptic lists of error messages
(or misspelled words).
Simply run the compilation (or
.i spell )
from
.i edit
and scroll through the list of messages in the shell
window while correcting them in the other window.
.pp
The line containing the most recently executed shell command
may be addressed by '%'.
If a compilation produces a long string of error messages,
"%h" will bring the beginning of the list to the top of the
shell window.
(@:.h will then scroll the list.)
"%!" will reexecute the last shell command.
.sh 2 "Marks and Buffers"
.pp
There are several ways to refer to a particular line or
group of lines in a file.
One way is to remember the line numbers.
However this method is somewhat error prone in that
arbitrary numbers are usually difficult to remember,
and that if any inserting or deleting is done in the
file above the lines in question, the line numbers will change.
These problems may be solved by
.i marking
the line (or first and last of a group of lines)
using the 'k' command.
Thereafter the line (or lines) may be referred to as '\c
.i x
(or '\c
.i x ,'\c
.i y ),
where
.i x
is the mark name given in the 'k' command.
Each mark has a window as well as a line associated with it
so that, if marks a and b are set in window b and the
current window is a, the command
"'a,'bt." will copy the desired group of lines from window
b into window a.
.pp
Marks are only aliases for other means of addressing and
so can only refer to lines that are still in one of the
files being edited.
Lines which are subsequently deleted or are in a file
no longer being edited may be preserved in a
.i buffer.
There are 26 named buffers, one for each letter of the
alphabet.
Text is placed in a buffer with the yank ('y') command.
For example "1,5ya" copies lines 1 through 5 into buffer a.
Text is extracted from a buffer with the put ('p') command;
".pa" appends a copy of buffer a after the current line.
There is one more, unnamed, buffer which is automatically filled
with the lines most recently deleted with a 'd' or 'c' command.
Thus if you make a terrible mistake and accidently delete
the wrong lines, the command ".-1p" will undo the damage.
.pp
Buffers are also used to store macros, which are described in the
next section.
.sh 2 "Maps and Macros"
.pp
.i Edit
has more special function escape sequences than many
terminals have special function keys.
If you are one of the lucky people for whom this is not the
case, or if some of the keys you do have do things you have
no desire to do, it is possible to 
associate any escape sequence
(or any arbitrary character sequence)
with a single key by using the
map function.
It is not necessary that the key being mapped generate an escape
sequence.
Any control key that is not already spoken for
(eg. TAB or RETURN) may be mapped.
.pp
For example
the EF and EP keys on the Infoton are unused.
(The CPY and PRT keys are unusable.)
.(b F
.pp
To cause the EF key to perform the join function (esc-j),
perform the following sequence from command mode:
.ba +3n
.nr zz \n(ps
.nr ps 0
.np
Enter the "M" command.
Directions on how to proceed will appear on the message line
and the prompt will change to "Mapping:\ ".
.np
Press the EF key followed by the space key.
(The command line will echo the actual characters transmitted by
the EF key (the cent sign is the escape character).)
Further instructions will appear and the prompt will
change to "To:\ ".
.np
Press the ESCAPE, 'j', and RETURN keys.
.ba -3n
.)b
.pp
.nr ps \n(zz
From now on pressing the EF key will have the same effect as
the sequence ESCAPE, 'j'.
.pp
Any character sequence, not just escape sequences, can be mapped.
To use the EP key to scroll the shell window,
substitute EP for EF in step 2 above, and enter the string
"@:.h\en" in step 3.
Note that: 1) the newline character is necessary to terminate
the command, 2) that it must be entered as "\en" since the RETURN
character would terminate the mapped sequence, and 3) this trick
will only scroll the shell window from command mode.
.pp
You can have these mappings established automatically every time
you use
.i edit
by setting the MAP environment variable.
To do this it is necessary to discover that the sequence actually
transmitted by the EF key is esc-q and by the EP key is esc-J.
The following two lines in your .profile file will accomplish this:
.nf
     MAP='/^[q/^[j/^[J/@:.h^J'
     export MAP
.br
.fi
Some explanation is in order.
The first character of the string ('|') is the delimiter which
separates the source string from the target string
(just like in the 's' command).
It also separates source/target pairs from each other.
Control characters are represented as "^\c
.i X ".
Since ESCAPE is really CONTROL-[, it is represented as "^[".
(Likewise, line feed is CONTROL-J.)
.pp
Although it is possible to save a multi-line command sequence
in a mapped key, it can be rather tedious to enter
(and difficult to correct).
For this purpose there is a more powerful and flexible macro
facility.
Any sequence of lines stored in a buffer may be executed as
edit commands by entering "@\c
.i x "
where
.i x
is the buffer name.
For example if buffer b contains the lines
.nf
	1,$s/p/P/g
	1,$s/q/Q/g
.br
.fi
then the command "@b" will change all the p's and q's in
the file to upper case.
The '@' command accepts an address range.
The command group is executed once for each line in the address
range.
Thus, if buffer a has the lines
.(b
	1,$s/p/P/g
	1,$s/q/Q/g
.)b
then "1,$@a" will have the same effect as "@b", while
"1,5@a" will only change lines 1 through 5.
.pp
A macro may have up to 9 positional parameters.
.pp
There are several ways of getting the commands into the buffer.
One way is to create an empty window, enter the commands,
and use the yank command.
This is convenient for developing complicated sequences
and edit scripts,
as it allows you to easily edit the command sequence.
To avoid the hassle of creating a new window
(and being beeped at when you don't write it out)
you can use the fill buffer command.
The command ">b" will cause all subsequent lines typed on the
command line to go into buffer b until a line consisting of a
single '.' is entered (just like insert mode).
A message appears and
the prompt is changed to b> during this process to remind you
of what is going on.
.pp
The load buffer command can be used in conjunction with the
execute ('X') command to load macro libraries.
If the file "library" consists of the lines
.(b
     >a
     s/alpha/beta/
     .
     >b
     s/omega/gamma/
     .
.)b
then the command "X library" will result in macros a and b
being defined.
.sh 2 "Getting Help"
.pp
If you get tired of carrying this weighty tome around with you,
a condensed version is available on line.
If, for example, you have forgotten the format of the map command,
type "help map" or "help M"
and a brief description of the command will appear in a window
in the bottom half of the screen.
If you have forgotten what all of the commands are, typing
"help commands" will give you a list.
If you have forgotten everything, just type "help".
When you get tired of the help messages taking up valuable
screen space, type "nohelp" and the help window will go away.
.(b F
.sh 1 Summary
.tr ~\ 
.sh 2 Commands
.sp
.TS
lb lb cb
l l l.
Command	Name	Description
_
~	~	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
a	append	T{
insert text after addressed line
T}
b	break	T{
break line at specified regular expression
T}
c	change	T{
replace addressed lines with inserted text
T}
C	center	T{
center addressed lines between margins
T}
d	delete	T{
delete addressed lines
T}
e	edit	T{
begin editing the specified file in the specified window.
used to change windows
T}
f	find	T{
position cursor at next occurrence of regular expression
T}
g	global	T{
execute command on all lines matching regular expression
T}
h	head	T{
position addressed line at top of window
T}
help	help	T{
display help information
T}
i	insert	T{
insert text before addressed line
T}
j	join	T{
join addressed lines to form a single line
T}
J	justify	T{
justify addressed lines between margins
T}
k	mark	T{
associate mark name with addressed line
T}
l	list	T{
display line with control characters expanded
T}
m	move	T{
move addressed lines
T}
M	map	T{
establish mapping between input and output sequences
T}
nohelp	nohelp	T{
remove the help window
T}
noshell	noshell	T{
remove the shell window
T}
o	option	T{
set an option value
T}
p	put	T{
put contents of named buffer after addressed line
T}
P	print	T{
display only the addressed line
T}
q	quit	T{
leave the editor or delete the named window
T}
r	read	T{
read the named file after the addressed line
T}
s	substitute	T{
substitute the replacement string for the given regular expression
T}
S	shell	T{
execute the specified unix command (terminal i/o)
T}
t	copy	T{
copy addressed lines
T}
v	global	T{
execute command on all lines not matching regular expression
T}
w	write	T{
write addressed lines to named file
T}
W	write	T{
append addressed lines to named file
T}
x	expand	T{
fill the screen
T}
X	execute	T{
execute the named file as editor commands
T}
y	yank	T{
fill the named buffer with the addressed lines
T}
!	shell	T{
execute the specified unix command (window i/o)
T}
>	fill buffer	T{
enter text into named buffer
T}
@	macro	T{
execute the named buffer as editor commands
T}
.TE
.)b
.(b F
.sh 2 "Escape Sequences"
.sp
.TS
lb lb
c l.
Sequence	Function
_
esc-@	set insert mode
esc-^	scroll down
esc--	scroll down
esc-1	split line at current character
esc-2	set overwrite mode
esc-a	append after current line
esc-A	cursor up 1 line
esc-B	cursor down 1 line
esc-C	cursor right 1 character
esc-d	delete current line
esc-D	cursor left 1 character
esc-h	scroll current line to top of screen
esc-H	cursor to start of '.'
esc-i	insert above current line
esc-j	join current line to next line
esc-K	cursor to end of current line
esc-l	set left margin
esc-L	cursor left 8 characters
esc-m	goto matching bracket character
esc-M	cursor right 8 characters
esc-o	accept misspelled word as correct
esc-P	delete current character
esc-r	set right margin
esc-s	go to misspelled word
.TE
.)b
.(b F
.sh 2 Options
.sp
.TS
cb cb cb
cll.
Option	Type	Description
_
a	boolean	Accept control characters
b	boolean	Sound bell when shell prompt returns
c	numeric	Crash recovery
d	numeric	Debugging
i	boolean	Auto-indent
j	boolean	Join spacing (default is on)
l	boolean	Left break
m	numeric	Set margins
o	boolean	Establish overwrite mode as default
p	numeric	Paging threshold (default is 0)
r	boolean	Read only mode
s	boolean	Spell checking
t	numeric	Tab spacing (default is 8)
u	boolean	Upper case text
w	boolean	Word break
x	boolean	Automatic expand
X	string	Run time startup file
z	numeric	Expand size
.TE
.)b
.(b F
.sh 2 "Supported Terminal Types"
.sp
.TS
lb lb
ll.
Code	Full name
_
2621	Hewlett-Packard 2621A
h19	Heath H19
i100	Infoton 100
v200	Visual 200
.TE
.)b
.(b F
.sh 2 "Environment Variables"
.sp
.TS
lb lb
ll.
Variable	Function
_
EDIT	Set default options
TERM	Specify terminal type code
MAP	Set default mappings for all terminal types
MAP\fIterm\fP	Set default mappings for specific terminal
.TE
.)b
.sh 1 Limitations
.pp
Some size limitations:
512 characters per line,
256 characters per global command list,
64 characters per file name,
and 128K characters in the temporary file.
The limit on the number of lines depends on the amount of core:
each line takes 1 word.
For the current PDP11 version this means a limit of about
15000
lines if only one file is being edited.
However, when multiple files are being edited
the memory management scheme can
cause the available memory to become fragmented
so that, although the total amount of memory is adequate,
no single contiguous segment is large enough.
.sh 1 Bugs
.pp
In addition to the problems mentioned above which are
inherent in the implementation of
.i edit
there are a number of things which might rightly be
considered wrong.
These will probably be fixed some day.
.np
It is not possible to explicitly change the size of a window
.np
If the same file is being edited in two different windows,
changes made in one window are not reflected in the other.
.np
The implementation of justify was quick and dirty.
However, its performance is rather lethargic.
