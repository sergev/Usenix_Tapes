
;; Run Prolog under Emacs (GNU Emacs 17.64)
;; Copyright (C) 1986 Masanobu UMEDA (umerin@flab.fujitsu.junet)

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

(require 'shell)

(make-variable-buffer-local 'shell-prompt-pattern)

(defvar prolog-mode-syntax-table nil "")
(defvar prolog-mode-abbrev-table nil "")

(defvar prolog-eof-string "\^D"
  "End of file string sent to inferior prolog process.")

(if (not prolog-mode-syntax-table)
    (let ((i 0))
      (setq prolog-mode-syntax-table (make-syntax-table))
      (set-syntax-table prolog-mode-syntax-table)
      (modify-syntax-entry ?_ "w")
      (modify-syntax-entry ?\\ "\\")
      (modify-syntax-entry ?/ ".")
      (modify-syntax-entry ?* ".")
      (modify-syntax-entry ?+ ".")
      (modify-syntax-entry ?- ".")
      (modify-syntax-entry ?= ".")
      (modify-syntax-entry ?% "<")
      (modify-syntax-entry ?< ".")
      (modify-syntax-entry ?> ".")
      (modify-syntax-entry ?\' "\""))
  )

(define-abbrev-table 'prolog-mode-abbrev-table ())

(defun prolog-mode-variables ()
  (set-syntax-table prolog-mode-syntax-table)
  (setq local-abbrev-table prolog-mode-abbrev-table)
  (make-local-variable 'paragraph-start)
  (setq paragraph-start (concat "^%%\\|^$\\|" page-delimiter)) ;'%%..'
  (make-local-variable 'paragraph-separate)
  (setq paragraph-separate paragraph-start)
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'prolog-indent-line)
  (make-local-variable 'comment-start)
  (setq comment-start "%")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "%+ *")
  (make-local-variable 'comment-column)
  (setq comment-column 40)
  (make-local-variable 'comment-indent-hook)
  (setq comment-indent-hook 'prolog-comment-indent))

(defun prolog-mode-commands (map)
  (define-key map "\t" 'prolog-indent-line))

(defvar prolog-mode-map (make-sparse-keymap))
(define-key prolog-mode-map "\e\C-x" 'prolog-consult-region)
(prolog-mode-commands prolog-mode-map)

(defun prolog-mode ()
  "Major mode for editing Prolog code for Prologs.
Commands:
Blank lines and '%%...' separate paragraphs.  '%'s start comments.
\\{prolog-mode-map}
Entry to this mode calls the value of prolog-mode-hook
if that value is non-nil."
  (interactive)
  (kill-all-local-variables)
  (use-local-map prolog-mode-map)
  (setq major-mode 'prolog-mode)
  (setq mode-name "Prolog")
  (prolog-mode-variables)
  (run-hooks 'prolog-mode-hook))

(defun prolog-indent-line (&optional whole-exp)
  "Indent current line as Prolog code.
With argument, indent any additional lines of the same clause
rigidly along with this one (not yet)."
  (interactive "p")
  (let ((indent (prolog-indent-level))
	(pos (- (point-max) (point))) beg)
    (beginning-of-line)
    (setq beg (point))
    (skip-chars-forward " \t")
    (if (zerop (- indent (current-column)))
	nil
      (delete-region beg (point))
      (indent-to indent))
    (if (> (- (point-max) pos) (point))
	(goto-char (- (point-max) pos)))
    ))

(defun prolog-indent-level ()
  "Compute prolog indentation level."
  (save-excursion
    (beginning-of-line)
    (skip-chars-forward " \t")
    (cond
     ((looking-at "%%") 0)		;Large comment starts
     ((looking-at "%") comment-column)	;Small comment starts
     ((bobp) 0)				;Beginning of buffer
     (t
      (let ((empty t) ind more less)
	(if (looking-at ")")
	    (setq less t)		;Find close
	  (setq less nil))
	;; See previous indentation
	(while empty
	  (forward-line -1)
	  (beginning-of-line)
	  (if (bobp) (setq empty nil))
	  (skip-chars-forward " \t")
	  (if (not (or (looking-at "%") (looking-at "\n")))
	      (setq empty nil)))
	(setq ind (current-column))	;Beginning of clause
	;; See its beginning
	(if (looking-at "(")
	    (setq more t)		;Find open
	  (setq more nil))
	(end-of-prolog-clause)
	(or (bobp) (forward-char -1))
	;; See its tail
	(if (looking-at "[,(;>]")
	    (if (and more
		     (looking-at "[^,]"))
		(+ ind tab-width)	;More indentation
	      (max tab-width ind))	;Same indentation
	  (if (looking-at "-")
	      tab-width			;TAB
	    (if (or less
		    (looking-at "[^.]"))
		(max (- ind tab-width) 0) ;Less indentation
	      0)			;No indentation
	    )
	  )
	))
     )))

(defun end-of-prolog-clause ()
  "Go to end of clause in this line."
  (beginning-of-line 1)
  (if (null comment-start)
      (error "No comment syntax defined")
    (let* ((eolpos (save-excursion (end-of-line) (point))))
      (if (re-search-forward comment-start-skip eolpos 'move)
	  (goto-char (match-beginning 0)))
      (skip-chars-backward " \t")
      )
    )
  )

(defun prolog-comment-indent ()
  "Compute prolog comment indentation."
  (if (looking-at "%%")
      0
    (save-excursion
      (skip-chars-backward " \t")
      (max (1+ (current-column))	;Insert one space at least
	   comment-column))
    )
  )


;;;
;;; Inferior prolog mode
;;;
(defvar inferior-prolog-mode-map nil)
(if inferior-prolog-mode-map
    nil
  (setq inferior-prolog-mode-map (copy-alist shell-mode-map))
  (prolog-mode-commands  inferior-prolog-mode-map)
  (define-key inferior-prolog-mode-map "\e\C-x" 'prolog-consult-region))

(defun inferior-prolog-mode ()
  "Major mode for interacting with an inferior Prolog process.

The following commands are available:
\\{inferior-prolog-mode-map}

Entry to this mode calls the value of prolog-mode-hook with no arguments,
if that value is non-nil.  Likewise with the value of shell-mode-hook.
prolog-mode-hook is called after shell-mode-hook.

You can send text to the inferior Prolog from other buffers
using the commands send-region, send-string and \\[prolog-consult-region].

Commands:
Tab indents for Prolog; with argument, shifts rest
 of expression rigidly with the current line.
Paragraphs are separated only by blank lines and '%%'. '%'s start comments.

Return at end of buffer sends line as input.
Return not at end copies rest of line to end and sends it.
\\[shell-send-eof] sends end-of-file as input.
\\[kill-shell-input] and \\[backward-kill-word] are kill commands, imitating normal Unix input editing.
\\[interrupt-shell-subjob] interrupts the shell or its current subjob if any.
\\[stop-shell-subjob] stops, likewise. \\[quit-shell-subjob] sends quit signal, likewise."
  (interactive)
  (kill-all-local-variables)
  (setq major-mode 'inferior-prolog-mode)
  (setq mode-name "Inferior Prolog")
  (setq mode-line-format 
	"--%1*%1*-Emacs: %17b   %M   %[(%m: %s)%]----%3p--%-")
  (prolog-mode-variables)
  (use-local-map inferior-prolog-mode-map)
  (make-local-variable 'last-input-start)
  (setq last-input-start (make-marker))
  (make-local-variable 'last-input-end)
  (setq last-input-end (make-marker))
  (setq shell-prompt-pattern "^| [ ?][- ] *") ;Set prolog prompt pattern
  (run-hooks 'shell-mode-hook 'prolog-mode-hook))

(defun run-prolog ()
  "Run an inferior Prolog process, input and output via buffer *prolog*."
  (interactive)
  (switch-to-buffer (make-shell "prolog" "prolog"))
  (inferior-prolog-mode))

(defun prolog-consult-region ()
  "Send the region to the Prolog process made by M-x run-prolog."
  (interactive)
  (save-excursion
    (send-string "prolog" "[-user].\n")	;Reconsult mode
    (send-region "prolog" (mark) (point))
    (send-string "prolog" prolog-eof-string) ;Send eof to prolog process.
    )
  )

(defun prolog-consult-region-and-go ()
  "Send the region to the inferior Prolog, and switch to *prolog* buffer."
  (interactive)
  (prolog-consult-region)
  (switch-to-buffer "*prolog*"))
