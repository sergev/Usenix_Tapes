
; file: edtdoc.el
; HISTORY - edt.el
;       This emulation was originally from Lynn Olson, as distibuted on Usenet:
;
;               From: lynno@tektronix.UUCP (Lynn Olson )
;               Newsgroups: net.sources
;               Subject: Emacs EDT simulator
;               Message-ID: <1782@tektronix.UUCP>
;               Date: Fri, 24-Feb-84 01:57:36 EST
;               Article-I.D.: tektroni.1782
;               Organization: Tektronix, Beaverton OR
;
;                      Here's a revised version of the CMU-Emacs EDT simulator
;               cranked out a couple of weeks ago...
;
;              Lynn Olson, now on tekfdi (Spectrum Analyzers), on 2/17/83.
;
;       It was hacked by me (yetti!mike.UUCP) to run under GNU Emacs.
;       All praise should go to her, all blame, bug-reports and suggestions
;       should go to me (yetti!mike.UUCP or symalg@YUSOL.BITNET), all flames
;       to /dev/null.  Most of this documentation is directly modified from
;       Lynn's edt.ml file, but in the true Lisp tradition, the source code
;       is *assumed* to be self-documenting :-)
;
;       For those of you familiar with the edt.ml file, I have
;       changed DEL Char, Del Word, DEL Line, and Cut to separate buffers.
;       I made some functions behave more like EDT - no chaining
;       on the Delete Line or Cut. Study the bindings carefully:
;       it's long and moderately complicated code, but a worthwhile result.
;
;    N.B. - if for some reason your keypad looses its VT100 bindings,
;               usually after exiting some recursive call, type ^L
;               to refresh the screen and reload the keypad.
;
; Most of the unshifted functions normal keypad bindings are unaltered
; with the following exceptions:
;
;       PAGE            : Next Paragraph
;       ENTER           : Other Window
;       FIND            : I-Search
;       HELP            : Describe Key
;
; LineFeed is used for the EDT function of delete-previous-word.
;
; The shifted GOLD/Keys are mostly unaltered, with the following exceptions :
;
;       RESET           : Redraw Screen and set keypad
;       SUBS            : Shell (Un*x) or sub-process-command (VMS)
;       SPECINS         : Copy Region to Kill Ring
;       FNDNXT          : Find All Occurrences (ATON)
;       HELP            : Describe Command
;
;Other new commands are GOLD/keyboard pairs, which are mapped as follows:
;
;GOLD/up-arrow          : Current line to top of window.
;GOLD/down-arrow        : Current line to bottom of window.
;GOLD/left-arrow        : Backward sentence.
;GOLD/right-arrow       : Forward sentence.
;GOLD/%                 : Goto percentage of buffer.
;GOLD/=                 : Goto line number.
;GOLD/`                 : Display line number.
;GOLD/DEL               : Delete current window.
;GOLD/BackSpace         : Delete Other Windows.
;GOLD/ <CR>             : Indent Newline.
;GOLD/ <space>          : Undo.
;GOLD                   : Mark-section-wisely.
;
;       I have also added a bunch of GOLD letter commands, which
;       generally correspond to their ^X^letter counter-parts.
;       These are simply for convenience, as these commands are
;       so frequently used.  These commands are defined the same
;       for the letter being either upper or lower-case.
;
; GOLD letter combinations:
; GOLD/b        : buffer-menu
; GOLD/d        : delete-window
; GOLD/e        : compile
; GOLD/i        : insert-file
; GOLD/l        : goto-line
; GOLD/n        : next-error
; GOLD/o        : switch-to-buffer-other-window
; GOLD/r        : revert-file
; GOLD/s        : save-buffer
; GOLD/v        : find-file-other-window
; GOLD/w        : write-file
     
;  The following commands alter the Emacs default key map. Aside from
;these changes, all other Emacs commands retain the documented bindings.
;
;  LineFeed     : Delete-Previous-Word.
;  ^Q           : Reserved for X-ON flow control.
;  ^S           : Reserved for X-OFF flow control.
;
;                 E M A C S / E D T   K E Y P A D
; Keypad keys on their own are in UPPER-CASE;
; Keypad keys preceded by GOLD are in Lower-case.
;
;+---------------+---------------+---------------+---------------+
;|     GOLD      |   HELP KEY    |     FIND      |   DEL LINE    |
;|     ----      |   --------    |     ----      |   --------    |
;| Swap Dot-Mark | Help Function |    Find All   |   Undel Line  |
;+---------------+---------------+---------------+---------------+
;|   PARAGRAPH   |     SECT      |     APPEND    |   DEL WORD    |
;|   ---------   |     ----      |     ------    |   --------    |
;|    Command    |   Fill Region | Replace regexp|   Undel Word  |
;+---------------+---------------+---------------+---------------+
;|    ADVANCE    |    BACKUP     |      CUT      |   DEL CHAR    |
;|    -------    |    ------     |      ---      |   --------    |
;|    Bottom     |     Top       |     Paste     |   Undel Char  |
;+---------------+---------------+---------------+---------------+
;|     WORD      |     EOL       |      CHAR     |               |
;|     ----      |     ---       |      ----     |               |
;|   Chgcase     |  Delete-EOL   |      Copy     |  ENTER WINDOW |
;+---------------+---------------+---------------+  ------------ |
;|          LINE  SCROLL         |     SELECT    |  Visit Shell  |
;|          ------------         |     ------    |               |
;|           Open Line           | ReDraw Window |               |
;+-------------------------------+---------------+---------------+
;
;
;                             K E Y B O A R D
;    +---------------+---------------+---------------+---------------+
;    |       ^       |       v       |     <----     |     ---->     |
;    |     -----     |     -----     |     -----     |     -----     |
;    |  Line-to-Top  |Line-to-Bottom |Begin Sentence | End Sentence  |
;    +---------------+---------------+---------------+---------------+
;    |       -       |       =       |       `       |  BACK SPACE   |
;    |     -----     |     -----     |     -----     |  ----------   |
;    | Goto Percent  |  Goto Line #  |  What Line #  | D Other Windw |
;    +---------------+---------------+---------------+---------------+
;                                                    |      DEL      |
;                                                    | ------------- |
;                                                    |  Del Window   |
;                                                    +---------------+
;                                                    |       \       |
;                                                    |     -----     |
;                                                    | Split Window  |
;                                                    +---------------+
;Ctrl-L         :Redraw Display.
;ESC-?          :Apropos Command.
;
;GOLD/ <space>  :UNDO
;GOLD/ <CR>     :Indent Newline with indentation level of previous line.
;
; Note that this is a VMS version, and the line
;(define-key GOLD-map "\eOM" 'subprocess-command)       ;;Subprocess    "ENTER"
;should be changed to
;(define-key GOLD-map "\eOM" 'shell)                    ;;Shell         "ENTER"
;for Un*x systems.
;DEC, VMS, VT-100, and EDT are trademarks of Digital Equipment Corporation.
;UNIX is a trademark of American Telephone & Telegraph.
