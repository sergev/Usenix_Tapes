;  Originally from somewhere.  Extensively modified by dmj@uw-june
;  Last edited Feb. 19, 1986.
; 
; The following extensions/modifications to the original TeX-mode 
; have been made:
; 
; 1.  Since double-quotes are normalling illegal in TeX, unless the
;     variable TeX-expand-quote is set, typing a '"' issues a warning.
; 2.  The function "TeX-$" and its subsidiary "in-math-mode" were recoded
;     for greater speed, at least on Emacs-264.
; 3.  ESC-\ makes prompts for a LaTeX environment (?) name.  Say you
;     type "foo".  It inserts \begin{foo}   \end{foo}, in a (hopefully)
;     pleasing arrangement.  The mark is left right after the \begin{foo}
;     so you can use ^X-^X (exchange-dot-and-mark) to insert parameters
;     and return to your former place.
; 4.  Suppose the last braced command you typed was "{\foo ... }"
;     The next time you type "{", simply typing a space will insert \foo.
;     Note, the string foo is captured when its closing "}" was typed.
;     As an experimental feature, it forgets the string if you don't
;     use it for a while (twice, right now).
; 5.  ESC-m on a word will surround the word with "{\em ... }"
; 6.  ESC-j will justify a paragraph.  It works very hard to do this right.
;     Paragraphs are bounded by blanks or by \begin or \end or by
;     TeX comments ("% ...").   It will not break a \verb+ ... +.  (+ 
;     is not special.  Any character except blank will do.)  Neither
;     "\%" nor "\verb+  %  +" will be considered comments.
;     It uses the following to determine the right margin, in order of
;     increasing priority: current right margin - 10, the buffer specific 
;     variable "justify-right-margin", a ^U prefix. TeX-justify is in
;     a separate file, TeX-justify.ml.
; 7.  If the text is like "\cite{Knuth84}", with the cursor on Knuth84, the
;     bibTeX.ml function "describe-cite-in-buffer" will be invoked to
;     display the citation in another window.
; 
; As for as the original TeX-mode code goes, I (dmj) am not even sure
; what some of it is supposed to do.  Here is what some of the stuff
; not mentioned above does.
; 
; $ is bound to TeX-$ which at the end of a math mode region bounces to 
; the matching $ at the beginning of the math region.  Since the beginning
; ending are both delimited by $, it has to search from the beginning of the
; file.  This can get slow, so somewhere where you are sure you are not in
; math mode, hit ESC-$, which is bound to TeX-end-math-region.  This will put
; in the string "%emacs-mathOK".  TeX-$ only searches back as far as the
; nearest occurance of this.
; 
; TeX-brace is what "}" was formerly bound to.  

(if (! (is-bound TeX-cmd-string))
    (progn (declare-global TeX-cmd-string)
	   (setq TeX-cmd-string ""))
)

(declare-global TeX-cmd-cnt)

