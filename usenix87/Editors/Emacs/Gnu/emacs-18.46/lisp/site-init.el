;; startup file for WU -- define away from ^s and ^q

(define-key global-map "\^h" 'isearch-forward)
(define-key ctl-x-map "\^h" 'save-buffer)
(define-key esc-map "h" 'isearch-forward-regexp)
(define-key esc-map "r" 'isearch-backward-regexp)
(defconst search-repeat-char ?\^H "Character to repeat isearch-forward." ) 
(define-key global-map "\^\\" 'quoted-insert)
(define-key global-map "\^s" 'undefined)
(define-key global-map "\^q" 'undefined)
(define-key esc-map "\^s" 'undefined)
(define-key esc-map "\^q" 'undefined)

(define-key global-map "\^_" 'help-command)
(setq help-char ?\C-_ )
(setq mh-progs "/usr/local/bin/mh/")
(setq mh-lib "/usr/local/lib/mh/")
(fset 'help-command help-map)
