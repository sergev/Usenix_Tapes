{csetq b "/dev/bu_rmt0"}
{csetq w "/dev/rw_rmt0"}
{csetq n "/dev/nrw_rmt0"}
{csetq s "/dev/rmt0"}
{csetq d (lambda () (sys 3 a (arrayl a) e))}
{csetq i (lambda (x) (sys 3 a x e))}
{csetq o (lambda (x) (sys 4 a x e))}
{csetq a (array 32766 4)}
{csetq e 3}
{csetq g (lambda (x) (sys 19 x 3 e))}
{csetq l "ldostape -c1 /dev/rmt0"}
{csetq z
(lambda (ind val)
	(prog ()
	 loop (a (setq ind (add1 ind)) (setq val (add1 val)))
	      (go loop)
))}
