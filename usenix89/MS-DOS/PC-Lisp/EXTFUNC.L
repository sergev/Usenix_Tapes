; EXTFUNC.L     						
; ~~~~~~~~~     						
;     A small library of functions to help fill in the gap between PC and      
; Franz Lisp. These functions are for learning purposes only are not very
; effectient or very robust. Also included is a set of turtle graphics
; commands that will work on just about any MS-DOS machine via the BIOS.  
;	
;		Peter Ashwood-Smith

(defun member(x y)(cond((null y)nil)((equal x(car y))y)(t(member x(cdr y]  
(defun memq(x y)(cond((null y) nil)((eq x(car y))y)(t(memq x(cdr y]  
(defun tailp(l1 l2)(cond ((null l2) nil)((eq l1 l2) l1)(t(tailp l1(cdr l2]  
(defun arrayp(x) nil)		
(defun bcdp(x) nil)		
(defun bigp(x) nil)		
(defun dtpr(x) (and(listp x)(atom (cdr x)]   	
(defun fixp(n) nil)
(defun hunkp(n) nil)
(defun litatom(n) (and(atom n)(not(floatp n]   
(defun numbp(n) (floatp n))		
(defun numberp(n) (floatp n))
(defun purep(n)(or(eq n t)(eq n nil)(eq n 'lambda)(eq n 'nlambda)]  
(defun stringp(n) nil)        			
(defun symbolp(n) (litatom n))			
(defun valuep(n) nil)
(defun vectorp(n) nil)
(defun typep(n)(type n))
(defun eqstr(a b)(equal a b))
(defun neq(a b)(not(eq a b)))
(defun nequal(a b)(not(equal a b)))
(defun append1(a b)(append a (list b)))
(defun copy(a)(reverse(reverse a)))		
(defun ncons(a)(cons a nil))
(defun xcons(a b)(cons b a))
(defun last(l)(nth (- (length l) 1) l))
(defun nthcdr(n l)(cond((< n 0)(cons nil l))((= n 0)l)(t(nthcdr (- n 1)(cdr l] 
(defun nthelem(n l) (nth (- n 1) l))
(defun add fexpr(l)(eval(cons '+ l]             
(defun add1(n)(+ 1 n))
(defun diff fexpr(l)(eval(cons '- l]
(defun difference fexpr(l)(eval(cons '- l]
(defun minus(n)(- 0 n))
(defun product fexpr(l)(eval(cons '* l]
(defun times fexpr(l)(eval(cons '* l] 
(defun quotient fexpr(l)(eval(cons '/ l]
(defun sub1(n)(- n 1))
(defun evenp(n)(= (mod n 2) 0))
(defun minusp(n)(< n 0))
(defun oddp(n)(= (mod n 2) 1))
(defun onep(n)(= 1 n))
(defun plusp(n)(> n 0))
(defun zerop(n)(= n 0))
(defun infile(f)(fileopen f 'r)) 
(defun character-index(a c)(prog(n)(setq n 1 a(explode a))(cond((floatp c)(setq c(ascii c))))nxt:(cond((null a)(return nil)))(cond((eq(car a)c)(return n)))(setq n(+ n 1)a(cdr a))(go nxt:]  
	
; 
; Some simple Turtle Graphics Routines to demonstrate PC-LISP. Remember that
; the graphics commands go though the BIOS so they are portable but slow.
;			         

(defun TurtleGraphicsUp()   (#scrmde# 6) (#scrsap# 0) (TurtleCenter))	
(defun TurtleGraphicsDown() (#scrmde# 2))
(defun TurtleCenter()       (setq Lastx 100 Lasty 100 Heading 1.570796372))
(defun TurtleRight(n)       (setq Heading (+ Heading (* n 0.01745329))))
(defun TurtleLeft(n)        (setq Heading (- Heading (* n 0.01745329))))

(defun TurtleForward(n) 
      (setq Newx(+ Lastx(*(cos Heading)n))Newy(+ Lasty(*(sin Heading)n)))
      (#scrline#(* Lastx 3.2) Lasty (* Newx 3.2) Newy 1)
      (setq Lastx Newx Lasty Newy)
)

;
; end of Turtle Graphics primitives, start of Graphics demonstration code
; you can cut this out if you like and leave the Turtle primitives intact.
;

(defun Line_T(n)	
	(TurtleForward n) (TurtleRight 180)
	(TurtleForward (/ n 4))	
)
	
(defun Square(n)
	(TurtleForward n)  (TurtleRight 90)	
	(TurtleForward n)  (TurtleRight 90)	
	(TurtleForward n)  (TurtleRight 90)	
	(TurtleForward n)			
)

(defun Triangle(n)
	(TurtleForward n)  (TurtleRight 120)
	(TurtleForward n)  (TurtleRight 120)
	(TurtleForward n)
)

(defun Make(ObjectFunc Size times skew)	
      (prog()       
       TOP:(cond ((= times 0) (return)))
	   (ObjectFunc Size) 
	   (TurtleRight skew)
	   (setq times (- times 1))
	   (go TOP:)	
       )
)

(defun GraphicsDemo()
	   (TurtleGraphicsUp) 
	   (Make Square 40 18 5) (Make Square 60 18 5)
	   (gc)							; idle work
	   (TurtleGraphicsUp) 
	   (Make Triangle 40 18 5) (Make Triangle 60 18 5)
	   (gc)							; idle work
	   (TurtleGraphicsUp) 
	   (Make Line_T 80 50 10)
	   (gc)							; idle work
	   (TurtleGraphicsDown)
)
