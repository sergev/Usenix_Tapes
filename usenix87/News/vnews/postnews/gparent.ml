; This mlisp macro reads in the body of the article being followed up,
; preceding each line with a "> ".
; It is intended to be bound to ^X^Y.

(defun
    (gparent
	(end-of-file)
	(save-excursion
	    (set-mark)
	    (insert-file (getenv "A"))
	    (re-search-forward "^$")
	    (next-line)
	    (erase-region)
	    (re-replace-string "^" "> ")
	)
    )
)
