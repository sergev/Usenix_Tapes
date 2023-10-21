This is the only official distribution of MicroEmacs.  Accept no
substitutes.  It is distributed on Usenet on 6 shar archives.
Save them as uemacs1.shar ... uemacs6.shar.

Move to .../uemacs and unshar all six source files:
	uemacs[1-3].shar		Editor Mainline
	uemacs4.shar			System-specific sources
	uemacs5.shar			Terminal-specific sources
	uemacs6.shar			Out of date TeX documentation

The shar files will automatically create the following directory structure:

	.../uemacs			Editor mainline sources
	.../uemacs/sys			System specific sources
	.../uemacs/sys/atari		Atari ST
	.../uemacs/sys/cpm86
	.../uemacs/sys/msdos
	.../uemacs/sys/ultrix		(and other 4.2 bsd Unix systems)
	.../uemacs/sys/vms
	.../uemacs/tty			Terminal handler sources
	.../uemacs/tty/ansi		VT100 and other Ansi-style terminals.
	.../uemacs/tty/atari
	.../uemacs/tty/heath

To compile, choose one system, and one tty, and copy the files from the
appropriate subdirectory into the mainline directory.  Then compile all
sources, and link.  The ultrix version includes a Makefile.

Unfortunately, the documentation is not yet ready.  You will have
to dig the keypad bindings out of the source code.  Most are setup
in symbol.c, a few might be set in the terminal or system specific
modules.

You can get a current keypad binding chart by entering the command

	<META>X display-bindings

<META> is usually bound to <ESC> or <ALT>.  Here are my possibly
out-of-date bindings.  Note the following abbreviations:

	C-<something>		Control-<something>
	M-<something>		Meta -- generally the <ESC> key.
	F10			(etc. -- a keypad function key).

Up              back-line
Down            forw-line
Left            back-char
Right           forw-char
Find            search-again
Insert          yank
Remove          kill-region
Select          set-mark
Previous        back-page
Next            forw-page
Help            help
Do              execute-macro
F17             back-window
F18             forw-window
F19             enlarge-window
F20             shrink-window
Rubout          back-del-char
C-@             set-mark
C-A             goto-bol
C-B             back-char
C-C             spawn-cli
C-D             forw-del-char
C-E             goto-eol
C-F             forw-char
C-G             abort
Backspace       back-del-char
C-J             ins-nl-and-indent
C-K             kill-line
C-L             refresh
Return          ins-nl
C-N             forw-line
C-O             ins-nl-and-backup
C-P             back-line
C-Q             quote
C-R             back-i-search
C-S             forw-i-search
C-T             twiddle
C-V             forw-page
C-W             kill-region
C-Y             yank
C-Z             jeff-exit
M-!             reposition-window
M-%             query-replace
M-.             set-mark
M-<             goto-bob
M->             goto-eob
M-B             back-word
M-C             cap-word
M-D             forw-del-word
M-F             forw-word
M-L             lower-word
M-Q             quote
M-R             back-search
M-S             forw-search
M-U             upper-word
M-V             back-page
M-W             copy-region
M-X             extended-command
M-Rubout        back-del-word
C-M-G           abort
M-Backspace     back-del-word
C-M-R           display-message
C-M-V           display-version
C-X (           start-macro
C-X )           end-macro
C-X 1           only-window
C-X 2           split-window
C-X =           display-position
C-X B           use-buffer
C-X E           execute-macro
C-X G           goto-line
C-X K           kill-buffer
C-X N           forw-window
C-X P           back-window
C-X R           back-i-search
C-X S           forw-i-search
C-X Z           enlarge-window
C-X C-B         display-buffers
C-X C-C         quit
C-X C-D         display-directory
C-X C-F         set-file-name
C-X C-G         abort
C-X C-L         lower-region
C-X C-N         down-window
C-X C-O         del-blank-lines
C-X C-P         up-window
C-X C-R         file-read
C-X C-S         file-save
C-X C-U         upper-region
C-X C-V         file-visit
C-X C-W         file-write
C-X C-X         swap-dot-and-mark
C-X C-Z         shrink-window

