
;
; main body of the word-for-word Latin "translation"
;

(setq debugp nil)
(setq sentencep nil)
(setq identp t)

; return first n elements of a list
(defun nthcar (n lis)
	(cond
		((lessp n 1) nil)
		((greaterp n (length lis)) lis)
		((equal n 1) (list (car lis)))
		(t (cons (car lis) (nthcar (sub1 n) (cdr lis))))))

; return number, given a letter
(defun letcode (x)
	(cadr (assoc x '((x 0) (a 1) (b 2) (c 3) (d 4) (e 5)))))

; extract information from suffix
(defun sgetinfo (x)
	(setq sex (explode x))
	(setq sposp (car sex))
	(caseq sposp
		((a c d i p) nil)
		(n
			(setq sdecl (cadr sex))
			(setq sperson (caddr sex))
			(setq scase (cadddr sex)))
		(v
			(setq sconj (nth 1 sex))
			(setq smood (nth 2 sex))
			(setq svoice (nth 3 sex))
			(setq stense (nth 4 sex))
			(setq sperson (nth 5 sex)))
		(t nil)))

; put information "into" suffix
(defun sputinfo ()
	(setq s
		(caseq dposp
			((c d i p) dposp)
			(a (concat 'a sdecl sperson scase))
			(n (concat dposp ddecl 'sn))
			(t nil))))

; extract information from dictionary entry
(defun dgetinfo (x)
	(setq dex (explode x))
	(setq dposp (car dex))
	(caseq dposp
		(n
			(setq ddecl (nth 1 dex))
			(setq dgender (nth 2 dex)))
		(v
			(setq dconj (nth 1 dex))
			(setq dpart (nth 2 dex)))))

; return suffixes in word
(defun match (x)
	(setq root x)				; default root is word itself
	(setq matches (list (list root 'x)))
	(cond
		(x
			(setq ex (explode x)))
		(t
			(setq ex nil)))
	(setq len (sub1 (length ex)))
	(do n 1 (add1 n) (greaterp n len)
		(cond
			((setq tmpend (cadr (tassoc 
						(implode (nthcdr n ex)) suf)))
				(and (atom tmpend) (setq tmpend (list tmpend)))
				(setq root (implode (nthcar n ex)))
				(setq matches (append matches
					(list (append (list root) tmpend)))))))
	matches)

; lookup "roots" in dictionary
(defun lookup (x)
	(setq looks nil)
	(setq firstp t)
	(do lis x (cdr lis) (null lis)
		(setq key (caar lis))
		(setq entry
			(cond
				(firstp
					(tassoc key idict))))
		(cond
			((null entry)
				(setq entry (tassoc key dict))))
		(cond
			((or entry (not firstp))
				(setq looks (append looks (list (list
					(cond
						(entry
							(cadr entry))
						((not firstp)
							'(x unknown)))
					(cdar lis)))))))
		(setq firstp nil))
	looks)

; check that a normal suffix is appropriate for a single word
(defun checknorm ()
	(cond
		((eq dposp 'x)
			t)
		(t (cond
			((or (eq sposp dposp) (eq dposp 'a))
				(caseq dposp
					(n (eq ddecl sdecl))
					(v (caseq dconj
						(e (member sconj '(d e)))
						(t (or
							(eq dconj sconj)
							(eq sconj 'x)))))
					(a (eq sposp 'n))
					(t (eq dposp sposp))))))))

; check irregular words
(defun checkirreg ()
	(setq ex (explode d))
	(caseq (car ex)
		(n
			(or
				(eq 'x (nth 3 ex))
				(eq 'x (nth 1 ex))))
		(v
			(eq 'x (nth 1 ex)))
		((a c d i p)
			t)))

; check possible suffixes for ones that make sense
(defun check (x)
	(setq checks nil)
	(do lis x (cdr lis) (null lis)
		(setq sd (car lis))
		(setq d (caar sd))
		(setq sdchecks nil)
		(dgetinfo d)
		(do lis (cadr sd) (cdr lis) (null lis)
			(setq s (car lis))
			(sgetinfo s)
			(cond ((caseq sposp
					(x (checkirreg))
					(t (checknorm)))
				(caseq sposp
					(x
						(caseq dposp
							(n (sputinfo))
							(t (setq s d))))
					(n
						(and
							(eq dposp 'a)
							(sputinfo))))
				(setq sdchecks (append sdchecks (list s))))))
		(cond
			(sdchecks
				(setq checks (append checks
					(list (cons (cadar sd) sdchecks)))))))
	checks)

; print out information known about word from ending
(defun prsuf (x)
	(sgetinfo x)
	(msg (B 8))				; print one tab
	(caseq sposp
		((n a)
			(msg
				(caseq sposp
					(n "noun, ")
					(a "adjective, "))
				(caseq sdecl
					(x 'irregular)
					(t (concat (letcode sdecl) '| decl|)))
				", "
				(caseq sperson
					(s 'singular)
					(p 'plural))
				", "
				(caseq scase
					(n 'nominative)
					(g 'genitive)
					(d 'dative)
					(a 'accusative)
					(b 'ablative))))
		(a
			(msg "adverb"))
		(c
			(msg "conjunction"))
		(i
			(msg "interjection"))
		(p
			(msg "preposition"))
		(v
			(msg
				"verb, "
				(caseq sconj
					(x 'irregular)
					(t (concat (letcode sconj) '| conj|)))
				", "
				(caseq smood
					(i 'indicative)
					(s 'subjunctive))
				", "
				(caseq svoice
					(a 'active)
					(p 'passive))
				", "
				(caseq stense
					(p 'present)
					(i 'imperfect)
					(f 'future)
					(r 'perfect)
					(l 'pluperfect)
					(u "future perfect"))
				", "
				(caseq sperson
					(a "first person, sing")
					(b "second person, sing")
					(c "third person, sing")
					(d "first person, plural")
					(e "second person, plural")
					(f "third person, plural"))))
		(t
			(msg "unknown part of speech")
			nil))
	(terpri))

; print all information about one word
(defun printinfo (x)
	(msg N word ": ")
	(cond
		(x
			(setq lis (car x))
			(setq wordtrans (car lis))
			(msg wordtrans N)
			(and sentencep
				(setq trans
					(concat trans
						(concat
							(caseq wordtrans
								(unknown '|?|)
								(t wordtrans))
							'| |))))
			(and identp (mapcar 'prsuf (cdr lis))))
		(t
			(terpri)))
	(and debugp
		(msg N (B 8) matched N (B 8) looked N (B 8) checked N)))

; work on one word (figure out ending, lookup, etc.)
(defun doword (x)
	(setq word x)
	(setq matched (match word))
	(setq looked (lookup matched))
	(setq checked (check looked))
	(printinfo checked))

; "translate" words from interactive input
(defun suzy ()
	(msg N "Hello." N)
	(setsyntax #/, 'vseparator)		; ignore commas
	(prog (lis)
		loop
		(setq trans '||)
		(msg N "? ")
		(setq lis (lineread))
		(cond
			((null lis)
				(return nil))
			(t
				(caseq (car lis)
					(nil (return nil))
					((exit quit) (return nil))
					(debug (setq debugp t))
					(nodebug (setq debugp nil))
					(sentence (setq sentencep t))
					(nosentence (setq sentencep nil))
					(ident (setq identp t))
					(noident (setq identp nil))
					(t
						(mapcar 'doword lis)
						(and sentencep
							(msg N trans N))))))
		(go loop))
	(msg N "Goodbye." N N)
	(exit))
