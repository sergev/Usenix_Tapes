;; keybinding for standard default sunterm keys
;; Copyright (C) 1987 Free Software Foundation, Inc.

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

;;  Jeff Peck, Sun Microsystems Inc  <peck@sun.com>

(defun ignore-key ()
  "interactive version of ignore"
  (interactive)
  (ignore))

(defun scroll-down-in-place (n)
  (interactive "p")
  (previous-line n)
  (scroll-down n))

(defun scroll-up-in-place (n)
  (interactive "p")
  (next-line n)
  (scroll-up n))

(defun kill-region-and-unmark (beg end)
  "Like kill-region, but pops the mark [which equals point, anyway.]"
  (interactive "r")
  (kill-region beg end)
  (setq this-command 'kill-region-and-unmark)
  (set-mark-command t))

(defun prev-complex-command ()
  "Select Previous-complex-command"
  (interactive)
  (if (zerop (minibuffer-depth))
      (repeat-complex-command 1)
    (previous-complex-command 1)))

(defun rerun-prev-command ()
  "Repeat Previous-complex-command."
  (interactive)
  (eval (nth 0 command-history)))

(defvar grep-arg nil "Default arg for RE-search")
(defun grep-arg ()
  (if (memq last-command '(research-forward research-backward)) grep-arg
    (let* ((command (car command-history))
	   (command-name (symbol-name (car command)))
	   (search-arg (car (cdr command)))
	   (search-command 
	    (and command-name (string-match "search" command-name)))
	   )
      (if (and search-command (stringp search-arg)) (setq grep-arg search-arg)
	(setq search-command this-command 
	      grep-arg (read-string "REsearch: " grep-arg)
	      this-command search-command)
	grep-arg))))

(defun research-forward ()
  "Repeat RE search forward."
  (interactive)
  (re-search-forward (grep-arg)))

(defun research-backward ()
  "Repeat RE search backward."
  (interactive)
  (re-search-backward (grep-arg)))

;;;
;;; handle sun's extra function keys
;;; this version for those who run with standard .ttyswrc and no emacstool
;;;
;;; sunview picks up expose and open on the way UP, 
;;; so we ignore them on the way down
;;;

(defvar sun-esc-bracket nil
  "*If non-nil, rebind ESC [ as prefix for Sun function keys.")

(defvar sun-raw-map (make-sparse-keymap) "*Keymap for ESC-[ encoded keyboard")

(define-key sun-raw-map "210z" 'backward-page)		; R3
(define-key sun-raw-map "213z" 'forward-page)		; R6
(define-key sun-raw-map "214z" 'beginning-of-buffer)	; R7
(define-key sun-raw-map "216z" 'scroll-down)		; R9
(define-key sun-raw-map "218z" 'recenter)               ; R11
(define-key sun-raw-map "220z" 'end-of-buffer)		; R13
(define-key sun-raw-map "222z" 'scroll-up)		; R15
(define-key sun-raw-map "193z" 'redraw-display)		; Again
(define-key sun-raw-map "194z" 'list-buffers)		; Props
(define-key sun-raw-map "195z" 'undo)			; Undo
(define-key sun-raw-map "196z" 'ignore-key)		; Expose-down
(define-key sun-raw-map "197z" 'sun-select-region)	; Put
(define-key sun-raw-map "198z" 'ignore-key)		; Open-down
(define-key sun-raw-map "199z" 'sun-yank-selection)	; Get
(define-key sun-raw-map "200z" 'exchange-point-and-mark); Find
(define-key sun-raw-map "201z" 'kill-region-and-unmark)	; Delete
(define-key sun-raw-map "226z" 'scroll-down-in-place)	; t3
(define-key sun-raw-map "227z" 'scroll-up-in-place)	; t4
(define-key sun-raw-map "229z" 'shrink-window)		; t6
(define-key sun-raw-map "230z" 'enlarge-window)		; t7

(if sun-esc-bracket
    (progn
      (define-key esc-map "[" sun-raw-map)		; Install sun-raw-map
      (define-key esc-map "[A" 'previous-line )		; R8
      (define-key esc-map "[B" 'next-line)		; R14
      (define-key esc-map "[C" 'forward-char)		; R12
      (define-key esc-map "[D" 'backward-char)		; R10
      (define-key esc-map "[[" 'backward-paragraph)	; the original esc-[
      ))

;;; Since .emacs gets loaded before this file, a hook is supplied
;;; for you to put your own bindings in.

(defvar sun-raw-map-hooks nil
  "List of forms to evaluate after setting sun-raw-map.")

(let ((hooks sun-raw-map-hooks))
  (while hooks
    (eval (car hooks))
    (setq hooks (cdr hooks))
    ))


;;; This section adds defintions for the emacstool users
;;; emacstool event filter converts function keys to C-x*{c}{lrt}
;;;
;;; for example the Open key (L7) would be encoded as "\C-x*gl"
;;; the control, meta, and shift keys modify the character {lrt}
;;; note that (unshifted) C-l is ",",  C-r is "2", and C-t is "4"
;;;
;;; {c} is [a-j] for LEFT, [a-i] for TOP, [a-o] for RIGHT.
;;; A higher level insists on encoding {h,j,l,n}{r} (the arrow keys)
;;; as ANSI escape sequences.  Use the shell command 
;;; % setkeys noarrows
;;; if you want these to come through for emacstool.
;;;
;;; If you are not using EmacsTool, 
;;; you can also use this by creating a .ttyswrc file to do the conversion.
;;; but it won't include the CONTROL, META, or SHIFT keys!
;;;
;;; Important to define SHIFTed sequence before matching unshifted sequence.
;;; (talk about bletcherous old uppercase terminal conventions!*$#@&%*&#$%)
;;;  this is worse than C-S/C-Q flow control anyday!
;;;  Do *YOU* run in capslock mode?
;;;

;;; Note:  al, el and gl are trapped by EmacsTool, so they never make it here.

(defvar meta-flag t)

(defvar suntool-map (make-sparse-keymap)
  "*Keymap for Emacstool bindings.")

(define-key suntool-map "gr" 'beginning-of-buffer)	; r7
(define-key suntool-map "iR" 'backward-page)		; R9
(define-key suntool-map "ir" 'scroll-down)		; r9
(define-key suntool-map "kr" 'recenter)			; r11
(define-key suntool-map "mr" 'end-of-buffer)		; r13
(define-key suntool-map "oR" 'forward-page)		; R15
(define-key suntool-map "or" 'scroll-up)		; r15
(define-key suntool-map "b\M-L" 'rerun-prev-command)	; M-AGAIN
(define-key suntool-map "b\M-l" 'prev-complex-command)	; M-Again
(define-key suntool-map "bl" 'redraw-display)		; Again
(define-key suntool-map "cl" 'list-buffers)		; Props
(define-key suntool-map "dl" 'undo)			; Undo
(define-key suntool-map "el" 'ignore-key)		; Expose-Open
(define-key suntool-map "fl" 'sun-select-region)	; Put
(define-key suntool-map "f," 'copy-region-as-kill)	; C-Put
(define-key suntool-map "gl" 'ignore-key)		; Open-Open
(define-key suntool-map "hl" 'sun-yank-selection)	; Get
(define-key suntool-map "h," 'yank)			; C-Get
(define-key suntool-map "il" 'research-forward)		; Find
(define-key suntool-map "i," 're-search-forward)	; C-Find
(define-key suntool-map "i\M-l" 'research-backward)	; M-Find
(define-key suntool-map "i\M-," 're-search-backward)	; C-M-Find

(define-key suntool-map "jL" 'yank)			; DELETE        
(define-key suntool-map "jl" 'kill-region-and-unmark)	; Delete
(define-key suntool-map "j\M-l" 'exchange-point-and-mark); M-Delete
(define-key suntool-map "j," 
  '(lambda () (interactive) (pop-mark 1)))		; C-Delete

(define-key suntool-map "fT" 'shrink-window-horizontally)	; T6
(define-key suntool-map "gT" 'enlarge-window-horizontally)	; T7
(define-key suntool-map "ft" 'shrink-window)			; t6
(define-key suntool-map "gt" 'enlarge-window)			; t7
(define-key suntool-map "cT" '(lambda(n) (interactive "p") (scroll-down n)))
(define-key suntool-map "dT" '(lambda(n) (interactive "p") (scroll-up n)))
(define-key suntool-map "ct" 'scroll-down-in-place)		; t3
(define-key suntool-map "dt" 'scroll-up-in-place)		; t4
(define-key ctl-x-map "*" suntool-map)

;;; Since .emacs gets loaded before this file, a hook is supplied
;;; for you to put your own bindings in.

(defvar suntool-map-hooks nil
  "List of forms to evaluate after setting suntool-map.")

(let ((hooks suntool-map-hooks))
  (while hooks
    (eval (car hooks))
    (setq hooks (cdr hooks))
    ))

;;;
;;; If running under emacstool, arrange to call suspend-emacstool
;;; instead of suspend-emacs.
;;;
;;; First mouse blip is a clue that we are in emacstool.
;;;
;;; C-x C-@ is the mouse command prefix.

(autoload 'sun-mouse-handler "sun-mouse" 
	  "Sun Emacstool handler for mouse blips (not loaded)." t)

(defun emacstool-init ()
  "Set up Emacstool window, if you know you are in an emacstool."
  ;; Make sure sun-mouse and sun-mouse-fns are loaded.
  (require 'sun-mouse-fns)
  (define-key ctl-x-map "\C-@" 'sun-mouse-handler)

  (if (< (sun-window-init) 0)
      (message "Not a Sun Window")
    (progn
      (substitute-key-definition 'suspend-emacs 'suspend-emacstool global-map)
      (substitute-key-definition 'suspend-emacs 'suspend-emacstool esc-map)
      (substitute-key-definition 'suspend-emacs 'suspend-emacstool ctl-x-map))
      (send-string-to-terminal
       (concat "\033]lEmacstool - GNU Emacs " emacs-version "\033\\"))
    ))

(defun sun-mouse-once ()
  "Converts to emacstool and sun-mouse-handler on first mouse hit."
  (interactive)
  (emacstool-init)
  (sun-mouse-handler)			; Now, execute this mouse blip.
  )
(define-key ctl-x-map "\C-@" 'sun-mouse-once)
