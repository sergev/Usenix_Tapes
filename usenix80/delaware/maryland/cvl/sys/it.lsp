? This is a set of utility testers for the grinnell GMR-27 from ULISP.
{csetq grinnell (open "/dev/gr" 2)}

{csetq internal
(array 3 5)}

{internal 1 124000q}
{internal 2 130000q}
{internal 3 120000q}

{csetq it
(lambda arg
	(cond [arg <internal 2 (logor 130000q (car arg))>])
	(sys 4 internal (times 2 (arrayl internal)) grinnell)
	)}

{csetq loop
(lambda cnt
	(prog <>
	      a
	      <inttest cnt>
	      <cond [<greaterp cnt 2> <setq cnt 0>]
		    [<setq cnt (add1 cnt)>]>
	      <go a>))}

{csetq make
(lambda ()
	(prog <(msk seed) (ar (array 513 5))
		(row (prog <(cnt 1)>
			 a <ar cnt (logand 7777q msk)>
			   <setq msk (plus msk increment)>
			   <setq cnt (add1 cnt)>
			   <cond [<lessp cnt 513> <go a>]>
			   <ar 513 034011q>  ? SLU
			   <return 1>))>
	    a <sys 4 ar (times (arrayl ar) 2) grinnell>
	      <setq row (add1 row)>
	      <cond [<lessp row 513> <go a>]>
	))}

{csetq seed 7777q}

{csetq increment 0421q}

{csetq sta (array 15 5)}
{sta 1  026002q} ? LUM
{sta 2  024041q} ? LWM
{sta 3  044777q} ? LEA
{sta 4  064000q} ? LLA
{sta 5  050777q} ? LEB
{sta 6  070001q} ? LLB
{sta 7  054000q} ? LEC
{sta 8  074001q} ? LLC
{sta 9  100007q} ? LDC
{sta 10 017777q} ? LSM
{sta 11 030000q} ? ERS
{sta 12 100003q} ? LDC
{sta 13 120020q} ? SPD
{sta 14 140000q} ? LPD gray (1) vs color (0)
{sta 15 120000q} ? SPD

{csetq starts
(lambda ()
	(sys 4 sta (times 2 (arrayl sta)) grinnell)
	)}

{csetq r (array 1 5)}

{csetq b (array 11 5)}
(b 1 120400q) ? SPD Memory
(b 2 034005q) ? SLU EC LC
(b 3 160000q) ? RPD
(b 4 120004q) ? SPD Cursors
(b 5 130000q) ? LPA
(b 6 154037q) ? 0E
(b 7 154037q) ? 0L
(b 8 154077q) ? 1E
(b 9 154077q) ? 1L
(b 10 140017q) ? LPR
(b 11 144017q) ? LPR

{csetq wa
(lambda (ar . cnt)
	(cond [<null cnt> <setq cnt 1>]
	      [t <setq cnt (car cnt)>])
	(prog <>
	    a <cond [<greaterp (currcol) 72> <terpri>]>
	      <attempt [do <prin1 (logor (ar cnt))>
			   <prin1 (list cnt) (sub1 (currcol))>
			   <prin1 " ">]
		       [-9 (return (sub1 cnt))]
		       [-8 t]>
	      <setq cnt (add1 cnt)>
	      <go a>))}

{csetq wr
(lambda (ar . cnt)
	(sys 4 ar
	 (cond [cnt <car cnt>]
	       [t <times (arrayl ar) 2>]) grinnell))}

{csetq rd
(lambda (ar . cnt)
	(sys 3 ar
	 (cond [cnt <car cnt>]
	       [t <times (arrayl ar) 2>]) grinnell)
	(wa ar (stack
	 (cond [<null cnt> cnt]
	       [<cdr cnt>]
	       [t cnt])))
	)}

{csetq tsts (array 6 5)} ? Cycle thru internal tests
(tsts 1 124000q)
(tsts 2 130000q)
(tsts 3 130001q)
(tsts 4 130002q)
(tsts 5 130003q)
(tsts 6 120000q)

{csetq cam (array 7 5)}
(cam 1  100003q)?LDC
(cam 2  010000q)?LSM
(cam 3  120020q)?SPD video
(cam 4  140001q)?LPR grey
(cam 5  120002q)?SPD digitize
(cam 6  140000q)?LPR shift
(cam 7  150000q)?LPD ave
