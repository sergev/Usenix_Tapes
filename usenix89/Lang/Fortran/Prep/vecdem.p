c Demo to demonstrate some PREP facilities.  This program is a demo
c only and will not compile without a lot of variable definitions.

#include "vecdem.h"

        subroutine w_accel_l(psi, lin_fac, source, omega)
        include "ellipdim"

        if (w_bypass) return
        w_error = FALSE

c Set up the basis consisting of past iterates
[	basis(#,#,1) = psi(#,#)
	basis(#,#,2) = psi(#,#) - psi_alt(#,#,1)
	basis(#,#,3) = psi(#,#) - 2*psi_alt(#,#,1) + psi_alt(#,#,2)
	basis(#,#,4) = 1      ]
	PERIODIC( basis1 )
	PERIODIC( basis2 )
	PERIODIC( basis3 )
	PERIODIC( basis4 )

c Calculate the matrix and the source vector
        do i = 1, w_dim
	ii = i
	do j = i, w_dim
	jj = j
           call make_mat_l(psi, lin_fac, source, omega, i, j)
        end_do
	end_do

	do i = 1, w_dim
           w_source(i) = 0
           w_source(i) = source(#,#)*basis(#,#,i) + w_source(i)
        end_do

c invert the symmetric matrix
        call linsys(w_matrix, w_dim, w_dim, w_source, w_coeff, ising, lfirst,
     *              lprint, work)
        if (ising == 1) then
           write(*,*) ' WARNING:  W_matrix is singular '
           w_error = TRUE
           return
        endif

c calculate the improved solution
        psi(#,#) = 0
        do i = 1, w_dim
           psi(#,#) = psi(#,#) + w_coeff(i)*basis(#,#,i)
        end_do

c output section for error checking
        do i = 1, w_dim
           write(*,100) i, .5*w_matrix(i,i) - w_source(i),
     *                  i, w_coeff(i)
        end_do

	do_limits = { w_dim }
        action = 0
        do i = 1, w_dim
           action = action + w_matrix(i,#)*w_coeff(i)*w_coeff(#)
        end_do
        action = action/2
        action = action - w_source(#)*w_coeff(#)
        write(*,*) ' new action = ',action

        return


100     format(' action(',i1')= ',g16.9,'    w_coeff(',i1,')= ', g16.9)

        end
