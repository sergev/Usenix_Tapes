;; TeX mode commands.
;; Copyright (C) 1985, 1986 Free Software Foundation, Inc.
;; Rewritten following contributions by William F. Schelter
;; and Dick King (king@kestrel).
;; Modified August 1986 by Stephen Gildea <mit-erl!gildea> and
;; Michael Prange <mit-erl!prange> to add LaTeX support and enhance
;; TeX-region.
;; Added TeX-directory and reorganized somewhat  gildea 21 Nov 86

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

;; Still to do:
;;  Make TAB indent correctly for TeX code.  Then we can make linefeed
;;  do something more useful.
;;
;;  Have spell understand TeX instead of assuming the entire world
;;  uses nroff.
;;
;;  The code for finding matching $ needs to be fixed.

(defvar TeX-directory "/tmp/"
  "*Directory in which to run TeX subjob.  Temporary files are
created in this directory.")
(defvar TeX-dvi-print-command "lpr -d"
  "*Command string used by \\[TeX-print] to print a .dvi file.")
(defvar TeX-show-queue-command "lpq"
  "*Command string used by \\[TeX-show-print-queue] to show the print queue
that \\[TeX-print] put your job on.")
(defvar TeX-default-mode 'plain-TeX-mode
  "*Mode to enter for a new file when it can't be determined whether
the file is plain TeX or LaTeX or what.")

(defvar TeX-command nil
  "The command to run TeX on a file.  The name of the file will be appended
to this string, separated by a space.")
(defvar TeX-trailer nil
  "String appended after the end of a region sent to TeX by \\[TeX-region].")
(defvar TeX-start-of-header nil
  "String used by \\[TeX-region] to delimit the start of the file's header.")
(defvar TeX-end-of-header nil
  "String used by \\[TeX-region] to delimit the end of the file's header.")
(defvar TeX-shell-cd-command "cd"
  "Command to give to shell running TeX to change directory.  The value of
TeX-directory will be appended to this, separated by a space.")
(defvar TeX-zap-file nil
  "Temporary file name used for text being sent as input to TeX.
Should be a simple file name with no extension or directory specification.")

(defvar TeX-mode-syntax-table nil
  "Syntax table used while in TeX mode.")

(defvar TeX-mode-map nil)

(defun TeX-define-common-keys (keymap)
  "Define the keys that we want defined both in TeX-mode
and in the TeX-shell."
  (define-key keymap "\C-c\C-k" 'TeX-kill-job)
  (define-key keymap "\C-c\C-l" 'TeX-recenter-output-buffer)
  (define-key keymap "\C-c\C-q" 'TeX-show-print-queue)
  (define-key keymap "\C-c\C-p" 'TeX-print)
  )

(if TeX-mode-map 
    nil
  (setq TeX-mode-map (make-sparse-keymap))
  (TeX-define-common-keys TeX-mode-map)
  (define-key TeX-mode-map "\"" 'TeX-insert-quote)
  (define-key TeX-mode-map "\n" 'TeX-terminate-paragraph)
  (define-key TeX-mode-map "\e}" 'up-list)
  (define-key TeX-mode-map "\e{" 'TeX-insert-braces)
  (define-key TeX-mode-map "\C-c\C-r" 'TeX-region)
  (define-key TeX-mode-map "\C-c\C-b" 'TeX-buffer)
  (define-key TeX-mode-map "\C-c\C-f" 'TeX-close-LaTeX-block)
  )

;(fset 'TeX-mode 'tex-mode) in loaddefs.
(defun tex-mode ()
  "Major mode for editing files of input for TeX or LaTeX.
Trys to intuit whether this file is for plain TeX or LaTeX and
calls plain-tex-mode or latex-mode.  If it cannot be determined
\(e.g., the file is empty), the value of TeX-default-mode is used."
  (interactive)
  (let ((mode
	 (save-excursion
	   (goto-char (point-min))
	   (while (and (search-forward "\\" nil t)
		       (let ((search-end (point)))
			 (save-excursion (beginning-of-line)
					 (search-forward "%" search-end t)))))
	   (if (not (eobp))
	       (if (looking-at "documentstyle")
		   'latex-mode
		 'plain-tex-mode)))))
    (if mode (funcall mode)
      (funcall TeX-default-mode))))

(fset 'plain-TeX-mode 'plain-tex-mode)
(fset 'LaTeX-mode 'latex-mode)

(defun plain-tex-mode ()
  "Major mode for editing files of input for plain TeX.
Makes $ and } display the characters they match.
Makes \" insert `` when it seems to be the beginning of a quotation,
and '' when it appears to be the end; it inserts \" only after a \\.

Use \\[TeX-region] to run TeX on the current region, plus a \"header\"
copied from the top of the file (containing macro definitions, etc.),
running TeX under a special subshell.  \\[TeX-buffer] does the whole buffer.
\\[TeX-print] prints the .dvi file made by either of these.

Use \\[validate-TeX-buffer] to check buffer for paragraphs containing
mismatched $'s or braces.

Special commands:
\\{TeX-mode-map}

Mode variables:
TeX-directory
	Directory in which to create temporary files for TeX jobs
	run by \\[TeX-region] or \\[TeX-buffer].
TeX-dvi-print-command
	Command string used by \\[TeX-print] to print a .dvi file.
TeX-show-queue-command
	Command string used by \\[TeX-show-print-queue] to show the print
	queue that \\[TeX-print] put your job on.

Entering plain-TeX mode calls the value of text-mode-hook,
then the value of TeX-mode-hook, and then the value
of plain-TeX-mode-hook."
  (interactive)
  (TeX-common-initialization)
  (setq mode-name "TeX")
  (setq major-mode 'plain-TeX-mode)
  (make-local-variable 'TeX-command)
  (setq TeX-command "tex")
  (make-local-variable 'TeX-start-of-header)
  (setq TeX-start-of-header "%**start of header")
  (make-local-variable 'TeX-end-of-header)
  (setq TeX-end-of-header "%**end of header")
  (make-local-variable 'TeX-trailer)
  (setq TeX-trailer "\\bye\n")
  (run-hooks 'text-mode-hook 'TeX-mode-hook 'plain-TeX-mode-hook))

(defun latex-mode ()
  "Major mode for editing files of input for LaTeX.
Makes $ and } display the characters they match.
Makes \" insert `` when it seems to be the beginning of a quotation,
and '' when it appears to be the end; it inserts \" only after a \\.

Use \\[TeX-region] to run LaTeX on the current region, plus the preamble
copied from the top of the file (containing \\documentstyle, etc.),
running LaTeX under a special subshell.  \\[TeX-buffer] does the whole buffer.
\\[TeX-print] prints the .dvi file made by either of these.

Use \\[validate-TeX-buffer] to check buffer for paragraphs containing
mismatched $'s or braces.

Special commands:
\\{TeX-mode-map}

Mode variables:
TeX-directory
	Directory in which to create temporary files for TeX jobs
	run by \\[TeX-region] or \\[TeX-buffer].
TeX-dvi-print-command
	Command string used by \\[TeX-print] to print a .dvi file.
TeX-show-queue-command
	Command string used by \\[TeX-show-print-queue] to show the print
	queue that \\[TeX-print] put your job on.

Entering LaTeX mode calls the value of text-mode-hook,
then the value of TeX-mode-hook, and then the value
of LaTeX-mode-hook."
  (interactive)
  (TeX-common-initialization)
  (setq mode-name "LaTeX")
  (setq major-mode 'LaTeX-mode)
  (make-local-variable 'TeX-command)
  (setq TeX-command "latex")
  (make-local-variable 'TeX-start-of-header)
  (setq TeX-start-of-header "\\documentstyle")
  (make-local-variable 'TeX-end-of-header)
  (setq TeX-end-of-header "\\begin{document}")
  (make-local-variable 'TeX-trailer)
  (setq TeX-trailer "\\end{document}\n")
  (run-hooks 'text-mode-hook 'TeX-mode-hook 'LaTeX-mode-hook))

(defun TeX-common-initialization ()
  (kill-all-local-variables)
  (use-local-map TeX-mode-map)
  (setq local-abbrev-table text-mode-abbrev-table)
  (if (null TeX-mode-syntax-table)
      (progn
	(setq TeX-mode-syntax-table (make-syntax-table))
	(set-syntax-table TeX-mode-syntax-table)
	(modify-syntax-entry ?\\ ".")
	(modify-syntax-entry ?\f ">")
	(modify-syntax-entry ?\n ">")
	(modify-syntax-entry ?$ "$$")
	(modify-syntax-entry ?% "<")
	(modify-syntax-entry ?\" ".")
	(modify-syntax-entry ?& ".")
	(modify-syntax-entry ?_ ".")
	(modify-syntax-entry ?@ "_")
	(modify-syntax-entry ?~ " ")
	(modify-syntax-entry ?' "w"))
    (set-syntax-table TeX-mode-syntax-table))
  (make-local-variable 'paragraph-start)
  (setq paragraph-start "^[ \t]*$\\|^[\f\\\\]")
  (make-local-variable 'paragraph-separate)
  (setq paragraph-separate paragraph-start)
  (make-local-variable 'comment-start)
  (setq comment-start "%")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "[^\\]\\(\\\\\\\\\\)*%+ *")
  (make-local-variable 'comment-indent-hook)
  (setq comment-indent-hook 'TeX-comment-indent))

(defun TeX-comment-indent ()
  (if (looking-at "%%%")
      (current-column)
    (skip-chars-backward " \t")
    (max (if (bolp) 0 (1+ (current-column)))
	 comment-column)))

(defun TeX-insert-quote (arg)
  "Insert ``, '' or \" according to preceding character.
With prefix argument, always insert \" characters."
  (interactive "P")
  (if arg
      (let ((count (prefix-numeric-value arg)))
	(if (listp arg)
	    (self-insert-command 1)	;C-u always inserts just one
	  (self-insert-command count)))
    (insert
     (cond
      ((or (bobp)
	   (save-excursion
	     (forward-char -1)
	     (looking-at "[ \t\n]\\|\\s(")))
       "``")
      ((= (preceding-char) ?\\)
       ?\")
      (t "''")))))

(defun validate-TeX-buffer ()
  "Check current buffer for paragraphs containing mismatched $'s.
As each such paragraph is found, a mark is pushed at its beginning,
and the location is displayed for a few seconds."
  (interactive)
  (let ((opoint (point)))
    (goto-char (point-max))
    ;; Does not use save-excursion
    ;; because we do not want to save the mark.
    (unwind-protect
	(while (and (not (input-pending-p)) (not (bobp)))
	  (let ((end (point)))
	    (search-backward "\n\n" nil 'move)
	    (or (TeX-validate-paragraph (point) end)
		(progn
		  (push-mark (point))
		  (message "Mismatch found in pararaph starting here")
		  (sit-for 4)))))
      (goto-char opoint))))

(defun TeX-validate-paragraph (start end)
  (condition-case ()
      (save-excursion
	(save-restriction
	  (narrow-to-region start end)
	  (goto-char start)
	  (forward-sexp (- end start))
	  t))
    (error nil)))

(defun TeX-terminate-paragraph (inhibit-validation)
  "Insert two newlines, breaking a paragraph for TeX.
Check for mismatched braces/$'s in paragraph being terminated.
A prefix arg inhibits the checking."
  (interactive "P")
  (or inhibit-validation
      (TeX-validate-paragraph
       (save-excursion
	 (search-backward "\n\n" nil 'move)
	 (point))
       (point))
      (message "Paragraph being closed appears to contain a mismatch"))
  (insert "\n\n"))

(defun TeX-insert-braces ()
  "Make a pair of braces and be poised to type inside of them."
  (interactive)
  (insert ?\{)
  (save-excursion
    (insert ?})))

;;; Like TeX-insert-braces, but for LaTeX.
;;; By Michael Prange and Stephen Gildea.
(defun TeX-close-LaTeX-block ()
  "Creates an \\end{...} to match \\begin{...} on the current line and
puts point on the blank line between them."
  (interactive "*")
  (let ((fail-point (point)))
    (end-of-line)
    (if (re-search-backward "\\\\begin{\\([^}\n]*\\)}"
			    (save-excursion (beginning-of-line) (point)) t)
	(let ((text (buffer-substring (match-beginning 1) (match-end 1)))
	      (indentation (current-column)))
	  (end-of-line)
	  (delete-horizontal-space)
	  (insert "\n\n")
	  (indent-to indentation)
	  (insert "\\end{" text "}")
	  (forward-line -1))
      (goto-char fail-point)
      (ding))))

;;; Invoking TeX in an inferior shell.

;;; Why use a shell instead of running TeX directly?  Because if TeX
;;; gets stuck, the user can switch to the shell window and type at it.

;;; It's kludge that we have to create a special buffer just 
;;; to write out the TeX-trailer.  It would nice if there were a
;;; function like write-region that would write literal strings.

(defun TeX-region (beg end)
  "Run TeX on the current region.  A temporary file (TeX-zap-file) is
written in directory TeX-directory, and TeX is run in that directory.
If the buffer has a header, it is written to the temporary file before
the region itself.  The buffer's header is all lines between the
strings defined by TeX-start-of-header and TeX-end-of-header
inclusive.  The header must start in the first 100 lines.  The value
of TeX-trailer is appended to the temporary file after the region."
  (interactive "r")
  (if (get-buffer "*TeX-shell*")
      (TeX-kill-job)
    (TeX-start-shell))
  (or TeX-zap-file (setq TeX-zap-file (make-temp-name "#tz")))
  (let ((tex-out-file (concat TeX-zap-file ".tex"))
	(temp-buffer (get-buffer-create " TeX-Output-Buffer"))
	(zap-directory (expand-directory-name TeX-directory)))
    (save-excursion
      (save-restriction
	(widen)
	(goto-char (point-min))
	(forward-line 100)
	(let ((search-end (point))
	      (hbeg (point-min)) (hend (point-min))
	      (default-directory zap-directory))
	  (goto-char (point-min))
	  ;; Initialize the temp file with either the header or nothing
	  (if (search-forward TeX-start-of-header search-end t)
	      (progn
		(forward-line -1)
		(setq hbeg (point))	;mark beginning of header
		(if (search-forward TeX-end-of-header nil t)
		    (progn (forward-line 1)
			   (setq hend (point)))	;mark end of header
		  (setq hbeg (point-min))))) ;no header
	  (write-region (min hbeg beg) hend tex-out-file nil nil)
	  (write-region (max beg hend) end tex-out-file t nil))
	(let ((local-tex-trailer TeX-trailer))
	  (set-buffer temp-buffer)
	  (erase-buffer)
	  ;; make sure trailer isn't hidden by a comment
	  (insert-string "\n")
	  (if local-tex-trailer (insert-string local-tex-trailer))
	  (set-buffer-directory temp-buffer zap-directory)
	  (write-region (point-min) (point-max) tex-out-file t nil))))
    (set-buffer-directory "*TeX-shell*" zap-directory)
    (send-string "TeX-shell" (concat TeX-shell-cd-command " "
				     zap-directory "\n"))
    (send-string "TeX-shell" (concat TeX-command " \""
				     tex-out-file "\"\n")))
  (TeX-recenter-output-buffer 0))

(defun TeX-start-shell ()
  (require 'shell)
  (save-excursion
    (set-buffer (make-shell "TeX-shell" "sh" "/dev/null" "-v"))
    (TeX-define-common-keys (current-local-map))))

(defun set-buffer-directory (buffer directory)
  "Set BUFFER's default directory to be DIRECTORY."
  (setq directory (expand-directory-name directory))
  (if (not (file-directory-p directory))
      (error "%s is not a directory" directory)
    (save-excursion
      (set-buffer buffer)
      (setq default-directory directory))))

;;; On Emacs version 18, this should use file-name-as-directory.
(defun expand-directory-name (name)
  "Like expand-file-name, but returns a directory name."
  (let ((dir-name (expand-file-name name)))
    (or (string-match "/$" dir-name)
	(setq dir-name (concat dir-name "/")))
    dir-name))

(defun TeX-buffer ()
  "Run TeX on current buffer.  See  TeX-region  for more information."
  (interactive)
  (TeX-region (point-min) (point-max)))

(defun TeX-kill-job ()
  "Kill the currently running TeX job."
  (interactive)
  (quit-process "TeX-shell" t))

(defun TeX-recenter-output-buffer (linenum)
  "Redisplay buffer showing current TeX job so that most recent
output can be seen.  The last line of the buffer is displayed on
line LINE of the window, or centered if LINE is nil."
  (interactive "P")
  (let ((tex-shell (get-buffer "*TeX-shell*"))
	(old-buffer (current-buffer)))
    (if (null tex-shell)
	(message "No TeX output buffer")
      (pop-to-buffer tex-shell)
      (bury-buffer tex-shell)
      (goto-char (point-max))
      (recenter (if linenum
		    (prefix-numeric-value linenum)
		  (/ (window-height) 2)))
      (pop-to-buffer old-buffer)
      )))

(defun TeX-print ()
  "Print the .dvi file made by \\[TeX-region] or \\[TeX-buffer].
Runs the shell command defined by TeX-dvi-print-command."
  (interactive)
  (send-string "TeX-shell"
	       (concat TeX-dvi-print-command " \"" TeX-zap-file ".dvi\"\n"))
  (TeX-recenter-output-buffer nil))

(defun TeX-show-print-queue ()
  "Show the print queue that \\[TeX-print] put your job on.
Runs the shell command defined by TeX-show-queue-command."
  (interactive)
  (if (not (get-buffer "*TeX-shell*"))
      (TeX-start-shell))
  (send-string "TeX-shell" (concat TeX-show-queue-command "\n"))
  (TeX-recenter-output-buffer nil))

