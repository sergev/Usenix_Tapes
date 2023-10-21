subroutine dgelg(r,a,m,n,eps,ier)
#     ..................................................................
#
#        subroutine dgelg
#
#        purpose
#           to solve a general system of simultaneous linear equations.
#
#        usage
#           call dgelg(r,a,m,n,eps,ier)
#
#        description of parameters
#           r      - double precision m by n right hand side matrix
#                    (destroyed). on return r contains the solutions
#                    of the equations.
#           a      - double precision m by m coefficient matrix
#                    (destroyed).
#           m      - the number of equations in the system.
#           n      - the number of right hand side vectors.
#           eps    - single precision input constant which is used as
#                    relative tolerance for test on loss of
#                    significance.
#           ier    - resulting error parameter coded as follows
#                    ier=0  - no error,
#                    ier=-1 - no result because of m less than 1 or
#                             pivot element at any elimination step
#                             equal to 0,
#                    ier=k  - warning due to possible loss of signifi-
#                             cance indicated at elimination step k+1,
#                             where pivot element was less than or
#                             equal to the internal tolerance eps times
#                             absolutely greatest element of matrix a.
#
#        remarks
#           input matrices r and a are assumed to be stored columnwise
#           in m*n resp. m*m successive storage locations. on return
#           solution matrix r is stored columnwise too.
#           the procedure gives results if the number of equations m is
#           greater than 0 and pivot elements at all elimination steps
#           are different from 0. however warning ier=k - if given -
#           indicates possible loss of significance. in case of a well
#           scaled matrix a and appropriate tolerance eps, ier=k may be
#           interpreted that matrix a has the rank k. no warning is
#           given in case m=1.
#
#        subroutines and function subprograms required
#           none
#
#        method
#           solution is done by means of gauss-elimination with
#           complete pivoting.
#
#     ..................................................................
#
#
#
implicit real*8(a-h,o-z)
dimension a(1),r(1)
if (m>0) {
#
#     search for greatest element in matrix a
	ier = 0
	piv = 0.d0
	mm = m*m
	nm = n*m
	do l = 1,mm {
		tb = abs(a(l))
		if (tb>piv) {
			piv = tb
			i = l
			}
		}
	tol = eps*piv
#     a(i) is pivot element. piv contains the absolute value of a(i).
#
#
#     start elimination loop
	lst = 1
	do k = 1,m {
#
#     test on singularity
		if (piv<=0)
			go to 10
		if (ier==0)
			if (piv<=tol)
				ier = k-1
		pivi = 1.d0/a(i)
		j = (i-1)/m
		i = i-j*m-k
		j = j+1-k
#     i+k is row-index, j+k column-index of pivot element
#
#     pivot row reduction and row interchange in right hand side r
		do l = k,nm,m {
			ll = l+i
			tb = pivi*r(ll)
			r(ll) = r(l)
			r(l) = tb
			}
#
#     is elimination terminated
		if (k>=m)
			break 1
#
#     column interchange in matrix a
		lend = lst+m-k
		if (j>0) {
			ii = j*m
			do l = lst,lend {
				tb = a(l)
				ll = l+ii
				a(l) = a(ll)
				a(ll) = tb
				}
			}
#
#     row interchange and pivot row reduction in matrix a
		do l = lst,mm,m {
			ll = l+i
			tb = pivi*a(ll)
			a(ll) = a(l)
			a(l) = tb
			}
#
#     save column interchange information
		a(lst) = j
#
#     element reduction and next pivot search
		piv = 0.d0
		lst = lst+1
		j = 0
		do ii = lst,lend {
			pivi = -a(ii)
			ist = ii+m
			j = j+1
			do l = ist,mm,m {
				ll = l-j
				a(l) = a(l)+pivi*a(ll)
				tb = abs(a(l))
				if (tb>piv) {
					piv = tb
					i = l
					}
				}
			do l = k,nm,m {
				ll = l+j
				r(ll) = r(ll)+pivi*r(l)
				}
			}
		lst = lst+m
		}
#     end of elimination loop
#
#
#     back substitution and back interchange
	if (m>=1) {
#     end of elimination loop
#
#
#     back substitution and back interchange
		if (m!=1) {
			ist = mm+m
			lst = m+1
			do i = 2,m {
				ii = lst-i
				ist = ist-lst
				l = ist-m
				l = a(l)+.5d0
				do j = ii,nm,m {
					tb = r(j)
					ll = j
					do k = ist,mm,m {
						ll = ll+1
						tb = tb-a(k)*r(ll)
						}
					k = j+l
					r(j) = r(k)
					r(k) = tb
					}
				}
			}
		return
		}
	}
#
#
#     error return
10  ier = -1
return
end



