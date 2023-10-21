; 
; Written by Donald Acton and Barry Brachman with help from Marc Majka
;  March 11/85
;
; Dept. of Computer Science
; University of British Columbia
; 

(defun 
    (sp spword
      (setq spword (get-tty-string "Word: " ))
      (do-sp spword)
    )
)

(defun						; bjb
    (sp-from-buffer spword
	(setq spword (get-next-word))
	(message (concat "Word: " spword))
	(sit-for 0)
	(do-sp spword)
    )
)

(defun
    (do-sp curr found tmp
      (setq curr (current-buffer-name))
      (pop-to-buffer "sp")
      (setq needs-checkpointing 0)
      (erase-buffer)
      (set-mark)
      (filter-region (concat "sp " spword))
      (beginning-of-file)
      (setq found " *not found*")
      (setq tmp case-fold-search)
      (setq case-fold-search 1)
      (error-occurred (re-search-forward (concat "\. " spword "$"))
                       (beginning-of-line)
                       (line-to-top-of-window)
                       (setq found " *found*"))
      (setq case-fold-search tmp)
      (setq mode-line-format " %b - %m      %p")
      (setq mode-string (concat spword found))
      (pop-to-buffer curr)
      (novalue)))

;
; Return the word the cursor is pointing at
; or the word immediately to the left of the
; cursor if it is between words
;
; April 15/85 - bjb
;
(defun
    (get-next-word original-dot rb spword
	(save-excursion
	    (setq original-dot (dot))
	    (backward-word) (forward-word) (setq rb (dot))
	    (if (> original-dot rb)
		(progn (forward-word) (backward-word))
	    (backward-word))
	    (set-mark)
	    (forward-word)
	    (setq spword (region-to-string))
	)
	spword
   )
) 