(defun 
    
    (TeX-warn-character
	(insert-character (last-key-struck))
	(error-message "Really?")
    )
    
    (TeX-double-quote lpos rpos
	(if (= '\134' (preceding-char))
	    (insert-character '"')
	    (progn 
		   (save-excursion 
		       (setq lpos 0)
		       (setq rpos 0)
		       (set-mark)
		       (error-occurred 
			   (search-reverse "``")
			   (setq lpos (dot))
			   (exchange-dot-and-mark)
			   (search-reverse "''")
			   (setq rpos (dot)))
		   )
		   (if (> lpos rpos)
		       (insert-string "''")
		       (insert-string "``"))))
    )
    
    (insert-TeX-aux-macros
	(beginning-of-line)
	(insert-string (concat "% \\noflash{...text...} makes a box as wide"
			       " as its arg, but which is whitespace\n"
			       "\\def\\noflash#1{\\setbox0=\\hbox{#1}\\hbox"
			       " to 1\\wd0{\\hfill}}\n"))
    )
    
    
    (skip-white-space
	(re-search-forward "[^	 ]")
	(backward-character)
    )
    
    (setup-indented-TeX-display prior nfl-ins padding col
	(save-excursion 
	    (delete-region-to-buffer "TeX indent temp")
	    (temp-use-buffer "TeX indent temp")
	    (beginning-of-file)
	    (setq padding "                    ")
	    (setq prior padding)
	    (while (! (eobp))
		   (skip-white-space)
		   (setq col (current-column))
		   (delete-white-space)
		   (if (eolp)
		       (insert-string "\\vskip 5pt")
		       (progn 
			      (setq nfl-ins (substr prior 1 (- col 1)))
			      (if (= (length nfl-ins) 0)
				  (insert-string "\\hbox{")
				  (insert-string (concat "\\hbox{\\noflash{"
							 nfl-ins "}")))
			      (set-mark)
			      (end-of-line)
			      (setq prior (concat nfl-ins (region-to-string)
						  padding))
			      (insert-character '}'))
		   )
		   (next-line)
		   (beginning-of-line)
	    )
	    (insert-character '}')
	    (beginning-of-file)
	    (insert-string "\\vbox{")
	)
	(yank-buffer "TeX indent temp")
	(novalue)
    )
    
    
    (unsetup-indented-TeX-display
	(search-reverse "\\vbox{")
	(set-mark)
	(forward-paren)
	(save-excursion 
	    (delete-region-to-buffer "TeX indent temp")
	    (temp-use-buffer "TeX indent temp")
	    (end-of-file)
	    (delete-previous-character)
	    (beginning-of-file)
	    (provide-prefix-argument 6 (delete-next-character))
	    (while (! (eobp))
		   (delete-next-character)
		   (if (!= 'h' (following-char))
		       ; a \vskip line (blank)
		       (kill-to-end-of-line)
		       ; an \hbox line
		       (progn
			     (provide-prefix-argument 5
				 (delete-next-character))
			     (if (= '\134' (following-char)); backslash
				 (replace-noflash))
			     (end-of-line)
			     (delete-previous-character)
		       )
		   )
		   (next-line)
		   (beginning-of-line)
	    )
	)
	(yank-buffer "TeX indent temp")
	(novalue)
    )
    
    (replace-noflash col
	(set-mark)
	(search-forward "}")
	(setq col (current-column))
	(delete-to-killbuffer)
	(to-col (- col 10))
    )
    
    
    (in-math-mode  mm-count 
	(save-excursion 
	    (insert-character ' ')
	    (set-mark)
	    (if (error-occurred (search-reverse "%emacs-mathOK"))
		(beginning-of-file))
	    (save-restriction 
		(narrow-region)
		(if (= '\$' (following-char))
		    (setq mm-count 1)
		    (setq mm-count 0))
		(while (! (error-occurred 
			      (re-search-forward "[^\\\$]\$\$*[^\$]")
			      (backward-character))
		       )
		       (setq mm-count (+ mm-count 1))
		)
		(end-of-file)
		(delete-previous-character); kill space
	    ) 			;  end of save-restriction
	)			;  end of save-excursion
	(% mm-count 2)
    )
    
    
    (TeX-end-math-region
	(if (in-math-mode)
	    (error-message "You're still in math mode!")
	    (progn 
		   (if (! (bolp))
		       (insert-character '\n'))
		   (insert-string "%emacs-mathOK\n")
	    )
	)
    )
    
    (check-too-many-$ l$
	; Checks if 3 $s are just before cursor.
	; Complains in \$$$ case as well.
	(save-excursion 
	    (setq l$ (dot))
	    (while (= (preceding-char) '$')
		   (backward-character))
	    (if (> (- l$ (dot)) 2)
		(error-message "Too many '$'s"))
	)
    )
    
    (rev-find-mm$
	(error-occurred 
	    (re-search-reverse "[^\\\$]\$\$*[^\$]")
	    (forward-character))
    )
    
    (TeX-$ c
	   (setq c (preceding-char))
	   (insert-character (last-key-struck))
	   (if (& (!= c '\134') (! (in-math-mode))) ; \134 = backslash
	       (save-excursion 
		   (if (! (eobp))
		       (re-search-reverse "[^\\\$]\$\$*[^\$]"))
		   (if (error-occurred 
			   (re-search-reverse "[^\\\$]\$\$*[^\$]")
			   (forward-character))
		       (beginning-of-file))		      
		   (if (dot-is-visible)
		       (sit-for 5)
		       (progn
			     (beginning-of-line)
			     (set-mark)
			     (end-of-line)
			     (message (region-to-string)))
		   )
	       )
	   )
	   (check-too-many-$) 
    )
    
    
    (c-paren
	    (insert-character (last-key-struck))
	    (save-excursion
		(backward-paren)
		(if (dot-is-visible)
		    (sit-for 5)
		    (progn
			  (beginning-of-line)
			  (set-mark)
			  (end-of-line)
			  (message (region-to-string)))
		)
	    )
    )
    
    (TeX-brace
	(if (= (preceding-char) '\134'); backslash
	    (insert-character (last-key-struck))
	    (c-paren)
	)
    )
    
    ; TeX-begin leaves the mark at the end of \begin{...} 
    ; the dot on the next line
    ; and \end{...} after that.  Hence you can add parameters by simply doing 
    ; ^X-^X and get back to the text area with another ^X-^X.
    
    (TeX-begin env-name current-col 
	(setq env-name (arg 1 "\\begin: "))
	(setq current-col (current-column))
	(insert-string "\\begin{")
	(insert-string env-name)
	(insert-character '}')
	(set-mark)
	(newline)
	(to-col current-col)
	(save-excursion 
	    (newline)
	    (to-col current-col)
	    (insert-string "\\end{")
	    (insert-string env-name)
	    (insert-character '}')
	    (if (| (! (eolp)) (eobp)) 
		(progn (newline)
		       (to-col current-col)
		)
	    )
	    (if (! (dot-is-visible)) (sit-for 0))
	)
    )
    
    (TeX-left-brace char prevchar
	(setq prevchar (preceding-char))
	(insert-character (last-key-struck))
	(if (& (!= TeX-cmd-string "")
	       (> TeX-cmd-cnt 0)
	       (!= prevchar '\134')  ; backslash
	    )
	    (progn 
		   (message "<space> => " TeX-cmd-string)
		   (setq char (get-tty-character))
		   (if (= char ' ')
		       (progn 
			      (insert-string TeX-cmd-string)
			      (setq TeX-cmd-cnt 2)
		       )
		       (setq TeX-cmd-cnt (- TeX-cmd-cnt 1))
		   )
		   (push-back-character char)
	    )
	)
    )
    
    (TeX-right-brace
	(if (= (preceding-char) '\134');  backslash
	    (insert-character (last-key-struck))
	    (progn 
		   (save-excursion 
		       (backward-paren)
		       (forward-character)
		       (if (= (following-char) '\134'); backslash
			   (progn (set-mark)
				  (forward-word)
				  (setq TeX-cmd-string (region-to-string))
				  (setq TeX-cmd-cnt 2)
			   )
		       )
		   )
		   (c-paren)
	    )
	)
    )
    
    (TeX-emphasize-word word-beg
	(error-occurred (forward-character))
	(backward-word)
	(setq word-beg (dot))
	(backward-word)
	(forward-word)
	(if (& (= (following-char) '}')
	       (save-excursion 
		   (forward-character)
		   (backward-paren)
		   (looking-at "{\\\\em"))
	    )
	    (progn (delete-next-character)
		   (goto-character word-beg))
	    (progn (goto-character word-beg)
		   (insert-string "{\\em "))
	)
	(forward-word)
	(insert-character '}')
	(forward-word)
	(if (! (eobp)) (backward-word))
    )
    
    (TeX-mode
	(local-bind-to-key "TeX-justify-paragraph" (+ 128 'j'))
	(local-bind-to-key "TeX-end-math-region" (+ 128 '$'))
	(local-bind-to-key "TeX-right-brace" '}')
	(local-bind-to-key "TeX-left-brace" '{')
	(local-bind-to-key "TeX-begin" "\e\134"); ESC-backslash
	(local-bind-to-key "TeX-emphasize-word" "\em"); pun?
	(local-bind-to-key "describe-cite-in-buffer" "\^X\^D")
	(if (! (is-bound TeX-inhibit-$))
	    (local-bind-to-key "TeX-$" '$'))
	(if (is-bound TeX-expand-quote)
	    (local-bind-to-key "TeX-double-quote" '\"')
	    (local-bind-to-key "TeX-warn-character" '\"'))
	(setq right-margin 77)
	(setq mode-string "TeX")
	(setq case-fold-search 1)
	(use-syntax-table "TeX-mode")
	(modify-syntax-entry "()   (")
	(modify-syntax-entry "(]   [")
	(modify-syntax-entry "(}   {")
	(modify-syntax-entry ")(   )")  
	(modify-syntax-entry ")[   ]")
	(modify-syntax-entry "){   }")
	(modify-syntax-entry "\\    \134")
	(use-abbrev-table "text-mode")
	(setq left-margin 1)
	(if (is-bound TeX-mode-hook)
	    (execute-mlisp-line TeX-mode-hook)
	)
	(novalue)
    )
)

(autoload "TeX-justify-paragraph" "TeX-justify.ml")
(autoload "describe-cite-in-buffer" "bibTeX.ml")
(novalue)
