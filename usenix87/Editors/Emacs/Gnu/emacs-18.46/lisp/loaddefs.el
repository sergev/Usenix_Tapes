;; Define standard autoloads and keys of other files, for Emacs.
;; Copyright (C) 1985, 1986, 1987 Free Software Foundation, Inc.

;; This file is part of GNU Emacs.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY.  No author or distributor
;; accepts responsibility to anyone for the consequences of using it
;; or for whether it serves any particular purpose or works at all,
;; unless he says so in writing.  Refer to the GNU Emacs General Public
;; License for full details.

;; Everyone is granted permission to copy, modify and redistribute
;; GNU Emacs, but only under the conditions described in the
;; GNU Emacs General Public License.   A copy of this license is
;; supposed to have been given to you along with GNU Emacs so you
;; can know your rights and responsibilities.  It should be in a
;; file named COPYING.  Among other things, the copyright notice
;; and this notice must be preserved on all copies.

;;; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;;; Special formatting conventions are used in this file!
;;;
;;; a backslash-newline is used at the beginning of a documentation string
;;; when that string should be stored in the file etc/DOCnnn, not in core.
;;;
;;; Such strings read into Lisp as numbers (during the pure-loading phase).
;;;
;;; But you must obey certain rules to make sure the string is understood
;;; and goes into etc/DOCnnn properly.  Otherwise, the string will not go
;;; anywhere!
;;;
;;; The doc string must appear in the standard place in a call to
;;; defun, autoload, defvar or defconst.  No Lisp macros are recognized.
;;; The open-paren starting the definition must appear in column 0.
;;;
;;; In defvar and defconst, there is an additional rule:
;;; The double-quote that starts the string must be on the same
;;; line as the defvar or defconst.
;;; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

