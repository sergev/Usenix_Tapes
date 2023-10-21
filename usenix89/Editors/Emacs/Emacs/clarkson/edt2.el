
;; file: edt.el
;; EDT(tm) Emulation mode for GNUEMACS.
;; Not Copyrighted (C) at all.
     
;; This file is not yet a part of GNU Emacs, though it would be an honour.
     
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
     
(defvar SS3-map nil
      "SS3-map maps the SS3 function keys on the VT200 keyboard.
        The SS3 keys are the numeric keypad keys in keypad application mode
        (DECKPAM).  SS3 is the ASCII-8bit character for the 7-bit escape
        sequence <ESC>O.  ")
(setq SS3-map (make-keymap))
(define-key global-map "\eO" SS3-map)
     
(defvar GOLD-map nil
      "GOLD-map maps the function keys on the VT100 keyboard preceeded
        by the PF1 key.  GOLD is the ASCII the 7-bit escape sequence <ESC>OP.")
     
(setq GOLD-map (make-keymap))
    (define-key global-map "\eOP" GOLD-map)
     
;;; GNU Emacs code converted from Mocklisp
(defun delete-region-to-buffer (bufname)
    (copy-to-buffer bufname (point) (mark))
    (delete-region (point) (mark))
    )
     
(defun  delete-current-line (num) "
        This deletes the current line from the cursor on,
        including the end of line.  The text is put in the LineBuffer
        and can be retrieved by the GOLD/Und L key.
        the GOLD/Und L key.
        Accepts a prefix argument for the number of lines to delete."
       (interactive "p")
       (while (> num 0)
              (if
                     (eolp)
                     (progn
                            (set-mark-command nil)
                            (if (not (eobp)) (forward-char 1)
                                             (insert-string "\n"))
                            )
                     (progn
                            (set-mark-command nil)
                            (end-of-line)
                            (if (not (eobp)) (forward-char 1)
                                             (insert-string "\n"))
                            )
                     )
              (delete-region-to-buffer "LineBuffer")
              (setq num (1- num))
              )
       )
(defun delete-to-eol (num) "
        This deletes the current line from the cursor on,
        up to the end of line.  The text is put in the LineBuffer
        and can be retrieved by the GOLD/Und L key.
        Accepts a prefix argument for the number of lines to delete."
       (interactive "p")
       (while (> num 0)
              (if
                     (eolp)
                     (progn
                            (set-mark-command nil)
                            (next-line 1)
                            (end-of-line)
                            (exchange-point-and-mark)
                            )
                     (progn
                            (set-mark-command nil)
                            (end-of-line)
                            (exchange-point-and-mark)
                            )
                     )
              (delete-region-to-buffer "LineBuffer")
              (setq num (1- num))
              )
       )
(defun delete-current-word (num) "
        This deletes the current word.
        The words are parked in the WordBuffer.
        Accepts a prefix argument for the number of words to delete."
       (interactive "p")
       (while (> num 0)
              (set-mark-command nil)
              (forward-word 1)
              (exchange-point-and-mark)
              (delete-region-to-buffer "WordBuffer")
              (setq num (1- num))
              )
       )
(defun  delete-previous-word (num) "
        This deletes the previous word.
        The words are parked in the WordBuffer.
        Accepts a prefix argument for the number of words to delete."
       (interactive "p")
       (while (> num 0)
              (set-mark-command nil)
              (backward-word 1)
              (exchange-point-and-mark)
              (delete-region-to-buffer "WordBuffer")
              (setq num (1- num))
              )
       )
(defun  delete-current-character (num) "
        This deletes the current character.
        The character is parked in the CharBuffer.
        Accepts a prefix argument for the number of characters to delete."
       (interactive "p")
       (while (> num 0)
              (copy-to-buffer "CharBuffer" (point) (1+ (point)))
              (delete-region (point) (1+ (point)))
              (setq num (1- num))
              )
       )
(defun  delete-previous-character (num) "
        This deletes the previous character.
        The character is parked in the CharBuffer.
        Accepts a prefix argument for the number of characters to delete."
       (interactive "p")
       (while (> num 0)
              (copy-to-buffer "CharBuffer" (point) (1- (point)))
              (delete-region (point) (1- (point)))
              (setq num (1- num))
              )
       )
(defun  undelete-line () "
        Undelete Lines from the LineBuffer. "
       (interactive)
       (insert-buffer-substring "LineBuffer")
       )
(defun undelete-word () "
        Undeletes Words from WordBuffer. "
       (interactive)
       (insert-buffer-substring "WordBuffer")
       )
(defun undelete-char () "
        Undelete Characters from CharBuffer. "
       (interactive)
       (insert-buffer-substring "CharBuffer")
       )
     
(defun  repeat-end-of-line (num) "
        Move EOL downward if key is pressed repeatedly.
        Accepts a prefix argument for the number of lines to move."
       (interactive "p")
       (while (> num 0)
              (if (eolp)
                     (progn
                            (forward-char)
                            (end-of-line))
                     (end-of-line)
                     )
              (setq num (1- num))
              )
       )
(defun previous-end-of-line (num) "
       Move EOL upward.
       Accepts a prefix argument for the number of lines to move."
       (interactive "p")
       (while (> num 0)
              (beginning-of-line)
              (backward-char)
              (setq num (1- num))
              )
       )
     
; not used - append-to-buffer used instead
(defun append-to-killring () "
       Append mark-point region to the last entry on the kill ring."
  (interactive)
  (append-next-kill)
  (kill-region (point) (mark))
  (message "%s" "Appended.")
  )
     
(defun copy-to-killring () "
        GOLD/COPY. Copy point to mark region to Kill Ring."
       (interactive)
       (copy-region-as-kill (point) (mark))
       (message "%s" "Copied to the Kill Ring.")
       )
     
(defun next-word (num) "
        WORD in the Advance mode.
        Accepts a prefix argument for the number of words to move."
       (interactive "p")
       (forward-word (1+ num))
       (forward-word -1)
       )
     
(defun previous-word (num) "
        WORD in the Backup mode.
        Accepts a prefix argument for the number of words to move."
       (interactive "p")
       (forward-word (- (1+ num)))
       (forward-word 1)
       )
     
(defun move-cursor-down (num) "
        Moves the cursor down one line and scrolls the display if necessary.
        Accepts a prefix argument for the number of lines to move."
       (interactive "p")
       (beginning-of-line)
       (next-line num)
       )
(defun move-cursor-up (num) "
        Moves the cursor up one line and scrolls the display if necessary.
        Accepts a prefix argument for the number of lines to move."
       (interactive "p")
       (beginning-of-line)
       (previous-line num)
       )
     
(defun scroll-window-down (num) "
        Scrolls the display down a window-full.
        Accepts a prefix argument for the number of window-fulls to scroll."
       (interactive "p")
       (scroll-down (- (* (window-height) num) 2))
       )
(defun scroll-window-up (num) "
        Scrolls the display up a window-full.
        Accepts a prefix argument for the number of window-fulls to scroll."
       (interactive "p")
       (scroll-up (- (* (window-height) num) 2))
       )
     
(defun next-paragraph (num) "
        Puts the cursor at the beginning of the next indented paragraph.
        Accepts a prefix argument for the number of parapgraphs."
       (interactive "p")
       (while (> num 0)
              (next-line 1)
              (forward-paragraph)
              (previous-line 1)
              (if (eolp) (next-line 1))
              (setq num (1- num))
              )
       )
(defun previous-paragraph (num) "
        Puts the cursor at the beginning of previous indented paragraph.
        Accepts a prefix argument for the number of parapgraphs."
       (interactive "p")
       (while (> num 0)
              (backward-paragraph)
              (previous-line 1)
              (if (eolp) (next-line 1))
              (setq num (1- num))
              )
       )
     
(defun beginning-of-file () "
        Move cursor to the beginning of file, but dont set the mark."
        (interactive)
        (goto-char (point-min))
        )
(defun end-of-file () "
        Move cursor to the end of file, but dont set the mark."
        (interactive)
        (goto-char (point-max))
        )
     
(defun goto-percent (perc) "
        Move point to this percentage of the buffer"
        (interactive "nGoto-percentage: ")
        (if (or (> perc 100) (< perc 0))
            (error "Percentage out of range 0 < percent < 100")
            (goto-char (/ (* (point-max) perc) 100))
            )
        )
     
(defun advance-direction () "
        Set EDT Advance mode for cursor keys."
       (interactive)
       (setq global-mode-string "ADVANCE")
       (define-key SS3-map "R" 'isearch-forward)                ;; PF3
       (define-key SS3-map "x" 'scroll-window-up)               ;; "8"
       (define-key SS3-map "w" 'next-paragraph)                 ;; "7"
       (define-key SS3-map "q" 'next-word)                      ;; "1"
       (define-key SS3-map "r" 'repeat-end-of-line)             ;; "2"
       (define-key SS3-map "s" 'forward-char)                   ;; "3"
       (define-key SS3-map "p" 'move-cursor-down)               ;; "0"
       )
(defun backup-direction () "
        Set EDT Backup mode for cursor keys."
       (interactive)
       (setq global-mode-string "BACKUP ")
       (define-key SS3-map "R" 'isearch-backward)               ;; PF3
       (define-key SS3-map "x" 'scroll-window-down)             ;; "8"
       (define-key SS3-map "w" 'previous-paragraph)             ;; "7"
       (define-key SS3-map "q" 'previous-word)                  ;; "1"
       (define-key SS3-map "r" 'previous-end-of-line)           ;; "2"
       (define-key SS3-map "s" 'backward-char)                  ;; "3"
       (define-key SS3-map "p" 'move-cursor-up)                 ;; "0"
       )
     
(defun redraw-crt-set-keypad () "
        Ctrl-L function. Redraw display and set keypad application mode."
       (interactive)
       (send-string-to-terminal "\e=")
       (redraw-display))
     
(defun beginning-of-window () "
        Home cursor to top of window."
        (interactive)
        (move-to-window-line 0)
)
     
(defun line-to-bottom-of-window () "
        Move the current line to the top of the window."
       (interactive)
       (recenter -1)
       )
(defun line-to-top-of-window () "
        Move the current line to the top of the window."
       (interactive)
       (recenter 0)
       )
     
(defun  case-flip-character (num) "
        Change the case of the character under the cursor.
        Accepts a prefix argument of the number of characters to invert."
       (interactive "p")
       (let (c)
              (while (> num 0)
                     (setq c (following-char))
                     (delete-char 1)
                     (insert
                            (cond  ((and (<= 97 c) (<= c 122))  (- c 32))
                                   ((and (<= 65 c) (<= c  90))  (+ c 32))
                                   (c))
                            )
                     (setq num (1- num))
                     )
              )
       )
     
(defun  fill-region-wisely () "
        Fill region in a manner consistent with the major-mode.
        Uses indent-region for emacs-lisp, lisp, and C,
        and fill-region for other modes."
       (interactive)
       (cond  ((eq major-mode 'emacs-lisp-mode)
                     (indent-region (point) (mark) nil))
              ((eq major-mode 'lisp-mode)
                     (indent-region (point) (mark) nil))
              ((eq major-mode 'c-mode)
                     (indent-region (point) (mark) nil))
              ((eq major-mode 'TeX-mode)
                     ;; fill-region in TeX-mode can be *very* dangerous.
                     (if (yes-or-no-p
                              "Do you really want to fill-region in TeX-mode?")
                            (fill-region (point) (mark))))
              (t        (fill-region (point) (mark))
                     )
              )
       )
(defun  mark-section-wisely () "
        Mark the section in a manner consistent with the major-mode.
        Uses mark-defun for emacs-lisp, lisp,
        mark-c-function for C,
        and mark-paragraph for other modes."
       (interactive)
       (cond  ((eq major-mode 'emacs-lisp-mode)
                     (mark-defun))
              ((eq major-mode 'lisp-mode)
                     (mark-defun))
              ((eq major-mode 'c-mode)
                     (mark-c-function))
              (t (mark-paragraph))
              )
       )
     
;;; Key Bindings
(global-set-key "\C-l" 'redraw-crt-set-keypad)          ;;"ctrl-L"
(global-set-key "\034" 'quoted-insert)                  ;; "C-\"
(global-set-key "\177" 'delete-previous-character)      ;;"Delete"
(define-key emacs-lisp-mode-map "\177" 'delete-previous-character) ;;"Delete"
(define-key lisp-mode-map "\177" 'delete-previous-character) ;;"Delete"
(global-set-key "\C-j" 'delete-previous-word)           ;;"LineFeed"
(define-key esc-map "?" 'apropos)                       ;;"<ESC>?"
     
;;Bind keyboard arrow keys
(define-key SS3-map "A" 'previous-line)                 ;; up arrow
(define-key SS3-map "B" 'next-line)                     ;; down-arrow
(define-key SS3-map "C" 'forward-char)                  ;; right-arrow
(define-key SS3-map "D" 'backward-char)                 ;; left-arrow
; some vt100 lookalikes have a home-key.
(define-key SS3-map "H" 'beginning-of-window)           ;;"home-arrow"
     
;;Bind keypad keys
(define-key SS3-map "\C-g" 'keyboard-quit)              ;; just for safety
(define-key SS3-map "Q" 'describe-key)                  ;;       "PF2"
(define-key SS3-map "S" 'delete-current-line)           ;;DEL L  "PF4"
(define-key SS3-map "y" 'append-to-buffer)              ;;APPEND "9"
(define-key SS3-map "m" 'delete-current-word)           ;;DEL W  "-"
(define-key SS3-map "t" 'advance-direction)             ;;ADV    "4"
(define-key SS3-map "u" 'backup-direction)              ;;BAK    "5"
(define-key SS3-map "v" 'kill-region)                   ;;CUT    "6"
(define-key SS3-map "l" 'delete-current-character)      ;;DEL C  ","
(define-key SS3-map "n" 'set-mark-command)              ;;SELECT "."
(define-key SS3-map "M" 'other-window)                  ;; Enter
     
;;Bind GOLD/Keyboard keys
(define-key GOLD-map "\C-g"  'keyboard-quit)            ;; just for safety
(define-key GOLD-map "\e[A" 'line-to-top-of-window)     ;;"up-arrow"
(define-key GOLD-map "\e[B" 'line-to-bottom-of-window)  ;;"down-arrow"
(define-key GOLD-map "\e[D" 'backward-sentence)         ;;"left-arrow"
(define-key GOLD-map "\e[C" 'forward-sentence)          ;;"right-arrow"
     
(define-key GOLD-map "\177" 'delete-window)             ;;"Delete"
(define-key GOLD-map "\010" 'delete-other-windows)      ;;"BackSpace"
(define-key GOLD-map "\015" 'newline-and-indent)        ;;"Return"
(define-key GOLD-map "\040" 'undo)                      ;;"Spacebar"
     
(define-key GOLD-map "%" 'goto-percent)                 ;; "%"
(define-key GOLD-map "=" 'goto-line)                    ;; "="
(define-key GOLD-map "`" 'what-line)                    ;; "`"
(define-key GOLD-map "\034" 'split-window-vertically)   ;; "\\"
     
; GOLD letter combinations:
(define-key GOLD-map "b" 'buffer-menu)                  ;; "b"
(define-key GOLD-map "B" 'buffer-menu)                  ;; "B"
(define-key GOLD-map "d" 'delete-window)                ;; "d"
(define-key GOLD-map "D" 'delete-window)                ;; "D"
(define-key GOLD-map "e" 'compile)                      ;; "e"
(define-key GOLD-map "E" 'compile)                      ;; "E"
(define-key GOLD-map "i" 'insert-file)                  ;; "i"
(define-key GOLD-map "I" 'insert-file)                  ;; "I"
(define-key GOLD-map "l" 'goto-line)                    ;; "l"
(define-key GOLD-map "L" 'goto-line)                    ;; "L"
;;(define-key GOLD-map "write-modified-files" "\eOPm")          ;; "m"
;;(define-key GOLD-map "write-modified-files" "\eOPM")          ;; "m"
(define-key GOLD-map "n" 'next-error)                           ;; "n"
(define-key GOLD-map "N" 'next-error)                           ;; "N"
(define-key GOLD-map "o" 'switch-to-buffer-other-window)        ;; "o"
(define-key GOLD-map "O" 'switch-to-buffer-other-window)        ;; "O"
(define-key GOLD-map "r" 'revert-file)                          ;; "r"
(define-key GOLD-map "r" 'revert-file)                          ;; "R"
(define-key GOLD-map "s" 'save-buffer)                          ;; "s"
(define-key GOLD-map "S" 'save-buffer)                          ;; "S"
(define-key GOLD-map "v" 'find-file-other-window)               ;; "v"
(define-key GOLD-map "V" 'find-file-other-window)               ;; "V"
(define-key GOLD-map "w" 'write-file)                           ;; "w"
(define-key GOLD-map "w" 'write-file)                           ;; "W"
;;(define-key GOLD-map "shrink-window" "\eOPz")                 ;; "z"
;;(define-key GOLD-map "shrink-window" "\eOPZ")                 ;; "z"
     
;;Bind GOLD/Keypad keys
(define-key GOLD-map "\eOP" 'mark-section-wisely)       ;;Gold     "PF1"
(define-key GOLD-map "\eOQ" 'describe-function)         ;;Help     "PF2"
(define-key GOLD-map "\eOR" 'occur-menu)                ;;Find     "PF3"
(define-key GOLD-map "\eOS" 'undelete-line)             ;;Und Line "PF4"
(define-key GOLD-map "\eOw" 'execute-extended-command)  ;;Command  "7"
(define-key GOLD-map "\eOx" 'fill-region-wisely)        ;;Fill     "8"
(define-key GOLD-map "\eOy" 'replace-regexp)            ;;Replace   "9"
(define-key GOLD-map "\eOm" 'undelete-word)             ;;UND word  "-"
(define-key GOLD-map "\eOt" 'end-of-file)               ;;Bottom    "4"
(define-key GOLD-map "\eOu" 'beginning-of-file)         ;;Top       "5"
(define-key GOLD-map "\eOv" 'yank)                      ;;Paste         "6"
(define-key GOLD-map "\eOl" 'undelete-char)             ;;UND Char      ","
(define-key GOLD-map "\eOq" 'case-flip-character)       ;;Chgcase       "1"
(define-key GOLD-map "\eOr" 'delete-to-eol)             ;;Del EOL       "2"
(define-key GOLD-map "\eOs" 'copy-to-killring)          ;;Copy          "3"
(define-key GOLD-map "\eOp" 'open-line)                 ;;Open L        "0"
(define-key GOLD-map "\eOn" 'redraw-crt-set-keypad)     ;;Reset Window  "."
(define-key GOLD-map "\eOM" 'subprocess-command)        ;;Subprocess    "ENTER"
     
;;Set VT-Series keypad application mode, initialize direction, and
;;prevent the two temporary buffers from being checkpointed.
(send-string-to-terminal "\e=")         ;; Turn on keypad
(advance-direction)
