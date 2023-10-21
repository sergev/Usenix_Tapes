
;
; tsort: support for binary "trees" in LISP
;

; return first n elements of a list
(defun nthcar (n lis)
	(cond
		((lessp n 1) nil)
		((greaterp n (length lis)) lis)
		((equal n 1) (list (car lis)))
		(t (cons (car lis) (nthcar (sub1 n) (cdr lis))))))

; put a list into a sorted "tree"
; 	e.g., (a b c d e f g) => (d (b a c) (e f g))
(defun tsort(lis)
	(prog (len mid)
		(setq len (length lis))
		(return (caseq len
			(0 nil)
			(1 (car lis))
			(t
				(setq mid (quotient len 2))
				(and (eq (mod len 2) 0) (setq mid (sub1 mid)))
				(list
					(nth mid lis)
					(tsort (nthcar mid lis))
					(tsort (nthcdr (add1 mid) lis]

; search a tree for an element
(defun tassoc (key lis)
	(cond
		((null lis) nil)
		((eq 2 (length lis))
			(cond
				((eq key (car lis)) lis)))
		((eq key (setq first (caar lis))) (car lis))
		((equal (list key first) (sort (list key first) nil))
			(tassoc key (cadr lis)))
		(t (tassoc key (caddr lis)))))