;; Know which function the debugger is!
(setq debugger 'debug)

(defconst mode-line-buffer-identification (purecopy '("Emacs: %17b")) "\
Mode-line control for identifying the buffer being displayed.
Its default value is \"Emacs: %17b\".  Major modes that edit things
other than ordinary files may change this (e.g. Info, Dired,...)")

(make-variable-buffer-local 'mode-line-buffer-identification)

(defconst mode-line-process nil "\
Mode-line control for displaying info on process status.
Normally nil in most modes, since there is no process to display.")

(make-variable-buffer-local 'mode-line-process)

(defconst mode-line-modified (purecopy '("--%1*%1*-")) "\
Mode-line control for displaying whether current buffer is modified.")

(make-variable-buffer-local 'mode-line-modified)

(setq-default mode-line-format
  (list (purecopy "")
   'mode-line-modified
   'mode-line-buffer-identification
   (purecopy "   ")
   'global-mode-string
   (purecopy "   %[(")
   'mode-name 'minor-mode-alist "%n" 'mode-line-process
   (purecopy ")%]----")
   (purecopy '(-3 . "%p"))
   (purecopy "-%-")))

(defvar minor-mode-alist nil "\
Alist saying how to show minor modes in the mode line.
Each element looks like (VARIABLE STRING);
STRING is included in the mode line iff VARIABLE's value is non-nil.")
(setq minor-mode-alist (mapcar 'purecopy
			       '((abbrev-mode " Abbrev")
				 (overwrite-mode " Ovwrt")
				 (auto-fill-hook " Fill")
				 ;; not really a minor mode...
				 (defining-kbd-macro " Def"))))

(defconst function-keymap (make-sparse-keymap) "\
Keymap containing definitions of keypad and function keys.")

;; These variables are used by autoloadable packages.
;; They are defined here so that they do not get overridden
;; by the loading of those packages.

(defconst paragraph-start "^[ \t\n\f]" "\
*Regexp for beginning of a line that starts OR separates paragraphs.")
(defconst paragraph-separate "^[ \t\f]*$" "\
*Regexp for beginning of a line that separates paragraphs.
If you change this, you may have to change paragraph-start also.")

(defconst sentence-end   (purecopy "[.?!][]\"')}]*\\($\\|\t\\|  \\)[ \t\n]*") "\
*Regexp describing the end of a sentence.
All paragraph boundaries also end sentences, regardless.")

(defconst page-delimiter "^\014" "\
*Regexp describing line-beginnings that separate pages.")

(defconst case-replace t "\
*Non-nil means query-replace should preserve case in replacements.")

;; indent.el may not be autoloading, but it still loses
;; if lisp-mode is ever called before this defvar is done.
(defvar indent-line-function 'indent-to-left-margin "\
Function to indent current line.")

(defconst only-global-abbrevs nil "\
*t means user plans to use global abbrevs only.
Makes the commands to define mode-specific abbrevs define global ones instead.")

;; Names in directory that end in one of these
;; are ignored in completion,
;; making it more likely you will get a unique match.
(setq completion-ignored-extensions
      (if (eq system-type 'vax-vms)
	  '(".obj" ".elc" ".exe" ".bin" ".lbin"
	    ".dvi" ".toc" ".log" ".aux"
	    ".lof" ".brn" ".rnt" ".mem" ".lni" ".lis"
	    ".olb" ".tlb" ".mlb" ".hlb" ".glo" ".idx" ".lot")
	'(".o" ".elc" "~" ".bin" ".lbin" ".fasl"
	  ".dvi" ".toc" ".log" ".aux"
	  ".lof" ".blg" ".bbl" ".glo" ".idx" ".lot")))

(defvar compile-command "make -k" "\
*Last shell command used to do a compilation; default for next compilation.")

(defvar dired-listing-switches "-al" "\
*Switches passed to ls for Dired.  MUST contain the `l' option.
MUST NOT contain the `F, `s' or `i'' option.")

(defconst lpr-switches nil "\
*List of strings to pass as extra switch args to lpr when it is invoked.")

(defvar tags-file-name nil "\
*File name of tag table.
To switch to a new tag table, setting this variable is sufficient.
Use the `etags' program to make a tag table file.")

(defconst shell-prompt-pattern "^[^#$%>]*[#$%>] *" "\
*Regexp used by Newline command in shell mode to match subshell prompts.
Anything from beginning of line up to the end of what this pattern matches
is deemed to be prompt, and is not reexecuted.")

(defconst ledit-save-files t "\
*Non-nil means Ledit should save files before transferring to Lisp.")
(defconst ledit-go-to-lisp-string "%?lisp" "\
*Shell commands to execute to resume Lisp job.")
(defconst ledit-go-to-liszt-string "%?liszt" "\
*Shell commands to execute to resume Lisp compiler job.")

(defconst display-time-day-and-date nil "\
*Non-nil means M-x display-time should display day and date as well as time.")

;;; Determine mode according to filename

(defvar auto-mode-alist nil "\
Alist of filename patterns vs corresponding major mode functions.
Each element looks like (REGEXP . FUNCTION).
Visiting a file whose name matches REGEXP causes FUNCTION to be called.")
(setq auto-mode-alist (mapcar 'purecopy
			      '(("\\.text$" . text-mode)
				("\\.c$" . c-mode)
				("\\.h$" . c-mode)
				("\\.tex$" . TeX-mode)
				("\\.el$" . emacs-lisp-mode)
				("\\.scm$" . scheme-mode)
				("\\.l$" . lisp-mode)
				("\\.lisp$" . lisp-mode)
				("\\.f$" . fortran-mode)
				("\\.mss$" . scribe-mode)
				("\\.pl$" . prolog-mode)
;;; Less common extensions come here
;;; so more common ones above are found faster.
				("\\.TeX$" . TeX-mode)
				("\\.sty$" . LaTeX-mode)
				("\\.bbl$" . LaTeX-mode)
				("\\.bib$" . text-mode)
				("\\.article$" . text-mode)
				("\\.letter$" . text-mode)
				("\\.texinfo$" . texinfo-mode)
				("\\.lsp$" . lisp-mode)
				("\\.prolog$" . prolog-mode)
				;; Mailer puts message to be edited in /tmp/Re.... or Message
				("^/tmp/Re" . text-mode)
				;; some news reader is reported to use this
				("^/tmp/fol/" . text-mode)
				("/Message[0-9]*$" . text-mode)
				("\\.y$" . c-mode)
				("\\.scm.[0-9]*$" . scheme-mode)
				("/\\..*emacs" . emacs-lisp-mode)
				("\\.ml$" . lisp-mode))))

(make-variable-buffer-local 'indent-tabs-mode)

(defvar ctl-x-4-map (make-keymap) "\
Keymap for subcommands of C-x 4")

;; Reduce total amount of space we must allocate during this function
;; that we will not need to keep permanently.
(garbage-collect)

;; Autoload random libraries.
;; Alphabetical order by library name.

(autoload 'add-change-log-entry "add-log"
  "\
Find change log file and add an entry for today.
With ARG, prompt for name and site of person."
  t)

(autoload '\` "backquote"
  "\
\(` FORM) Expands to a form that will generate FORM.
FORM is `almost quoted' -- see backquote.el for a description."
  nil t)

(autoload 'byte-compile-file "bytecomp"
  "\
Compile a file of Lisp code named FILENAME into a file of byte code.
The output file's name is made by appending \"c\" to the end of FILENAME."
  t)

(autoload 'byte-recompile-directory "bytecomp"
  "\
Recompile every .el file in DIRECTORY that needs recompilation.
This is if a .elc file exists but is older than the .el file.
If the .elc file does not exist, offer to compile the .el file
only if a prefix argument has been specified." 
  t)

(autoload 'batch-byte-compile "bytecomp"
  "\
Runs byte-compile-file on the files remaining on the command line.
Must be used only with -batch, and kills emacs on completion.
Each file will be processed even if an error occurred previously.
For example, invoke \"emacs -batch -f batch-byte-compile $emacs/ ~/*.el\""
  nil)


(autoload 'list-command-history "chistory"
  "\
List history of commands typed to minibuffer.
The number of commands listed is controlled by  list-command-history-max.
Calls value of  list-command-history-filter  (if non-nil) on each history
element to judge if that element should be excluded from the list.

The buffer is left in Command History mode."
  t)

(autoload 'command-history-mode "chistory"
  "\
Major mode for examining commands from  command-history.
The number of commands listed is controlled by  list-command-history-max.
The command history is filtered by  list-command-history-filter  if non-nil.

Like Emacs-Lisp Mode except that characters do not insert themselves and
Digits provide prefix arguments.  Tab does not indent.  Instead these
commands are provided:
     LF, CR	Move to the next line in the history.
     Delete	Move to the previous line in the history.

Calls the value of  command-history-hook  if that is non-nil
The Command History listing is recomputed each time this mode is
invoked."
  t)

(autoload 'repeat-matching-complex-command "chistory"
  "\
Edit and re-evaluate complex command with name matching PATTERN.
Matching occurrences are displayed, most recent first, until you
select a form for evaluation.  If PATTERN is empty (or nil), every form
in the command history is offered.  The form is placed in the minibuffer
for editing and the result is evaluated.  Prefix args don't count."
  t)


(autoload 'compare-windows "compare-w"
  "\
Compare text in current window with text in next window.
Compares the text starting at point in each window,
moving over text in each one as far as they match."
  t)

(autoload 'compile "compile"
  "\
Compile the program including the current buffer.  Default: run `make'.
Runs COMMAND, a shell command, in a separate process asynchronously
with output going to the buffer *compilation*.
You can then use the command \\[next-error] to find the next error message
and move to the source code that caused it."
  t)

(autoload 'grep "compile"
  "\
Run grep, with user-specified args, and collect output in a buffer.
While grep runs asynchronously, you can use the \\[next-error] command
to find the text that grep hits refer to."
  t)

(define-key ctl-x-map "`" 'next-error)

(autoload 'next-error "compile"
  "\
Visit next compilation error message and corresponding source code.
This operates on the output from the \\[compile] command.
If all preparsed error messages have been processed,
the error message buffer is checked for new ones.
A non-nil argument (prefix arg, if interactive)
means reparse the error message buffer and start at the first error."
  t)

(define-key esc-map "/" 'dabbrev-expand)

(autoload 'dabbrev-expand "dabbrev"
  "\
Expand previous word \"dynamically\".
Expands to the most recent, preceding word for which this is a prefix.
If no suitable preceding word is found, words following point are considered.

A positive prefix argument, N, says to take the Nth backward DISTINCT
possibility.  A negative argument says search forward.  The variable
dabbrev-backward-only may be used to limit the direction of search to
backward if set non-nil.

If the cursor has not moved from the end of the previous expansion and
no argument is given, replace the previously-made expansion
with the next possible expansion not yet tried."
  t)

(autoload 'debug "debug"
  "\
Enter debugger.  Returns if user says \"continue\".
Arguments are mainly for use when this is called
 from the internals of the evaluator.
You may call with no args, or you may
 pass nil as the first arg and any other args you like.
 In that case, the list of args after the first will 
 be printed into the backtrace buffer.")

(autoload 'cancel-debug-on-entry "debug"
  "\
Undoes effect of debug-on-entry on FUNCTION."
  t)

(autoload 'debug-on-entry "debug"
  "\
Request FUNCTION to invoke debugger each time it is called.
If the user continues, FUNCTION's execution proceeds.
Works by modifying the definition of FUNCTION,
which must be written in Lisp, not predefined."
  t)

(define-key ctl-x-map "d" 'dired)

(autoload 'dired "dired"
  "\
\"Edit\" directory DIRNAME.  Delete some files in it.
Dired displays a list of files in DIRNAME.
You can move around in it with the usual commands.
You can flag files for deletion with C-d
and then delete them by typing `x'.
Type `h' after entering dired for more info."
  t)

(define-key ctl-x-4-map "d" 'dired-other-window)

(autoload 'dired-other-window "dired"
  "\
\"Edit\" directory DIRNAME.  Like \\[dired] but selects in another window."
  t)

(autoload 'dired-noselect "dired"
  "\
Find or create a dired buffer, return it, don't select it.
Call like dired.")

(autoload 'dissociated-press "dissociate"
  "\
Dissociate the text of the current buffer.
Output goes in buffer named *Dissociation*,
which is redisplayed each time text is added to it.
Every so often the user must say whether to continue.
If ARG is positive, require ARG words of continuity.
If ARG is negative, require -ARG chars of continuity.
Default is 2." t)

(autoload 'doctor "doctor"
  "\
Switch to *doctor* buffer and start giving psychotherapy."
  t)

(autoload 'disassemble "disass"
  "\
Print disassembled code for OBJECT on (optional) STREAM.
OBJECT can be a function name, lambda expression or any function object
returned by SYMBOL-FUNCTION.  If OBJECT is not already compiled, we will
compile it (but not redefine it)."
  t)

(autoload 'electric-buffer-list "ebuff-menu"
  "\
Vaguely like ITS lunar select buffer;
combining typeoutoid buffer listing with menuoid buffer selection.

This pops up a buffer describing the set of emacs buffers.
If the very next character typed is a space then the buffer list
 window disappears.

Otherwise, one may move around in the buffer list window, marking
 buffers to be selected, saved or deleted.

To exit and select a new buffer, type Space when the cursor is on the
 appropriate line of the buffer-list window.

Other commands are much like those of buffer-menu-mode.

Calls value of  electric-buffer-menu-mode-hook  on entry if non-nil.

\\{electric-buffer-menu-mode-map}"
  t)


(autoload 'electric-command-history "echistory"
  "\
Major mode for examining and redoing commands from  command-history.
The number of command listed is controlled by  list-command-history-max.
The command history is filtered by  list-command-history-filter  if non-nil.
Combines typeout Command History list window with menu like selection
of an expression from the history for re-evaluation in the *original* buffer.

The history displayed is filtered by  list-command-history-filter  if non-nil.

This pops up a window with the Command History listing.  If the very
next character typed is Space, the listing is killed and the previous
window configuration is restored.  Otherwise, you can browse in the
Command History with  Return  moving down and  Delete  moving up, possibly
selecting an expression to be redone with Space or quitting with `Q'.

Like Emacs-Lisp Mode except that characters do not insert themselves and
Tab and linefeed do not indent.  Instead these commands are provided:
Space or !	edit then evaluate current line in history inside
		   the ORIGINAL buffer which invoked this mode.
		   The previous window configuration is restored
		   unless the invoked command changes it.
C-c C-c, C-], Q	Quit and restore previous window configuration.
LF, CR		Move to the next line in the history.
Delete		Move to the previous line in the history.
?		Provides a complete list of commands.

Calls the value of  electric-command-history-hook  if that is non-nil
The Command History listing is recomputed each time this mode is invoked."
  t)

(autoload 'edt-emulation-on "edt"
  "\
Begin emulating DEC's EDT editor.
Certain keys are rebound; including nearly all keypad keys.
Use \\[edt-emulation-off] to undo all rebindings except the keypad keys."
  t)

(autoload 'fortran-mode "fortran"
  "\
Major mode for editing fortran code.
Tab indents the current fortran line correctly. 
`do' statements must not share a common `continue'.

Type `;?' to display a list of built-in abbrevs for Fortran keywords.

Variables controlling indentation style and extra features:

 comment-start
    Normally nil in Fortran mode.  If you want to use comments
    starting with `!', set this to the string \"!\".
 fortran-do-indent
    Extra indentation within do blocks.  (default 3)
 fortran-if-indent
    Extra indentation within if blocks.  (default 3)
 fortran-continuation-indent
    Extra indentation appled to continuation statements.  (default 5)
 fortran-comment-line-column
    Amount of indentation for text within full-line comments.
 fortran-comment-indent-style
    nil    means don't change indentation of text in full-line comments,
    fixed  means indent that text at column fortran-comment-line-column
    relative  means indent at fortran-comment-line-column beyond the
 	      indentation for a line of code.
    Default value is fixed.
 fortran-comment-indent-char
    Single-character string to be inserted instead of space for 
    full-line comment indentation.  (default \" \")
 fortran-line-number-indent
    Maximum indentation for line numbers.  A line number will get
    less than this much indentation if necessary to avoid reaching
    column 5.  Default is 1.
 fortran-check-all-num-for-matching-do
    Non-nil causes all numbered lines to be treated as possible 'continue'
    statements.  (default nil)
 fortran-continuation-char
    Single-character string to be inserted in column 5 of a continuation
    line.  (default \"$\")
 fortran-comment-region
    String inserted by \\[fortran-comment-region] at start of each line in 
    region.  (default \"c$$$\")
 fortran-electric-line-number
    Non-nil causes line number digits to be moved to the correct column 
    as typed.  (default t)
 fortran-startup-message
    Set to nil to inhibit message first time fortran-mode is used.

Turning on Fortran mode calls the value of the variable fortran-mode-hook 
with no args, if that value is non-nil.
\\{fortran-mode-map}"
  t)

(autoload 'ftp-find-file "ftp"
  "\
FTP to HOST to get FILE, logging in as USER with password PASSWORD.
Interactively, HOST and FILE are specified by reading a string with colon separating
 the host from the filename.
USER and PASSWORD are defaulted from the values used when
 last ftping from HOST (unless password-remembering is disabled).
 Supply a password of the symbol `t' to override this default
 (interactively, this is done by giving a prefix arg)"
  t)

(autoload 'ftp-write-file "ftp"
  "\
FTP to HOST to write FILE, logging in as USER with password PASSWORD.
Interactively, HOST and FILE are specified by reading a string with colon separating
 the host from the filename.
USER and PASSWORD are defaulted from the values used when
 last ftping from HOST (unless password-remembering is disabled).
 Supply a password of the symbol `t' to override this default
 (interactively, this is done by giving a prefix arg)"
  t)

(autoload 'set-gosmacs-bindings "gosmacs"
  "\
Rebind some keys globally to make GNU Emacs resemble Gosling Emacs.
Use \\[set-gnu-bindings] to restore previous global bindings."
  t)

(autoload 'hanoi "hanoi"
  "\
Towers of Hanoi diversion.  Arg is number of rings."
  t)

(autoload 'Helper-help "helper"
  "\
Provide help for current mode."
  t)

(autoload 'Helper-describe-bindings "helper"
  "\
Describe local key bindings of current mode."
  t)

(autoload 'info "info"
  "\
Enter Info documentation browser."
  t)

(autoload 'Info-tagify "informat"
  "\
Create or update Info-file tag table in current buffer."
  t)

(autoload 'Info-validate "informat"
  "\
Check current buffer for validity as an Info file.
Check that every node pointer points to an existing node."
  t)

(autoload 'Info-split "informat"
  "\
Split an info file into an indirect file plus bounded-size subfiles.
Each subfile will be up 50000 characters plus one node.

To use this command, first visit a large Info file that has a tag table.
The buffer is modified into a (small) indirect info file
which should be saved in place of the original visited file.

The subfiles are written in the same directory the original file is in,
with names generated by appending `-' and a number to the original file name.

The indirect file still functions as an Info file, but it contains
just the tag table and a directory of subfiles."
  t)

(autoload 'batch-info-validate "informat"
  "\
Runs  Info-validate  on the files remaining on the command line.
Must be used only with -batch, and kills emacs on completion.
Each file will be processed even if an error occurred previously.
For example, invoke \"emacs -batch -f batch-info-validate $info/ ~/*.info\""
  nil)

(autoload 'ledit-mode "ledit"
  "\
Major mode for editing text and stuffing it to a Lisp job.
Like Lisp mode, plus these special commands:
  M-C-d	-- record defun at or after point
	   for later transmission to Lisp job.
  M-C-r -- record region for later transmission to Lisp job.
  C-x z -- transfer to Lisp job and transmit saved text.
  M-C-c -- transfer to Liszt (Lisp compiler) job
	   and transmit saved text.
To make Lisp mode automatically change to Ledit mode,
do (setq lisp-mode-hook 'ledit-from-lisp-mode)"
  t)

(autoload 'ledit-from-lisp-mode "ledit")

(autoload 'lpr-buffer "lpr"
  "\
Print contents of buffer as with Unix command `lpr'.
`lpr-switches' is a list of extra switches (strings) to pass to lpr."
  t)

(autoload 'print-buffer "lpr"
  "\
Print contents of buffer as with Unix command `lpr -p'.
`lpr-switches' is a list of extra switches (strings) to pass to lpr."
  t)

(autoload 'lpr-region "lpr"
  "\
Print contents of region as with Unix command `lpr'.
`lpr-switches' is a list of extra switches (strings) to pass to lpr."
  t)

(autoload 'print-region "lpr"
  "\
Print contents of region as with Unix command `lpr -p'.
`lpr-switches' is a list of extra switches (strings) to pass to lpr."
  t)

(autoload 'insert-kbd-macro "macros"
  "\
Insert in buffer the definition of kbd macro NAME, as Lisp code.
Second argument KEYS non-nil means also record the keys it is on.
 (This is the prefix argument, when calling interactively.)

This Lisp code will, when executed, define the kbd macro with the
same definition it has now.  If you say to record the keys,
the Lisp code will also rebind those keys to the macro.

To save a kbd macro, visit a file of Lisp code such as your ~/.emacs,
use this command, and then save the file."
  t)

(define-key ctl-x-map "q" 'kbd-macro-query)

(autoload 'kbd-macro-query "macros"
  "\
Query user during kbd macro execution.
With prefix argument, enters recursive edit,
 reading keyboard commands even within a kbd macro.
 You can give different commands each time the macro executes.
Without prefix argument, reads a character.  Your options are:
 Space -- execute the rest of the macro.
 DEL -- skip the rest of the macro; start next repetition.
 C-d -- skip rest of the macro and don't repeat it any more.
 C-r -- enter a recursive edit, then on exit ask again for a character
 C-l -- redisplay screen and ask again."
  t)

(autoload 'name-last-kbd-macro "macros"
  "\
Assign a name to the last keyboard macro defined.
One arg, a symbol, which is the name to define.
The symbol's function definition becomes the keyboard macro string.
Such a \"function\" cannot be called from Lisp, but it is a valid command
definition for the editor command loop."
  t)

(autoload 'make-command-summary "makesum"
  "\
Make a summary of current key bindings in the buffer *Summary*.
Previous contents of that buffer are killed first."
  t)

(autoload 'define-mail-alias "mailalias"
  "\
Define NAME as a mail-alias that translates to DEFINITION."
  t)

(autoload 'manual-entry "man"
  "\
Display Unix manual entry for TOPIC."
  t)

(autoload 'mh-rmail "mh-e"
  "\
Read your mail with mh." t)

(autoload 'mh-smail "mh-e"
  "\
Compose and send mail with mh." t)

(autoload 'convert-mocklisp-buffer "mlconvert"
  "\
Convert buffer of Mocklisp code to real Lisp that GNU Emacs can run."
  t)

(autoload 'modula-2-mode "modula2"
  "\
This is a mode intended to support program development in Modula-2.
All control constructs of Modula-2 can be reached by typing
Control-C followed by the first character of the construct.
\\{m2-mode-map}
  Control-c b begin         Control-c c case
  Control-c d definition    Control-c e else
  Control-c f for           Control-c h header
  Control-c i if            Control-c m module
  Control-c l loop          Control-c o or
  Control-c p procedure     Control-c Control-w with
  Control-c r record        Control-c s stdio
  Control-c t type          Control-c u until
  Control-c v var           Control-c w while
  Control-c x export        Control-c y import
  Control-c { begin-comment Control-c } end-comment
  Control-c Control-z suspend-emacs     Control-c Control-t toggle
  Control-c Control-c compile           Control-x ` next-error
  Control-c Control-l link

   m2-indent controls the number of spaces for each indentation."
  t)

(setq disabled-command-hook 'disabled-command-hook)

(autoload 'disabled-command-hook "novice")
(autoload 'enable-command "novice"
  "\
Allow COMMAND to be executed without special confirmation from now on.
The user's .emacs file is altered so that this will apply
to future sessions." t)

(autoload 'disable-command "novice"
  "\
Require special confirmation to execute COMMAND from now on.
The user's .emacs file is altered so that this will apply
to future sessions." t)

(autoload 'nroff-mode "nroff-mode"
  "\
Major mode for editing text intended for nroff to format.
\\{nroff-mode-map}
Turning on Nroff mode runs text-mode-hook, then nroff-mode-hook.
Also, try nroff-electric-mode, for automatically inserting
closing requests for requests that are used in matched pairs."
  t)

(autoload 'list-options "options"
  "\
Display a list of Emacs user options, with values and documentation."
  t)

(autoload 'edit-options "options"
  "\
Edit a list of Emacs user option values.
Selects a buffer containing such a list,
in which there are commands to set the option values.
Type \\[describe-mode] in that buffer for a list of commands."
  t)

(autoload 'outline-mode "outline"
  "\
Set up Emacs for editing an outline, doing selective hiding of text."
  t)

(autoload 'edit-picture "picture"
  "\
Switch to Picture mode, in which a quarter-plane screen model is used.
Printing characters replace instead of inserting themselves with motion
afterwards settable by these commands:
  M-`	  Move left after insertion.
  M-'	  Move right after insertion.
  M--	  Move up after insertion.
  M-=	  Move down after insertion.
  C-c `	  Move northwest (nw) after insertion.
  C-c '	  Move northeast (ne) after insertion.
  C-c /	  Move southwest (sw) after insertion.
  C-c \\   Move southeast (se) after insertion.
The current direction is displayed in the mode line.  The initial
direction is right.  Whitespace is inserted and tabs are changed to
spaces when required by movement.  You can move around in the buffer
with these commands:
  C-p	  Move vertically to SAME column in previous line.
  C-n	  Move vertically to SAME column in next line.
  C-e	  Move to column following last non-whitespace character.
  C-f	  Move right inserting spaces if required.
  C-b	  Move left changing tabs to spaces if required.
  C-c C-f Move in direction of current picture motion.
  C-c C-b Move in opposite direction of current picture motion.
  Return  Move to beginning of next line.
You can edit tabular text with these commands:
  M-Tab	  Move to column beneath (or at) next interesting charecter.
	    `Indents' relative to a previous line.
  Tab	  Move to next stop in local tab stop list.
  C-c Tab Set tab stops according to context of this line.
	    With ARG resets tab stops to default (global) value.
	    See also documentation of variable	picture-tab-chars
	    which defines \"interesting character\".  You can manually
	    change the tab stop list with command \\[edit-tab-stops].
You can manipulate text with these commands:
  C-d	  Clear (replace) ARG columns after point without moving.
  C-c C-d Delete char at point - the command normally assigned to C-d.
  Delete  Clear (replace) ARG columns before point, moving back over them.
  C-k	  Clear ARG lines, advancing over them.	 The cleared
	    text is saved in the kill ring.
  C-o	  Open blank line(s) beneath current line.
You can manipulate rectangles with these commands:
  C-c C-k Clear (or kill) a rectangle and save it.
  C-c k	  Like C-c C-k except rectangle is saved in named register.
  C-c C-y Overlay (or insert) currently saved rectangle at point.
  C-c y	  Like C-c C-y except rectangle is taken from named register.
  \\[copy-rectangle-to-register]   Copies a rectangle to a register.
  \\[advertised-undo]   Can undo effects of rectangle overlay commands
	    commands if invoked soon enough.
You can return to the previous mode with:
  C-c C-c Which also strips trailing whitespace from every line.
	    Stripping is suppressed by supplying an argument.

Entry to this mode calls the value of  edit-picture-hook  if non-nil.

Note that Picture mode commands will work outside of Picture mode, but
they are not defaultly assigned to keys."
  t)

(fset 'picture-mode 'edit-picture)

(autoload 'prolog-mode "prolog"
  "\
Major mode for editing Prolog code for Prologs.
Blank lines and `%%...' separate paragraphs.  `%'s start comments.
Commands:
\\{prolog-mode-map}
Entry to this mode calls the value of prolog-mode-hook
if that value is non-nil."
  t)

(autoload 'run-prolog "prolog"
  "\
Run an inferior Prolog process, input and output via buffer *prolog*."
  t)


(autoload 'clear-rectangle "rect"
  "\
Blank out rectangle with corners at point and mark.
The text previously in the region is overwritten by the blanks."
  t)

(autoload 'delete-rectangle "rect"
  "\
Delete (don't save) text in rectangle with point and mark as corners.
The same range of columns is deleted in each line
starting with the line where the region begins
and ending with the line where the region ends."
  t)

(autoload 'delete-extract-rectangle "rect"
  "\
Return and delete contents of rectangle with corners at START and END.
Value is list of strings, one for each line of the rectangle.")

(autoload 'extract-rectangle "rect"
  "\
Return contents of rectangle with corners at START and END.
Value is list of strings, one for each line of the rectangle.")

(autoload 'insert-rectangle "rect"
  "\
Insert text of RECTANGLE with upper left corner at point.
RECTANGLE's first line is inserted at point,
its second line is inserted at a point vertically under point, etc.
RECTANGLE should be a list of strings.")

(autoload 'kill-rectangle "rect"
  "\
Delete rectangle with corners at point and mark; save as last killed one.
Calling from program, supply two args START and END, buffer positions.
But in programs you might prefer to use delete-extract-rectangle."
  t)

(autoload 'open-rectangle "rect"
  "\
Blank out rectangle with corners at point and mark, shifting text right.
The text previously in the region is not overwritten by the blanks,
but insted winds up to the right of the rectangle."
  t)

(autoload 'yank-rectangle "rect"
  "\
Yank the last killed rectangle with upper left corner at point."
  t)

(autoload 'rnews "rnews"
  "\
Read USENET news for groups for which you are a member and add or
delete groups.
You can reply to articles posted and send articles to any group.

Type \\[describe-mode] once reading news to get a list of rnews commands."
  t)

(autoload 'news-post-news "rnewspost"
  "\
Edit a new USENET news article to be posted.

Type \\[describe-mode] once editing the article to get a list of commands."
  t)
(fset 'sendnews 'news-post-news)
(fset 'postnews 'news-post-news)

(autoload 'rmail "rmail"
  "\
Read and edit incoming mail.
Moves messages into file named by  rmail-file-name  (a babyl format file)
 and edits that file in Rmail Mode.
Type \\[describe-mode] once editing that file, for a list of Rmail commands.

May be called with filename as argument;
then performs rmail editing on that file,
but does not copy any new mail into the file."
  t)

(autoload 'rmail-input "rmail"
  "\
Run Rmail on file FILENAME."
  t)

(defconst rmail-dont-reply-to-names nil "\
*A regular expression specifying names to prune in replying to messages.
nil means don't reply to yourself.")

(defconst rmail-ignored-headers "^via:\\|^mail-from:\\|^origin:\\|^status:\\|^received:\\|^[a-z-]*message-id:\\|^summary-line:\\|^errors-to:" "\
*Gubbish header fields one would rather not see.")

(defvar rmail-delete-after-output nil "\
*Non-nil means automatically delete a message that is copied to a file.")

;;; Others are in paths.el.

(autoload 'run-scheme "xscheme"
  "\
Run an inferior Scheme process.
Output goes to the buffer `*scheme*'.
With argument, asks for a command line."
  t)

(autoload 'scheme-mode "scheme"
  "\
Set up things for editing scheme code,
or for running emacs as an inferior editor under scheme
to edit and zap functions."
  t)

(autoload 'scribe-mode "scribe"
  "\
Major mode for editing files of Scribe (a text formatter) source.
Scribe-mode is similar text-mode, with a few extra commands added.
\\{scribe-mode-map}

Interesting variables:

scribe-fancy-paragraphs
  Non-nil makes Scribe mode use a different style of paragraph separation.

scribe-electric-quote
  Non-nil makes insert of double quote use `` or '' depending on context.

scribe-electric-parenthesis
  Non-nil makes an open-parenthesis char (one of `([<{')
  automatically insert its close if typed after an @Command form."
  t)

;; Useful to set in site-init.el
(defconst send-mail-function 'sendmail-send-it "\
Function to call to send the current buffer as mail.
The headers are delimited by a string found in mail-header-separator.")

(defconst mail-self-blind nil "\
*Non-nil means insert BCC to self in messages to be sent.
This is done when the message is initialized,
so you can remove or alter the BCC field to override the default.")

(defconst mail-interactive nil "\
*Non-nil means when sending a message wait for and display errors.
nil means let mailer mail back a message to report errors.")

(defconst mail-yank-ignored-headers "^via:\\|^mail-from:\\|^origin:\\|^status:\\|^remailed\\|^received:\\|^[a-z-]*message-id:\\|^summary-line:\\|^to:\\|^cc:\\|^subject:\\|^in-reply-to:\\|^return-path:" "\
Delete these headers from old message when it's inserted in a reply.")

(defconst mail-header-separator "--text follows this line--" "\
*Line used to separate headers from text in messages being composed.")

(defconst mail-archive-file-name nil "\
*Name of file to write all outgoing messages in, or nil for none.")

(defvar mail-aliases t "\
Alias of mail address aliases,
or t meaning should be initialized from .mailrc.")

(autoload 'mail-other-window "sendmail"
  "\
Like mail command but displays in other window."
  t)

(autoload 'mail "sendmail"
  "\
Edit a message to be sent.  Argument means resume editing (don't erase).
Returns with message buffer selected; value t if message freshly initialized.
While editing message, type C-c C-c to send the message and exit.

Various special commands starting with C-c are available in sendmail mode
to move to message header fields.  Type C-c ? for a list of them.

If mail-self-blind is non-nil, a BCC to yourself is inserted
when the message is initialized.

If mail-default-reply-to is non-nil, it should be an address (a string);
a Reply-to: field with that address is inserted.

If mail-archive-file-name is non-nil, an FCC field with that file name
is inserted.

If mail-setup-hook is bound, its value is called with no arguments
after the message is initialized.  It can add more default fields.

When calling from a program, the second through fifth arguments
 TO, SUBJECT, CC and IN-REPLY-TO specify if non-nil
 the initial contents of those header fields.
 These arguments should not have final newlines.
The sixth argument REPLYBUFFER is a buffer whose contents
 should be yanked if the user types C-c y."
  t)

(define-key ctl-x-4-map "m" 'mail-other-window)
(define-key ctl-x-map "m" 'mail)

;; used in mail-utils
(defvar mail-use-rfc822 nil "\
*If non-nil, use a full, hairy RFC822 parser on mail addresses.
Otherwise, (the default) use a smaller, somewhat faster and
often-correct parser.")


(autoload 'server-start "server"
  "\
Allow this Emacs process to be a server for client processes.
This starts a server communications subprocess through which
client \"editors\" can send your editing commands to this Emacs job.
To use the server, set up the program `etc/emacsclient' in the
Emacs distribution as your standard \"editor\"."
  t)

(autoload 'run-lisp "shell"
  "\
Run an inferior Lisp process, input and output via buffer *lisp*."
  t)

(autoload 'shell "shell"
  "\
Run an inferior shell, with I/O through buffer *shell*.
If buffer exists but shell process is not running, make new shell.
The buffer is put in shell-mode, giving commands for sending input
and controlling the subjobs of the shell.  See shell-mode."
  t)

(autoload 'sort-lines "sort"
  "\
Sort lines in region alphabetically; argument means descending order.
Called from a program, there are three arguments:
REVERSE (non-nil means reverse order), BEG and END (region to sort)."
  t)

(autoload 'sort-paragraphs "sort"
  "\
Sort paragraphs in region alphabetically; argument means descending order.
Called from a program, there are three arguments:
REVERSE (non-nil means reverse order), BEG and END (region to sort)."
  t)

(autoload 'sort-pages "sort"
  "\
Sort pages in region alphabetically; argument means descending order.
Called from a program, there are three arguments:
REVERSE (non-nil means reverse order), BEG and END (region to sort)."
  t)

(autoload 'sort-numeric-fields "sort"
  "\
Sort lines in region numerically by the ARGth field of each line.
Fields are separated by whitespace and numbered from 1 up.
Specified field must contain a number in each line of the region.
With a negative arg, sorts by the -ARG'th field, in reverse order.
Called from a program, there are three arguments:
FIELD, BEG and END.  BEG and END specify region to sort."
  t)

(autoload 'sort-fields "sort"
  "\
Sort lines in region lexicographically by the ARGth field of each line.
Fields are separated by whitespace and numbered from 1 up.
With a negative arg, sorts by the -ARG'th field, in reverse order.
Called from a program, there are three arguments:
FIELD, BEG and END.  BEG and END specify region to sort."
  t)

(autoload 'sort-columns "sort"
  "\
Sort lines in region alphabetically by a certain range of columns.
For the purpose of this command, the region includes
the entire line that point is in and the entire line the mark is in.
The column positions of point and mark bound the range of columns to sort on."
  t)

(autoload 'sort-regexp-fields "sort"
  "\
Sort the region lexicographically as specifed by RECORD-REGEXP and KEY.
RECORD-REGEXP specifies the textual units which should be sorted.
  For example, to sort lines RECORD-REGEXP would be \"^.*$\"
KEY specifies the part of each record (ie each match for RECORD-REGEXP)
  is to be used for sorting.
  If it is \"\\digit\" then the digit'th \"\\(...\\)\" match field from
  RECORD-REGEXP is used.
  If it is \"\\&\" then the whole record is used.
  Otherwise, it is a regular-expression for which to search within the record.
If a match for KEY is not found within a record then that record is ignored.

With a negative prefix arg sorts in reverse order.

For example: to sort lines in the region by the first word on each line
 starting with the letter \"f\",
 RECORD-REGEXP would be \"^.*$\" and KEY \"\<f\w*\>\""
  t)


(autoload 'spell-buffer "spell"
  "\
Check spelling of every word in the buffer.
For each incorrect word, you are asked for the correct spelling
and then put into a query-replace to fix some or all occurrences.
If you do not want to change a word, just give the same word
as its \"correct\" spelling; then the query replace is skipped."
  t)

(autoload 'spell-region "spell"
  "\
Like spell-buffer but applies only to region.
From program, applies from START to END."
  t)

(define-key esc-map "$" 'spell-word)
(autoload 'spell-word "spell"
  "\
Check spelling of word at or before point.
If it is not correct, ask user for the correct spelling
and query-replace the entire buffer to substitute it."
  t)

(autoload 'spell-string "spell"
  "\
Check spelling of string supplied as argument."
  t)

(autoload 'untabify "tabify"
  "\
Convert all tabs in region to multiple spaces, preserving columns.
The variable tab-width controls the action."
  t)

(autoload 'tabify "tabify"
  "\
Convert multiple spaces in region to tabs when possible.
A group of spaces is partially replaced by tabs
when this can be done without changing the column they end at.
The variable tab-width controls the action."
  t)

(define-key esc-map "." 'find-tag)

(autoload 'find-tag "tags"
  "\
Find next tag (in current tag table) whose name contains TAGNAME.
 Selects the buffer that the tag is contained in
and puts point at its definition.
 If TAGNAME is a null string, the expression in the buffer
around or before point is used as the tag name.
 If second arg NEXT is non-nil (interactively, with prefix arg),
searches for the next tag in the tag table
that matches the tagname used in the previous find-tag.

See documentation of variable tags-file-name."
  t)

(define-key ctl-x-4-map "." 'find-tag-other-window)

(autoload 'find-tag-other-window "tags"
  "\
Find tag (in current tag table) whose name contains TAGNAME.
 Selects the buffer that the tag is contained in
and puts point at its definition.
 If TAGNAME is a null string, the expression in the buffer
around or before point is used as the tag name.
 If second arg NEXT is non-nil (interactively, with prefix arg),
searches for the next tag in the tag table
that matches the tagname used in the previous find-tag.

See documentation of variable tags-file-name."
  t)

(autoload 'list-tags "tags"
  "\
Display list of tags in file FILE."
  t)

(autoload 'next-file "tags"
  "\
Select next file among files in current tag table.
Non-nil argument (prefix arg, if interactive)
initializes to the beginning of the list of files in the tag table."
  t)

(autoload 'tags-apropos "tags"
  "\
Display list of all tags in tag table that contain STRING."
  t)

(define-key esc-map "," 'tags-loop-continue)
(autoload 'tags-loop-continue "tags"
  "\
Continue last tags-search or tags-query-replace command.
Used noninteractively with non-nil argument
to begin such a command.  See variable tags-loop-form."
  t)

(autoload 'tag-table-files "tags"
  "\
Return a list of files in the current tag table.")

(autoload 'tags-query-replace "tags"
  "\
Query-replace FROM with TO through all files listed in tag table.
If you exit (C-G or ESC), you can resume the query-replace
with the command \\[tags-loop-continue].

See documentation of variable tags-file-name."
  t)

(autoload 'tags-search "tags"
  "\
Search through all files listed in tag table for match for REGEXP.
Stops when a match is found.
To continue searching for next match, use command \\[tags-loop-continue].

See documentation of variable tags-file-name."
  t)

(autoload 'visit-tags-table "tags"
  "\
Tell tags commands to use tag table file FILE.
FILE should be the name of a file created with the `etags' program.
A directory name is ok too; it means file TAGS in that directory."
  t)

(autoload 'telnet "telnet"
  "\
Open a network login connection to host named HOST (a string).
Communication with HOST is recorded in a buffer *HOST-telnet*.
Normally input is edited in Emacs and sent a line at a time."
  t)

(autoload 'terminal-emulator "terminal"
  "\
Under a display-terminal emulator in BUFFER, run PROGRAM on arguments ARGS.
BUFFER's contents are made an image of the display generated by that program,
and any input typed when BUFFER is the current Emacs buffer is sent to that
program an keyboard input.
Interactively, BUFFER defaults to \"*terminal*\" and PROGRAM and ARGS
are parsed from an input-string using `sh' (which means that the ~user
filename-naming convention doesn't work.  Isn't un*x wonderful?)

The emulated display terminal will have height the same as that of the
the selected window, and width one less than that window's.

To switch buffers and leave the emulator, or to give commands
to the emulator itself (as opposed to the program running under it),
type Control-^.  The following character is an emulator command.
Type Control-^ twice to send it to the subprogram.
This escape character may be changed using the variable `terminal-escape-char'.

Meta characters may not currently be sent through the terminal emulator.

Here is a list of some of the variables which control the behaviour
of the emulator -- see their documentation for more information:
terminal-escape-char, terminal-scrolling, terminal-more-processing,
terminal-redisplay-interval.

This function calls the value of terminal-mode-hook if that exists
and is non-nil after the terminal buffer has been set up and the
subprocess started.

Presently with `termcap' only; if somebody sends us code to make this
work with `terminfo' we will try to use it."
  t)

(autoload 'latex-mode "tex-mode"
  "\
Major mode for editing files of input for LaTeX.
Makes $ and } display the characters they match.
Makes \" insert `` or '' as appropriate.

Use \\[validate-TeX-buffer] to check buffer for paragraphs containing
mismatched $'s or braces.

Use \\[TeX-region] to run LaTeX on the current region, plus the preamble
copied from the top of the file (containing \\documentstyle, etc.),
running LaTeX under a special subshell.  \\[TeX-buffer] does the whole buffer.
\\[TeX-print] prints the .dvi file made by either of those.

Special commands:
\\{TeX-mode-map}

Entering LaTeX mode calls the value of text-mode-hook,
then the value of TeX-mode-hook, and then the value
of LaTeX-mode-hook."
  t)

(autoload 'plain-tex-mode "tex-mode"
  "\
Major mode for editing files of input for plain TeX.
Makes $ and } display the characters they match.
Makes \" insert `` or '' as appropriate.

Use \\[validate-TeX-buffer] to check buffer for paragraphs containing
mismatched $'s or braces.

Use \\[TeX-region] to run TeX on the current region, plus a \"header\"
copied from the top of the file (containing macro definitions, etc.),
running TeX under a special subshell.  \\[TeX-buffer] does the whole buffer.
\\[TeX-print] prints the .dvi file made by either of those.

Special commands:
\\{TeX-mode-map}

Entering plain-TeX mode calls the value of text-mode-hook,
then the value of TeX-mode-hook, and then the value
of plain-TeX-mode-hook."
  t)

(autoload 'tex-mode "tex-mode"
  "\
Major mode for editing files of input for TeX or LaTeX.
Trys to intuit whether this file is for plain TeX or LaTeX and
calls plain-tex-mode or latex-mode."
  t)

(fset 'TeX-mode 'tex-mode)
(fset 'plain-TeX-mode 'plain-tex-mode)
(fset 'LaTeX-mode 'latex-mode)

(autoload 'texinfo-mode "texinfo"
  "\
Major mode for editing texinfo files.
These are files that are input for TEX and also to be turned
into Info files by M-x texinfo-format-buffer.
These files must be written in a very restricted and
modified version of TEX input format.

As for editing commands, like text-mode except for syntax table,
which is set up so expression commands skip texinfo bracket groups."
  t)

(autoload 'texinfo-format-buffer "texinfmt"
  "\
Process the current buffer as texinfo code, into an Info file.
The Info file output is generated in a buffer
visiting the Info file name specified in the @setfilename command."
  t)

(autoload 'batch-texinfo-format "texinfmt"
  "\
Runs  texinfo-format-buffer  on the files remaining on the command line.
Must be used only with -batch, and kills emacs on completion.
Each file will be processed even if an error occurred previously.
For example, invoke \"emacs -batch -f batch-texinfo-format $docs/ ~/*.texinfo\""
  nil)

(autoload 'display-time "time"
  "\
Display current time and load level in mode line of each buffer.
Updates automatically every minute.
If display-time-day-and-date is non-nil, the current day and date
are displayed as well."
  t)

(autoload 'underline-region "underline"
  "\
Underline all nonblank characters in the region.
Works by overstriking underscores.
Called from program, takes two arguments START and END
which specify the range to operate on."
  t)

(autoload 'ununderline-region "underline"
  "\
Remove all underlining (overstruck underscores) in the region.
Called from program, takes two arguments START and END
which specify the range to operate on."
  t)

(autoload 'ask-user-about-lock "userlock"
  "\
Ask user what to do when he wants to edit FILE but it is locked by USER.
This function has a choice of three things to do:
  do (signal 'buffer-file-locked (list FILE USER))
    to refrain from editing the file
  return t (grab the lock on the file)
  return nil (edit the file even though it is locked).
You can rewrite it to use any criterion you like to choose which one to do."
  nil)

(autoload 'ask-user-about-supersession-threat "userlock"
  "\
Ask a user who is about to modify an obsolete buffer what to do.
This function has two choices: it can return, in which case the modification
of the buffer will proceed, or it can (signal 'file-supersession (file)),
in which case the proposed buffer modification will not be made.
You can rewrite this to use any criterion you like to choose which one to do."
  nil)

(autoload 'vi-mode "vi"
  "\
Major mode that acts like the `vi' editor.
The purpose of this mode is to provide you the combined power of vi (namely,
the \"cross product\" effect of commands and repeat last changes) and Emacs.

This command redefines nearly all keys to look like vi commands.
It records the previous major mode, and any vi command for input
\(`i', `a', `s', etc.) switches back to that mode.
Thus, ordinary Emacs (in whatever major mode you had been using)
is \"input\" mode as far as vi is concerned.

To get back into vi from \"input\" mode, you must issue this command again.
Therefore, it is recommended that you assign it to a key.

Major differences between this mode and real vi :

* Limitations and unsupported features
  - Lines can be deleted/yanked with dd/yy etc., but to put back saved text as
    lines you need to specify it explicitly (use 9p/P.  Any numeric arg
    greater than one will do.)
  - Search patterns with line offset (e.g. /pat/+3 or /pat/z.)
    are not supported.
  - Ex commands are not implemented; try ':' to get some hints.
  - No line undo (i.e. the 'U' command), but multi-undo is a standard feature.
  - IMPORTANT: '1G' will go to the end of current buffer, NOT the first line
    (due to a technical difficulty with using 'G' command as an argument to
    the operators).  Use 'g' command instead.

* Modifications
  - The stopping positions for some point motion commands (word boundary,
    pattern search) are slightly different from standard 'vi'.
    Also, no automatic wrap around at end of buffer for pattern searching.
  - Since changes are done in two steps (deletion then insertion), you need
    to undo twice to completely undo a change command.  But this is not needed
    for undoing a repeated change command.
  - No need to set/unset 'magic', to search for a string with regular expr
    in it just put a prefix arg for the search commands.  Replace cmds too.
  - ^R is bound to incremental backward search, so use ^L to redraw screen.

* Extensions
  - Some standard (or modified) Emacs commands were integrated, such as
    incremental search, query replace, transpose objects, and keyboard macros.
  - In command state, ^X links to the 'ctl-x-map', and ESC can be linked to
    esc-map or set undefined.  These can give you the full power of Emacs.
  - See vi-com-map for those keys that are extensions to standard vi, e.g.
    `vi-name-last-change-or-macro', `vi-verify-spelling', `vi-locate-def',
    `vi-mark-region', and 'vi-quote-words'.  Some of them are quite handy.
  - Use \\[vi-switch-mode] to switch among different modes quickly.
  
Syntax table and abbrevs while in vi mode remain as they were in Emacs."
  t)

(autoload 'view-file "view"
  "\
View FILE in View mode, returning to previous buffer when done.
The usual Emacs commands are not available; instead,
a special set of commands (mostly letters and punctuation)
are defined for moving around in the buffer.
Space scrolls forward, Delete scrolls backward.
For list of all View commands, type ? or h while viewing.

Calls the value of  view-hook  if that is non-nil."
  t)

(autoload 'view-buffer "view"
  "\
View BUFFER in View mode, returning to previous buffer when done.
The usual Emacs commands are not available; instead,
a special set of commands (mostly letters and punctuation)
are defined for moving around in the buffer.
Space scrolls forward, Delete scrolls backward.
For list of all View commands, type ? or h while viewing.

Calls the value of  view-hook  if that is non-nil."
  t)

(autoload 'view-mode "view"
  "\
Major mode for viewing text but not editing it.")

(autoload 'vip-mode "vip"
  "\
Begin emulating the vi editor.  This is distinct from `vi-mode'.
This emulator has different capabilities from the `vi-mode' emulator.
See the text at the beginning of the source file .../lisp/vip.el
in the Emacs distribution."
  t)

(autoload 'yow "yow"
  "\
Return or display a Zippy quotation" t)
(autoload 'psychoanalyze-pinhead "yow"
  "\
Zippy goes to the analyst" t)


(define-key esc-map "\C-f" 'forward-sexp)
(define-key esc-map "\C-b" 'backward-sexp)
(define-key esc-map "\C-u" 'backward-up-list)
(define-key esc-map "\C-@" 'mark-sexp)
(define-key esc-map "\C-d" 'down-list)
(define-key esc-map "\C-k" 'kill-sexp)
(define-key esc-map "\C-n" 'forward-list)
(define-key esc-map "\C-p" 'backward-list)
(define-key esc-map "\C-a" 'beginning-of-defun)
(define-key esc-map "\C-e" 'end-of-defun)
(define-key esc-map "\C-h" 'mark-defun)
(define-key esc-map "(" 'insert-parentheses)
(define-key esc-map ")" 'move-past-close-and-reindent)
(define-key esc-map "\t" 'lisp-complete-symbol)

(define-key ctl-x-map "\C-e" 'eval-last-sexp)

(define-key ctl-x-map "/" 'point-to-register)
(define-key ctl-x-map "j" 'register-to-point)
(define-key ctl-x-map "x" 'copy-to-register)
(define-key ctl-x-map "g" 'insert-register)
(define-key ctl-x-map "r" 'copy-rectangle-to-register)

(define-key esc-map "q" 'fill-paragraph)
(define-key esc-map "g" 'fill-region)
(define-key ctl-x-map "." 'set-fill-prefix)

(define-key esc-map "[" 'backward-paragraph)
(define-key esc-map "]" 'forward-paragraph)
(define-key esc-map "h" 'mark-paragraph)
(define-key esc-map "a" 'backward-sentence)
(define-key esc-map "e" 'forward-sentence)
(define-key esc-map "k" 'kill-sentence)
(define-key ctl-x-map "\177" 'backward-kill-sentence)

(define-key ctl-x-map "[" 'backward-page)
(define-key ctl-x-map "]" 'forward-page)
(define-key ctl-x-map "\C-p" 'mark-page)
(put 'narrow-to-region 'disabled t)
(define-key ctl-x-map "p" 'narrow-to-page)
(put 'narrow-to-page 'disabled t)
(define-key ctl-x-map "l" 'count-lines-page)

(defun isearch-forward ()
  "\
Do incremental search forward.
As you type characters, they add to the search string and are found.
Type Delete to cancel characters from end of search string.
Type ESC to exit, leaving point at location found.
Type C-s to search again forward, C-r to search again backward.
Type C-w to yank word from buffer onto end of search string and search for it.
Type C-y to yank rest of line onto end of search string, etc.
Type C-q to quote control character to search for it.
Other control and meta characters terminate the search
 and are then executed normally.
The above special characters are mostly controlled by parameters;
 do M-x apropos on search-.*-char to find them.
C-g while searching or when search has failed
 cancels input back to what has been found successfully.
C-g when search is successful aborts and moves point to starting point."
  (interactive)
  (isearch t))

(defun isearch-forward-regexp ()
  "\
Do incremental search forward for regular expression.
Like ordinary incremental search except that your input
is treated as a regexp.  See \\[isearch-forward] for more info."
  (interactive)
  (isearch t t))

(defun isearch-backward ()
  "\
Do incremental search backward.
See \\[isearch-forward] for more information."
  (interactive)
  (isearch nil))

(defun isearch-backward-regexp ()
  "\
Do incremental search backward for regular expression.
Like ordinary incremental search except that your input
is treated as a regexp.  See \\[isearch-forward] for more info."
  (interactive)
  (isearch nil t))

(defvar search-last-string "" "\
Last string search for by a non-regexp search command.
This does not include direct calls to the primitive search functions,
and does not include searches that are aborted.")

(defvar search-last-regexp "" "\
Last string searched for by a regexp search command.
This does not include direct calls to the primitive search functions,
and does not include searches that are aborted.")

(defconst search-repeat-char ?\C-s "\
Character to repeat incremental search forwards.")
(defconst search-reverse-char ?\C-r "\
Character to repeat incremental search backwards.")
(defconst search-exit-char ?\e "\
Character to exit incremental search.")
(defconst search-delete-char ?\177 "\
Character to delete from incremental search string.")
(defconst search-quote-char ?\C-q "\
Character to quote special characters for incremental search.")
(defconst search-yank-word-char ?\C-w "\
Character to pull next word from buffer into search string.")
(defconst search-yank-line-char ?\C-y "\
Character to pull rest of line from buffer into search string.")
(defconst search-exit-option t "\
Non-nil means random control characters terminate incremental search.")

(defvar search-slow-window-lines 1 "\
*Number of lines in slow search display windows.
These are the short windows used during incremental search on slow terminals.
Negative means put the slow search window at the top (normally it's at bottom)
and the value is minus the number of lines.")

(defvar search-slow-speed 1200 "\
*Highest terminal speed at which to use \"slow\" style incremental search.
This is the style where a one-line window is created to show the line
that the search has reached.")

(autoload 'isearch "isearch")

(define-key global-map "\C-s" 'isearch-forward)
(define-key global-map "\C-r" 'isearch-backward)
(define-key esc-map "\C-s" 'isearch-forward-regexp)

(defun query-replace (from-string to-string &optional arg)
  "\
Replace some occurrences of FROM-STRING with TO-STRING.
As each match is found, the user must type a character saying
what to do with it.  For directions, type \\[help-command] at that time.

Preserves case in each replacement if  case-replace  and  case-fold-search
are non-nil and FROM-STRING has no uppercase letters.
Third arg DELIMITED (prefix arg if interactive) non-nil means replace
only matches surrounded by word boundaries."
  (interactive "sQuery replace: \nsQuery replace %s with: \nP")
  (perform-replace from-string to-string t nil arg)
  (message "Done"))

(defun query-replace-regexp (regexp to-string &optional arg)
  "\
Replace some things after point matching REGEXP with TO-STRING.
As each match is found, the user must type a character saying
what to do with it.  For directions, type \\[help-command] at that time.

Preserves case in each replacement if  case-replace  and  case-fold-search
are non-nil and REGEXP has no uppercase letters.
Third arg DELIMITED (prefix arg if interactive) non-nil means replace
only matches surrounded by word boundaries.
In TO-STRING, \\& means insert what matched REGEXP,
and \\=\\<n> means insert what matched <n>th \\(...\\) in REGEXP."
  (interactive "sQuery replace regexp: \nsQuery replace regexp %s with: \nP")
  (perform-replace regexp to-string t t arg)
  (message "Done"))

(defun replace-string (from-string to-string &optional delimited)
  "\
Replace occurrences of FROM-STRING with TO-STRING.
Preserve case in each match if  case-replace  and  case-fold-search
are non-nil and FROM-STRING has no uppercase letters.
Third arg DELIMITED (prefix arg if interactive) non-nil means replace
only matches surrounded by word boundaries."
  (interactive "sReplace string: \nsReplace string %s with: \nP")
  (perform-replace from-string to-string nil nil delimited)
  (message "Done"))

(defun replace-regexp (regexp to-string &optional delimited)
  "\
Replace things after point matching REGEXP with TO-STRING.
Preserve case in each match if case-replace and case-fold-search
are non-nil and REGEXP has no uppercase letters.
Third arg DELIMITED (prefix arg if interactive) non-nil means replace
only matches surrounded by word boundaries.
In TO-STRING, \\& means insert what matched REGEXP,
and \\=\\<n> means insert what matched <n>th \\(...\\) in REGEXP."
  (interactive "sReplace regexp: \nsReplace regexp %s with: \nP")
  (perform-replace regexp to-string nil t delimited)
  (message "Done"))

(define-key esc-map "%" 'query-replace)

(autoload 'perform-replace "replace")

(define-key ctl-x-map "\C-a" 'add-mode-abbrev)
(define-key ctl-x-map "\+" 'add-global-abbrev)
(define-key ctl-x-map "\C-h" 'inverse-add-mode-abbrev)
(define-key ctl-x-map "\-" 'inverse-add-global-abbrev)
(define-key esc-map "'" 'abbrev-prefix-mark)
(define-key ctl-x-map "'" 'expand-abbrev)
