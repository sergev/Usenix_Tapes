; BibTeX-mode, by David Jacobson, dmj@uw-june
; Last edited Feb. 19, 1986
; This provides help for editing .bib files for BibTeX.  
; Several functions are provided:
; 
; find-key will find a specified key in a sorted .bib file or find the place 
; where that key should be inserted.  
; 
; @article will insert a shell with all the possible fields for an article
; @<entry-type> will do the same for the other legal entry-types.
; 
; next-field (ESC-n) will move to the next un-filled-in field and announce 
; whether it is required, optional, etc.  If next-field is invoked from
; the text portion of an empty field (typically dot is inside ""), 
; that field is deleted, except on the key, in which case the last value 
; used in a find-key will be inserted.
; 
; justify-field (ESC-j) will justify a field.
; 
; forward-entry and backward-entry do the obvious.  They are bound to ESC-] 
; and ESC-[
; 
; If the global varible bibliography-shell-directory is defined, 
; the shells will be taken from there.
; 
; If the global variable extra-bib-fields-file is defined, the contents of 
; the named file will be added after the last field;  commas are fixed.
; 
; The function describe-abbrev-in-buffer (^X-^D) describes the bibTeX 
; abbreviation under the cursor.  Once it has been invoked there will be a 
; buffer named Bib-Abbrevs.  It can be perused to see all abbreviations.
; 
; The function describe-cite-in-buffer is intended to be invoked in another
; buffer.  (It is bound to ^X-^D in TeX-mode buffers.)  It finds the
; bibliography entry corresponding to the citation under the dot and
; displays it in another window.  If the same directory as the current
; file contains a file named .bibfile, the name of the bibliography
; file is taken from there.  (No shell metacharacters like ~ or .., please.)
; If no .bibfile exists, the user is prompted for a file name.
; To change the bibfile used by a buffer set the buffer specific
; variable bibliography-file to the name of the desired file.
; 
; 
; BUGS and LIMITATIONS:
;
; justify-field requires the field be delimited by '"' marks.  
; 
; find-key is based on collating sequence rather than alphabetical order.
; 
; The @<entry-type> functions are not undoable, neither is the first
; invocation of describe-abbrev-in-buffer or describe-cite-in-buffer.
; 
; The @<entry-type> functions conflict with the same ones in Scribe-mode.  
; Loading both packages at once will make one of them inoperative.
; 
; When saving a file after invoking any @<entry-type> function, the system
; will often give a warning approximately "File has been modified, do you
; want to overwrite it?"  This is because of an emacs bug.  Reply yes.
; 
; Describe-cite-in-buffer malfunctions when invoked in its own bibfile.
; 
; The re's that describe the beginning of an entry are not all consistent.
; It's sloppiness, not a feature.
; 
; NOTE TO INSTALLERS:
; 
; Hard coded is the directory ~dmj/latex/bibshells/ just below.  If this 
; is installed on machines other than uw-june, that entire directory must be
; copied and the new name must replace that below.  
; 

(defun 
    
    (insert-bib-shell bibshelldir
	(setq bibshelldir "~dmj/latex/bibshells")
	(if (is-bound bibliography-shell-directory)
	    (setq bibshelldir bibliography-shell-directory))
	(insert-file (concat bibshelldir "/" 
			     (arg 1 "Bibliography type: ")))
	(newline)
	(if (is-bound extra-bib-fields-file)
	    (save-excursion 
		(re-search-forward "[ \t\n]*)")
		(region-around-match 0)
		(exchange-dot-and-mark)
		(insert-character ',')
		(end-of-line)	; probably is already there, but make sure
		(forward-character)
		(insert-file extra-bib-fields-file)
	    )
	)
	(undo-boundary)
	(next-field)
    )
    
    (@article (insert-bib-shell "article"))
    (@book (insert-bib-shell "book"))
    (@booklet (insert-bib-shell "booklet"))
    (@conference (insert-bib-shell "inproceedings"))
    (@inbook (insert-bib-shell "inbook"))
    (@incollection (insert-bib-shell "incollection"))
    (@inproceedings (insert-bib-shell "inproceedings"))
    (@manual (insert-bib-shell "manual"))
    (@mastersthesis (insert-bib-shell "mastersthesis"))
    (@misc (insert-bib-shell "misc"))
    (@phdthesis (insert-bib-shell "phdthesis"))
    (@proceedings (insert-bib-shell "proceedings"))
    (@techreport (insert-bib-shell "techreport"))
    (@unpublished (insert-bib-shell "unpublished"))
    
    
    (next-field newdot string end-of-entry
	(if (! (is-bound Key-Search-String))
	    (progn 
		   (declare-buffer-specific Key-Search-String)
		   (setq Key-Search-String "")
	    )
	)
	(setq end-of-entry 0)
	(if (& (! (bobp))
	       (save-excursion
		   (backward-character)
		   (looking-at "\"\"\\|=,\\|=[ \t\n]*)"))
	       ; "" or =, or =<white-space><rt. paren>
	    )
	    (save-excursion
		(if (= (following-char) '"')
		    (forward-character))
		(set-mark)
		(search-reverse ",")
		(erase-region)
		(setq end-of-entry (looking-at "[ \t\n]*)"))
	    )
	    (& 
	       (= (following-char) ',')
	       (save-excursion 
		   (beginning-of-line)
		   (looking-at "@[A-Za-z]*(,[ \t]*$")
	       )
	    )
	    (insert-string Key-Search-String)
	)
	(if end-of-entry
	    (message "Last field deleted")
	    (progn 
		   (save-excursion
		       (re-search-forward "[*?@&|+]*>>")
		       (region-around-match 0)
		       (setq string (region-to-string))
		       (erase-region)
		       (setq newdot (dot))
		   )
		   
		   (goto-character newdot)
		   
		   (if (= string ">>") (message "Required")
		       (= string "?>>")(message "Optional")
		       (= string "|>>")(message "Or the following")
		       (= string "&|>>")(message "And/Or the following")
		       (= string "@>>") (message "Alternative")
		       (= string "*>>") (message "Required only if no author or editor is specified")
		       (= string "+>>") (message "Required: (" 
					    Key-Search-String ")")
		       (message string " is unknown -- see a local expert")
		       ; local expert: if the bibshell file is 
		       ; not corrupted, send a bug report to
		       ; dmj@uw-june or
		       ; ...ihnp4!uw-beaver!uw-june!dmj
		   )
	    )
	)
    )
    
    (next-key new-dot key
	(save-excursion 
	    (re-search-forward "^@.*(\\(.*\\),[ \t]*")
	    (region-around-match 1)
	    (setq key (region-to-string))
	    (setq new-dot (mark))
	)
	(goto-character new-dot)
	key
    )
    
    (forward-entry 
	(if (looking-at "^@.*(.*,")
	    (end-of-line))
	(if (error-occurred (next-key))
	    (end-of-file)
	    (beginning-of-line)
	)
	(novalue)
    )
    
    (previous-key new-dot key
	(save-excursion 
	    (beginning-of-line)	; emacs rev-searching finds the pattern if
	    ; dot is anywhere in it.  This prevents that.
	    (re-search-reverse "^@.*(\\(.*\\),[ \t]*")
	    (region-around-match 1)
	    (setq key (region-to-string))
	    (setq new-dot (mark))
	)
	(goto-character new-dot)
	key
    )
    
    (backward-entry
	(previous-key)
	(beginning-of-line)
    )
    
    (find-key temp-key not-found
	(declare-buffer-specific Key-Search-String)
	(setq Key-Search-String (arg 1 ": find-key Key: "))
	(beginning-of-file)
	(if (error-occurred 
		(while (progn 
			      (setq temp-key (next-key))
			      (< temp-key Key-Search-String))
		)
		; key we are on is >= Key-Search-String
		(if (!= temp-key Key-Search-String) ;  must be >
		    (progn 
			   (error-occurred 
			       (beginning-of-line)
			       (previous-line))
		    )
		)
	    )
	    (end-of-file) 	; pattern was not found
	)
	(novalue) 
	
    )
    
    (fast-find-key		; not same semantics as find-key
	(re-search-forward
	    (concat "^@[0-9A-Za-z]*(" (quote (arg 1 "Key: ")) ",")
	)
    )
    
    (justify-field key-loc eq-loc c-col last-col start-col dot-col dot-char
	no-quote
	(setq dot-char (following-char))
	(setq dot-col (current-column))
	(save-excursion 
	    (previous-key)
	    (setq key-loc (dot))
	)
	(save-excursion 
	    (if (looking-at ".*="); there is an = ahead
		(search-forward "=")
		(progn (search-reverse "=")
		       (if (< (dot) key-loc)
			   (error-message "Cannot find field"))
		       (forward-character)
		)
	    )
	    (setq eq-loc (dot))
	    (if (! (looking-at "[ \t]*\""))
		(error-message "Not a quoted string")
	    )
	    (search-forward "\"")
	    ;       close space between '=' and '"'
	    ;	    (backward-character)
	    ;	    (delete-white-space)
	    ;	    (forward-character)
	    (setq start-col (current-column))
	    ; find end of region to justify
	    (if (error-occurred 
		    (re-search-forward "\"\\|^[ \t]*\n\\|^[ \t][ \t]*)[ \t]*$\\|^@.*("))
		(progn 
		       (end-of-file)
		       (setq no-quote 1)
		)
		(progn 
		       (re-search-reverse "")
		       (if (setq no-quote (! (looking-at "\"")))
			   (previous-line)
		       )
		)
	    )
	    
	    ; make it one big line		       
	    (while (progn 
			  (beginning-of-line)
			  (> (dot) eq-loc)
		   )
		   (delete-previous-character)
		   (delete-white-space)
		   (insert-character ' ')
	    )
	    ; reassemble into lines of the right length
	    (setq left-margin start-col)
	    (end-of-line)		       
	    (setq c-col (current-column))
	    (while (progn
			 (setq last-col c-col)
			 (insert-character '!')
			 (delete-previous-character)
			 (end-of-line)
			 (setq c-col (current-column))
			 (< c-col last-col))
	    )
	    (setq left-margin 1)
	)
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
	
	;when the dot is out in the left margin it winds up at the end of the 
	;previous line.  We would like it to be the case that if no text 
	;moves, then neither does the cursor.  But 
	;as a temporary hack we will just do the following: 
	
	(if (& (eolp) (! (eobp))) (forward-character)) 	;hack
	
	(if no-quote (message "Warning: no closing quote"))
	(novalue)
    )
    
    (fetch&use-Bib-Abbrevs-buffer full-name abbrev
	(temp-use-buffer "Bib-Abbrevs")
	(setq needs-checkpointing 0)
	(setq case-fold-search 1)
	(if (= (buffer-size) 0)
	    (progn 
		   (insert-file "/usr/lib/tex/macros/alpha.bst")
		   (beginning-of-file)
		   (search-forward "MACRO")
		   (beginning-of-line)
		   (set-mark)
		   (beginning-of-file)
		   (erase-region)
		   (end-of-file)
		   (set-mark)
		   (search-reverse "MACRO")
		   (next-line)
		   (next-line)
		   (erase-region)
		   (beginning-of-file)
		   (while (! (eobp))
			  (if (looking-at 
				  "^MACRO[ \t]*{\\([a-z]*\\)}[ \t\n]*{\\(.*\\)}.*$"
			      )
			      (progn (region-around-match 1)
				     (setq abbrev (region-to-string))
				     (region-around-match 2)
				     (setq full-name (region-to-string))
				     (region-around-match 0)
				     (erase-region)
				     (insert-string abbrev)
				     (insert-character '\t')
				     (insert-string full-name)
				     (beginning-of-line)
				     (next-line)
			      )
			      (progn
				    (set-mark)
				    (next-line)
				    (erase-region)
			      )
			  )
		   )
		   
	    )
	)
    )
    
    (describe-abbrev-in-buffer abbrev expanded
	(save-excursion
	    (error-occurred (forward-character))
	    (backward-word)
	    (set-mark)
	    (forward-word)
	    (setq abbrev (region-to-string))
	    (fetch&use-Bib-Abbrevs-buffer)
	    (beginning-of-file)
	    (search-forward (concat abbrev "\t"))
	    (set-mark)
	    (end-of-line)
	    (setq expanded (region-to-string))
	)
	(message expanded)
    )
    
    (current-file-directory fname
	(setq fname (current-file-name))
	(save-excursion 
	    (temp-use-buffer "Bibfilename Buffer")
	    (erase-buffer)
	    (insert-string fname)
	    (search-reverse "/")
	    (set-mark)
	    (end-of-line)
	    (erase-region)
	    (beginning-of-line)
	    (region-to-string)
	)
    )
    
    
    
    (describe-cite-in-buffer bib-key filename indirectbibfile currentdir
	(if (| (! (is-bound bibliography-file))
	       (= bibliography-file "")
	    )
	    (progn 
		   (declare-buffer-specific bibliography-file)
		   (setq currentdir (current-file-directory))
		   (setq indirectbibfile (concat currentdir "/.bibfile"))
		   (if (file-exists indirectbibfile)
		       (save-excursion
			   (temp-use-buffer "Bibfile Name")
			   (erase-buffer)
			   (insert-file indirectbibfile)
			   (beginning-of-file)
			   (set-mark)
			   (if (!= (following-char) '/')
			       (progn (insert-string currentdir)
				      (insert-character '/')
			       )
			   )
			   (end-of-line)
			   (delete-white-space)
			   (setq filename (region-to-string))
		       )
		       (setq filename (get-tty-file "Bibliography file: "))
		   )
		   (setq bibliography-file "") 
		   (if (= filename "") (error-message "No file name"))
		   (if (! (file-exists filename))
		       (if (!= 'y' (string-to-char (get-tty-string (concat "\"" filename "\" does not exist.  Should I go on? "))))
			   (error-message "Aborted.")
		       )
		   )
	    )
	    (setq filename bibliography-file)
	)
	(save-excursion  orig-buffer first-time
	    (setq orig-buffer (dot))
	    (re-search-reverse 
		"[{, \t\n]\\([^,} \t\n]*\\)[,} \t\n]\\|[{, \t\n]\\([^,} \t\n]*\\)\\'")
	    (region-around-match 1)
	    ; re-search-reverse == back up 1 char at a time, do looking-at
	    (setq bib-key (region-to-string))
	    (if (= bib-key "")
		(progn (region-around-match 2)
		       (setq bib-key (region-to-string)))
	    )
	    (setq first-time (= bibliography-file ""))
	    (error-occurred (visit-file filename))
	    (if (& first-time
		   (!= mode-string "BibTeX")
		)
		(if (= 'y' (string-to-char (get-tty-string 
					       (concat (current-buffer-name) " not in BibTeX mode.  Should I set it? "))))
		    (bibTeX-mode)
		)
	    )
	    (setq filename (current-file-name))
	    (save-excursion 
		(temp-use-buffer orig-buffer)
		(setq bibliography-file filename)
	    )
	    (beginning-of-file)
	    (if (error-occurred (fast-find-key bib-key))
		(progn 
		       (find-key bib-key)
		       (error-message "\""bib-key "\" not found")
		)
		(line-to-top-of-window)
	    )
	)
    )
    
    
    (bibTeX-mode
	(TeX-mode)
	(local-bind-to-key "next-field" "\en")
	(local-bind-to-key "justify-field" "\ej")
	(local-bind-to-key "forward-entry" "\e]")
	(if (| (= (global-binding-of "\e[") "backward-paragraph")
	       (= (global-binding-of "\e[") "nothing")
	    )
	    (local-bind-to-key "backward-entry" "\e["))
	; vt100's and vt2xx's use ESC-[ for their special keys.  
	; If it gets locally bound to backward-entry real havoc ensues.
	(local-bind-to-key "describe-abbrev-in-buffer" "\^X\^D")
	(remove-local-binding '"')
	(setq mode-string "BibTeX")
	(declare-buffer-specific Key-Search-String)
	(setq Key-Search-String "")
	(novalue)
    )
    
)
