	
;
; mkit: read in dictionary lists, suffix lists, etc.; then save them as
;	binary trees
;

(defun mkit ()
	(load 'tsort)
	(setq ftype (read))
	(caseq ftype
		(dict.l
			(load 'mkdict)
			(mkdict 'dictionary)
			(sortcar dict nil)
			(setq dict (tsort dict))
			(pp (F dict.l) dict)
			(msg "mkit: " ftype " made" N)
			t)
		(idict.l
			(load 'idictionary)
			(sortcar idict nil)
			(setq idict (tsort idict))
			(pp (F idict.l) idict)
			(msg "mkit: " ftype " made" N)
			t)
		(suf.l
			(load 'suffixes)
			(sortcar suf nil)
			(setq suf (tsort suf))
			(pp (F suf.l) suf)
			(msg "mkit: " ftype " made" N)
			t)
		(t
			(msg "mkit: bad option: " ftype N)))
	(exit))

(setq user-top-level 'mkit)
