; Written by dmj@june.
; Last edited Feb. 19, 1986
; 
; By defining and assigning to the variable 
; justify-right-margin you can make it leave more right margin on the
; right than it normally would.  This is convenient, since maximum
; readability, at least for me, occurs at about 66 columns.  But I would
; like to keep the right margin at the default (77) so it will not break 
; long lines while I am trying to type in equations, tables, and the like.
; 
; 
; 
; KNOWN BUGS AND DEFICIENCIES:
; 
; For some unknown reason it sometimes scrolls the window a little.
; 
; Lines with \verbs come out a little short.
; This is because before justification \verbs are expanded by ceiling log10 n 
; characters plus one more if the first character is a space, comma, or 
; digit.  
; 
; There may be trouble if the character immediately following \verb is white.
; 


(defun
    (sentence-end ch 
	(save-excursion 
	    (while (| (= (setq ch (preceding-char)) ')')
		      (= ch '"')
		      (= ch '\'')
		      (= ch ']')
		      (= ch '}'))
		   (if (! (bobp)) (backward-character))
	    )
	    (setq ch (preceding-char))
	    (| (= ch '.')
	       (= ch '?')
	       (= ch ':')
	       (= ch '!'))
	)
    )
    
    (TeX-j-bndy
	(& (looking-at "[ \t]*$\\|%\\|.*[^\n\\]%\\|\\\\begin\\|\\\\end")
	   ; a fast, although too permisive, condition
	   (| (looking-at "[ \t]*$\\|%\\|\\\\begin\\|\\\\end")
	      ; white or leading %, \begin, or \end 
	      (save-excursion found-% delim-str search-str
		  (setq found-% 0)
		  (set-mark)
		  (end-of-line)
		  (exchange-dot-and-mark)
		  (save-restriction 
		      (narrow-region)
		      (while (& (! found-%)
				(! (error-occurred 
				       (re-search-forward 
					   "\\=%\\|[^\n\\]%\\|\\\\verb")))
			     )
			     (if (= (preceding-char) '%')
				 (setq found-% 1)
				 (error-occurred 
				     (setq delim-str 
					   (char-to-string (following-char)))
				     (forward-character)
				     (search-forward delim-str))
				 (end-of-line)
			     )
		      )
		  )
		  found-%
	      )
	   )
	)
    )


; The length of the actual verbatim string is placed right after the opening
; dilimiter.  A comma is added if the first character is a comma, a space,
; or a digit.  Then all spaces are replaced by the delimiter.

    (TeX-encode-verbs delim-char found-some size
	(setq found-some 0)
	(save-restriction 
	    (narrow-region)
	    (save-excursion 
		(beginning-of-file)
		(while (! (error-occurred 
			      (search-forward "\\verb")))
		       (setq found-some 1)
		       (setq delim-char (following-char))
		       (forward-character)
		       (set-mark)
		       (search-forward (char-to-string delim-char))
		       (backward-character)
		       (exchange-dot-and-mark)
		       (setq size (- (mark) (dot)))
		       (insert-string size)
		       (if (looking-at "[ ,0-9]") (insert-character ','))
		       (save-restriction 
			   (narrow-region)
			   (error-occurred 
			       (replace-string " " 
				   (char-to-string  delim-char)))
			   (end-of-file)
		       )
		)
	    )
	)
	found-some
    )
    
    
    (TeX-decode-verbs delim-char size
	(save-restriction 
	    (narrow-region)
	    (save-excursion 
		(beginning-of-file)
		(while (! (error-occurred 
			      (search-forward "\\verb")))
		       (setq delim-char (following-char))
		       (forward-character)
		       (set-mark)
		       (re-search-forward "[0-9]*")
		       (setq size (+ (region-to-string) 0))
		       (erase-region)
		       (if (= (following-char) ',') (delete-next-character))
		       (set-mark)
		       (goto-character (+ (dot) size))
		       (exchange-dot-and-mark)
		       (save-restriction 
			   (narrow-region)
			   (error-occurred 
			       (replace-string 
				   (char-to-string delim-char) " "))
			   (end-of-file)
		       )
		)
	    )
	)
    )
    
    
    
    (TeX-justify-paragraph old-right-margin did-move dot-char msg 
	(setq old-right-margin right-margin)
	(if prefix-argument-provided
	    (setq right-margin prefix-argument)
	    (& (is-bound justify-right-margin) (> justify-right-margin 0))
	    (setq right-margin justify-right-margin)
	)
	(setq dot-char (following-char))
	(save-excursion
	    (beginning-of-line)
	    ; kludge so that if we have just finished a paragraph we still 
	    ; justify it
	    (if (looking-at "[ \t]*$") ; white line pattern also matches eob
		(progn (previous-line)(beginning-of-line)))
	    ; find beginning of paragraph
	    (setq did-move 0)
	    (while (& (! (bobp))
		      (! (TeX-j-bndy)))
		   (progn (previous-line)
			  (beginning-of-line)
			  (setq did-move 1))
	    )
	    (if (& did-move (TeX-j-bndy))
		(progn (next-line)(beginning-of-line)))
	    
	    ; now we are at the beginning of the paragraph 
	    ; and beginning of line
	    
	    (set-mark)		; only used in fixing verbs
	    
	    (if (! (TeX-j-bndy))
		(progn last-col c-col need-to-decode-verbs
		       (setq need-to-decode-verbs 0)
		       (delete-white-space)
		       ; get first line to come out right if left margin 
		       ; is not 1
		       (to-col left-margin)
		       
		       ; make the whole paragraph into one long line
		       
		       (end-of-line)
		       (error-occurred (forward-character))
		       ; loop invariant is beginning of line following long 
		       ; line or end of buffer
		       (while  (! (TeX-j-bndy))
			       ; this will also match end-of-buffer
			       ; delete the new-line to make a longer line
			       (delete-previous-character)
			       (delete-white-space)
			       ; check for end-of-sentence.  
			       ; If so, add an extra space.
			       (if (sentence-end)
				   (insert-character ' '))
			       (insert-character ' ')
			       (end-of-line)
			       (error-occurred (forward-character))
		       )
		       ; reassemble into lines of the right length
		       ; but first get back to end of the real line
		       (if (bolp) (backward-character))
		       
		       (setq need-to-decode-verbs (TeX-encode-verbs))
		       
		       (setq c-col (current-column))
		       (while (progn
				    (setq last-col c-col)
				    (insert-character '!')
				    (delete-previous-character)
				    (end-of-line)
				    (setq c-col (current-column))
				    (< c-col last-col))
		       )
		       
		       (if need-to-decode-verbs (TeX-decode-verbs))
		       
		       (setq msg "Done!")
		)  		; end of giant progn
		(setq msg "Nothing to justify")
	    )			; end of giant if
	    
	)			; end of save-excursion
	
	; save-excursion uses sticky pointers.  We delete white space at 
	; end of lines and then re-insert it, which leaves the sticky 
	; pointer to the left of the space rather that to the right if 
	; that was where it was originally was.  So here we fix it up.
	; If there was nothing to justify, this will just be a no-op anyway.
	
	(if (& (!= dot-char (following-char))
	       (!= dot-char ' ')
	       (!= dot-char '\t')
	       (!= dot-char '\n'))
	    (error-occurred 
		(search-forward (char-to-string dot-char))
		(backward-character)
	    )
	)
	(setq right-margin old-right-margin)
	(message msg)
	(novalue)
    )
)
