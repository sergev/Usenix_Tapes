; A simple clock for the mode line.

(declare-global time-running)
(setq time-running 0)
(if (! (is-bound ships-bells)) (setq-default ships-bells 0))
(if (! (is-bound checkpoint-interval)) (setq-default checkpoint-interval 5))
(setq-default &checkpoint-interval 0)

(defun
    (time dead
	(if (|
		(setq dead (< (process-status "newtime") 0))
		(! time-running))
	    (save-excursion
		(if (! dead)
		    (kill-process "newtime"))
		(setq global-mode-string "time and load")
		(setq time-running 0)
		(start-filtered-process 
		    "exec /usr/lib/local/cmu-emacs/loadst -n 60"
		    "newtime" "newtime-filter")))
	(novalue)
    )
)

(defun (newtime-filter
	   (setq global-mode-string (process-output))
	   (if (>= (setq &checkpoint-interval (+ &checkpoint-interval 1))
		   checkpoint-interval)
	       (progn (setq &checkpoint-interval 0)
		      (if (checkpoint)
			  (message "Checkpointed..."))))
	   (if (| (= (substr global-mode-string -8 4) "Mail")
		  (= (substr global-mode-string -7 4) "Mail"))
	       (progn (if (= time-running 1)
			  (progn (send-string-to-terminal "\^G")
				 (message "You have new mail")))
		      (setq time-running 2))
	       (setq time-running 1))
	   (if ships-bells
	       (progn
		     (if (= (substr global-mode-string 2 1) ":")
			 (setq global-mode-string (concat " " global-mode-string)))
		     (if (= (substr global-mode-string 4 2) "00")
			 (bells (substr global-mode-string 1 2) 7)
			 (= (substr global-mode-string 4 2) "15")
			 (bells 1 7)
			 (= (substr global-mode-string 4 2) "30")
			 (bells 2 7)
			 (= (substr global-mode-string 4 2) "45")
			 (bells 3 7))
		     (if (= (substr global-mode-string 1 6) "12:00p")
			 (setq global-mode-string 
			       (concat "Noon" 
				       (substr global-mode-string 8 -8)))
			 (= (substr global-mode-string 1 6) "12:00a")
			 (setq global-mode-string 
			       (concat "Midnight" 
				       (substr global-mode-string 8 -8))))
		     (if (= (substr global-mode-string 6 1) "p")
			 (if (= (substr global-mode-string 1 2) "12")
			     (setq global-mode-string 
				   (concat "Lunch? " global-mode-string))
			     (= (substr global-mode-string 1 2) " 6")
			     (setq global-mode-string 
				   (concat "Dinner? " global-mode-string))
			     (= (substr global-mode-string 1 2) "11")
			     (setq global-mode-string 
				   (concat "Bedtime? " global-mode-string))))
	       ))
	   (sit-for 0)))

(defun (bells num long
	      (setq num (arg 1 "How many? "))
	      (setq long (arg 2 "How long? "))
	      (while (> num 0)
		     (send-string-to-terminal "\^G")
		     (sit-for long)
		     (setq num (- num 1)))))
;(defun
;    (newtime-filter
;	(setq global-mode-string (process-output))
;	(setq time-running 1)
;	(sit-for 0)
;    )
;)



