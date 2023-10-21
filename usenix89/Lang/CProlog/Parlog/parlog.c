
% I don't think it's necessary to define the arith ops as built-in
% unless you use the expansion of expressions.

% You could simply use this to declare all '$' routines as system:
built_in( T ) :-
	functor(T, F, A),
	name(F, ["$"|L]).

% Otherwise:

built_in(( '$+'(A, X) )).
built_in(( '$-'(A, X) )).
built_in(( '$\'(A, X) )).
built_in(( $exp(A, X) )).
built_in(( $log(A, X) )).
built_in(( $log10(A, X) )).
built_in(( $sqrt(A, X) )).
built_in(( $sin(A, X) )).
built_in(( $cos(A, X) )).
built_in(( $tan(A, X) )).
built_in(( $asin(A, X) )).
built_in(( $acos(A, X) )).
built_in(( $atan(A, X) )).
built_in(( $floor(A, X) )).
built_in(( '$+'(A, B, X) )).
built_in(( '$-'(A, B, X) )).
built_in(( '$*'(A, B, X) )).
built_in(( '$/'(A, B, X) )).
built_in(( $mod(A, B, X) )).
built_in(( '$/\'(A, B, X) )).
built_in(( '$\/'(A, B, X) )).
built_in(( '$<<'(A, B, X) )).
built_in(( '$>>'(A, B, X) )).
built_in(( '$//'(A, B, X) )).
