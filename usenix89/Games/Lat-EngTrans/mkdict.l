
;
; mkdict: transform a "normal" Latin-to-English dictionary into Lisp form
;

(setq pospcodes '(
	(m n) (f n) (n n)
	(I v) (II v) (III v) (IV v) (III-io v)
	(adj a)
	(adv d)
	(prep p)
	(interj i)
	(conj c)))

(setq gencodes '(
	(arum a)
	(erum e)
	(orum b)
	(uum d)
	(ae a)
	(ei e)
	(is c)
	(um c)
	(us d)
	(i b)))

(setq conjcodes '(
	(I a)
	(II b)
	(III c)
	(IV d)
	(III-io e)))

; print out a new dictionary entry
(defun addentry ()
	(setq dict (append dict (list (list root (list code trans))))))

; return first n elements of a list
(defun nthcar (n lis)
	(cond
		((lessp n 1) nil)
		((greaterp n (length lis)) lis)
		((equal n 1) (list (car lis)))
		(t (cons (car lis) (nthcar (sub1 n) (cdr lis))))))

; return longest suffix of word, or nil if no match found
(defun match (x)
	(setq root x)			; default root is word itself
	(setq decl nil)
	(cond
		(x
			(setq ex (explode x)))
		(t
			(setq ex nil)))
	(setq len (sub1 (length ex)))
	(do n 0 (add1 n) (or decl (greaterp n len))
		(cond
			((setq genmatch
				(assoc (implode (nthcdr n ex)) gencodes))
				(setq decl (cadr genmatch))
				(setq root (implode (nthcar n ex))))))
	root)

(defun donoun () 
	(match (cadr parts))
	(cond
		((neq root (implode (nthcar (length (explode root))
						(explode (car parts)))))
			(setq code (uconcat pospcode decl posp))
			(addentry)
			(setq root (car parts))
			(setq code (uconcat pospcode decl posp 'x))
			(addentry))
		((eq root (car parts))
			(setq code (uconcat pospcode decl posp 'x))
			(addentry))
		(t
			(setq code (uconcat pospcode decl posp))
			(addentry))))

; strip of n characters off end of x
(defun strip (n x)
	(setq ex (explode x))
	(implode (nthcar (diff (length ex) n) ex)))

(defun doverb ()
	(setq conjcode (cadr (assoc posp conjcodes)))
	(setq root (strip 3 (nth 1 parts)))
	(setq code (uconcat pospcode conjcode 'b))
	(addentry)
	(setq root (strip 1 (nth 2 parts)))
	(setq code (uconcat pospcode conjcode 'c))
	(addentry)
	(setq root (strip 2 (nth 3 parts)))
	(setq code (uconcat pospcode conjcode 'd))
	(addentry))

(defun doadj ()
	(setq root (strip 2 (car parts)))
	(setq code pospcode)
	(addentry))

(defun doadv ()
	(cond
		((eq 1 (length parts))
			(setq root (car parts))
			(setq code pospcode)
			(addentry))))

(defun doodd ()
	(setq root (car parts))
	(setq code pospcode)
	(addentry))

; print out an error message and the current line
(defun dicterr (message)
	(setq errorp t)
	(msg "error: " message ": " line N))

; process a dictionary line, based on part of speech
(defun doline ()
	(setq parts nil)
	(setq posp nil)
	(do lis line (cdr lis)
		(or posp (null lis))
		(setq word (car lis))
		(cond
			((listp word)
				(setq posp (car word))
				(setq pospcode (cadr (assoc posp pospcodes)))
				(setq trans (cadr lis)))
			(t
				(setq parts (cond
					((null parts) (list word))
					(t (append parts (list word))))))))
	(caseq pospcode
		(n (donoun))
		(v (doverb))
		(a (doadj))
		(d (doadv))
		((p i c r) (doodd))
		(t
			(dicterr "bad part-of-speech"))))

; read in a file, one line at a time, until the word "nil" is encountered
(defun mkdict (filename)
	(setq filein (infile filename))
	(setq dict nil)
	(do line (lineread filein) (lineread filein) (equal line '(zzz))
		(setq errorp nil)
		(doline))
	(close filein))
