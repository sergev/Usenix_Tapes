;-----------------------------------------------------------------------------
;NAME
;   vt100.ml -- vt100 terminal support package
;
;SYNOPSIS
;   No externally useable functions provided; this package is self contained.
;
;REQUIREMENTS
;   VT100 or lookalike terminal
;   Emacs version 2.00 or later
;   You should set default value of vt100-ml-path below (near line 121)
;
;KEY BINDINGS (^v to page through this documentation, esc v to page back)
;	 ------ ------ ------ ------
;	| left | right| para | page |
;	|margin|margin|  up  |  up  |
;	 ------ ------ ------ ------
;	| word | word | para | page |
;	| left | right| down | down |
;	 ------ ------ ------ ------
;	| bof/ | wrap/| line |window|
;	| eof  |scroll|?/goto|nxt/dl|
;	 ------ ------ ------ ------
;	| mark | paste| macro| w f  |
;	| &cut | /undo|xeq/ed| r i  |
;	 ------------- ------| i l  |
;	|    visit    | gold | t e  |
;	|  file/shell | /help| e(s) |
;	 ------------- ------ ------
;   See below for more detail.
;
;DESCRIPTION
;   This package, when loaded, sets up global bindings for the special
;   keys on a VT100 terminal or Wang PC using VT100 emulation.
;   Care has been taken to:
;     (1) maximize functionality
;     (2) minimize keystrokes
;     (3) minimize danger from missed keystrokes
; 
;EXPLANATION
;	gold		enables secondary choice for other keys
;  gold gold (help)     visit definition file for this package
; 
;	mark&cut	if mark is not set, mark one end of text(^@)
;                       if mark is set, cut the text between	(^w)
;			  mark and present cursor position
;	paste		insert the last cut text		(^y)
;			  after the cursor position
;  gold undo		undo last command
; 
;	visit(file)	make file visible (read if necessary)	(^x ^v)
;  gold visit(shell)	make shell visible (start if necessary)	(esc s)
;
;	write file	write current buffer to its file	(^x ^s)
;  gold write files	write all modified buffers to files	(^x ^m)
; 
;       nxt window	cursor to next window			(^x n)
;  gold dl window	split the current window		(^x 2)
;
;	macro xeq	playout last defined keyboard macro	(^x e)
;  gold macro ed	examine or edit current keyboard macro
;
;	line ?		display current line number
;  gold line goto	goto specified line
;
;	word left	move cursor to first letter of next word to the left
;	word right	move cursor to character following word to the right
; 
;	left margin	move cursor to left end of current line	(^a)
;	right margin	move cursor to right end of current line(^e)
; 
;	bof		move cursor to beginning of file	(esc <)
;  gold eof		move cursor to end of file		(esc >)
; 
;	page up		move display to previous page		(esc v)
;	page down	move display to next page		(^v)
; 
;	para up		move to previous paragraph
;	para down	move to next paragraph
; 
;	wrap		toggle wrap/no-wrap modes
;  gold scroll		horizontal scrolling:
;			  set distance from begin-of-line to begin-of-screen
;
;	backspace	delete previous character
;	delete		delete character over cursor
;	^c		pause emacs and return to unix
;
;VT100 TERMCAP
;
;d1|vt100|vt-100|pt100|pt-100|dec vt100:\
;	:co#80:li#24:am:cl=50\E[;H\E[2J:bs:cm=5\E[%i%2;%2H:nd=2\E[C:up=2\E[A:\
;	:ce=3\E[K:cd=50\E[J:so=2\E[7m:se=2\E[m:us=2\E[4m:ue=2\E[m:\
;	:is=\E>\E[?3l\E[?4l\E[?5l\E[?7h\E[?8h:ks=\E[?1h\E=:ke=\E[?1l\E>:\
;	:if=/usr/lib/tabset/vt100:ku=\EOA:kd=\EOB:kr=\EOC:kl=\EOD:\
;	:kh=\E[H:k1=\EOP:k2=\EOQ:k3=\EOR:k4=\EOS:pt:sr=5\EM:xn:
;
;HISTORY
;  {4}	30-Apr-85  Bill McKeeman at WangInst
;	Elaborated keypad definitions.
;  {3}	22-Nov-84  Mike Gallaher at unipress
;	Added standard keypad and labeled key definitions.
;  {2}	07-Aug-84  Eric S. Raymond
;	Fixed a documentation bug.
;  {1}	12-Jul-84  Eric S. Raymond and Dave Langdon at Rabbit Software
;	Created.
;-----------------------------------------------------------------------------
(declare-global vt100-mark-is-set) (setq vt100-mark-is-set 0)
(declare-global vt100-gold-key-pressed) (setq vt100-gold-key-pressed 0)
(declare-global vt100-ml-path)

; Preliminaries -- let's get the environment right
(progn ver  ; check for "Emacs V2.00 (12-Feb-85) Unix version"
  (setq ver (emacs-version))
  (if (| (< (substr ver 8 1) "2") (!= (substr ver 9 1) "."))
    (error-message "Need later version of emacs for vt100.ml package")
  )
  (send-string-to-terminal			"\e=")	; set keypad alt mode
  (if (error-occurred (setq vt100-ml-path (getenv "EPATH")))
    (progn						; path to help file
      (setq vt100-ml-path "/usr/local/lib/emacs/maclib/"); reasonable default
      (message "Unix environment variable EPATH not set; default used")
      (sit-for 2)
  ) )
)
; Special editing keys and settings
(bind-to-key "delete-previous-character"	"\^h")	; backspace
(bind-to-key "delete-next-character"		"\177")	; delete
(bind-to-key "pause-emacs"			"\^c")	; control c
(bind-to-key "vt100-careful-macro"		"\^xe")	; control x e
(setq right-margin 78)					; avoid edge effects
(setq scroll-step 1)					; auto scroll amount
(setq track-eol-on-^N-^P 0)				; vertical curse ctrl

; Cursor controls
(bind-to-key "backward-character"		"\e[D")	; cursor <
(bind-to-key "previous-line"			"\e[A")	; cursor ^
(bind-to-key "next-line"			"\e[B")	; cursor v
(bind-to-key "forward-character"		"\e[C")	; cursor >
(bind-to-key "backward-character"		"\eOD")	; cursor <
(bind-to-key "previous-line"			"\eOA")	; cursor ^
(bind-to-key "next-line"			"\eOB")	; cursor v
(bind-to-key "forward-character"		"\eOC")	; cursor >

; Keypad definitions
(bind-to-key "beginning-of-line"		"\eOP") ; keypad pf1 +
(bind-to-key "end-of-line"			"\eOQ") ; keypad pf2 -
(bind-to-key "backward-paragraph"		"\eOR") ; keypad pf3 *
(bind-to-key "previous-page"			"\eOS") ; keypad pf4 /
(bind-to-key "forward-paragraph"		"\eOy") ; keypad 9
(bind-to-key "forward-word"			"\eOx") ; keypad 8
(bind-to-key "backward-word"			"\eOw") ; keypad 7
(bind-to-key "vt100-line-number"		"\eOv") ; keypad 6
(bind-to-key "vt100-wrap-or-scroll"		"\eOu") ; keypad 5
(bind-to-key "vt100-file-begin-or-end"		"\eOt") ; keypad 4
(bind-to-key "vt100-macros"			"\eOs") ; keypad 3
(bind-to-key "vt100-paste-or-undo"		"\eOr") ; keypad 2
(bind-to-key "vt100-set-mark-or-cut"		"\eOq") ; keypad 1
(bind-to-key "vt100-visit-file-or-shell"	"\eOp") ; keypad 0
(bind-to-key "next-page"			"\eOm") ; keypad - print
(bind-to-key "vt100-next-or-delete-window"	"\eOl") ; keypad , erase
(bind-to-key "vt100-gold-key"			"\eOn") ; keypad .
(bind-to-key "vt100-write-file-or-files"	"\eOM") ; keypad enter

(defun 
  (vt100-gold-key
    (if (= vt100-gold-key-pressed 0)
      (setq vt100-gold-key-pressed 1)
      (vt100-help)
    )
  )

  (vt100-paste-or-undo
    (if (= vt100-gold-key-pressed 0)
      (yank-from-killbuffer)
      (error-occurred (new-undo))
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-set-mark-or-cut
    (if (error-occurred (mark)) (setq vt100-mark-is-set 0))
    (if (= vt100-mark-is-set 0)
      (progn (set-mark) (setq vt100-mark-is-set 1))
      (progn (delete-to-killbuffer) (setq vt100-mark-is-set 0))
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-write-file-or-files
    (if (= vt100-gold-key-pressed 0)
      (progn (write-current-file)(message(concat "Wrote "(current-file-name))))
      (progn (write-modified-files) (message "Wrote all modified files."))
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-macros
    (if (= vt100-gold-key-pressed 0)
      (vt100-careful-macro)
      (progn
        (error-occurred (define-keyboard-macro "vt100-current-macro"))
        (if (error-occurred (edit-macro "vt100-current-macro"))
          (message "No macro defined.  ^x( macro-body ^x) to define macro.")
          (message "Current macro: edit and (define-buffer-macro) or delete window.")
        )
      )
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-careful-macro
    (if (error-occurred (execute-keyboard-macro))
      (if (error-occurred (vt100-current-macro))
        (message "No macro defined. ^x( macro-body ^x) to define macro.")
        (message "Executed macro.")
      )
      (message "Executed macro.")
    )
  )

  (vt100-next-or-delete-window
    (if (= vt100-gold-key-pressed 0)
      (next-window)
      (delete-window)
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-line-number
    (if (= vt100-gold-key-pressed 0)
      (message (concat "line number = " (line-number)))
      (goto-line (get-tty-string "go to line number: "))
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-file-begin-or-end
    (if (= vt100-gold-key-pressed 0)
      (beginning-of-file)
      (end-of-file)
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-visit-file-or-shell
    (if (= vt100-gold-key-pressed 0)
      (visit-file (get-tty-file "Enter file name:"))
      (shell)
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-wrap-or-scroll
    (if (= vt100-gold-key-pressed 0)
      (setq wrap-long-lines (! wrap-long-lines))
      (setq left-offset (get-tty-string "Move left screen edge to:"))
    )
    (setq vt100-gold-key-pressed 0)
  )

  (vt100-help
    (setq vt100-gold-key-pressed 0)
    (switch-to-buffer "vt100-help")
    (if (error-occurred (read-file (concat vt100-ml-path "vt100.ml")))
      (progn
        (delete-buffer "vt100-help")
        (error-message
          "Unix environment variable EPATH does not point to vt100.ml"
    ) ) )
    (setq read-only 1)
    (beginning-of-file)
  )
)
; vt100.ml ends here -----------------------------------------------------
