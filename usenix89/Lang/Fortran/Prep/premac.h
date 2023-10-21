c Some standard macros for prep.

c logical stuff
: ==	.eq. ;
: >=	.ge. ;
: >	.gt. ;
: <	.lt. ;
: <=	.le. ;
: !=	.ne. ;
: <>	.ne. ;
: !	.not. ;
: |	.or. ;
: &	.and. ;
: TRUE	.true. ;
: FALSE	.false. ;
: ^	** ;

#if DOUBLE	; double precision defs
: real		doubleprecision ;
: alog		dlog ;
: amax1		dmax1 ;
: amin1		dmin1 ;
: abs		dabs ;
: sin		dsin ;
: cos		dcos ;
: 0.0		0.d0 ;
: .e		.d;
#endif
