Test Vector classes
IntVec I(3,1,2):          1         3         5
DoubleVec A(10,1.0):             1            2            3            4            5            6
	            7            8            9           10
DoubleVec B(10,10.0,-1.0):            10            9            8            7            6            5
	            4            3            2            1
C = A; C[A<=5.] = DoubleVec(5,-1.,0.):            -1           -1           -1           -1           -1            6
	            7            8            9           10
C = A; C[A<=3.] = A[I]:             2            4            6            4            5            6
	            7            8            9           10
C = A; C[A<=5.] = B[A<=5.]:            10            9            8            7            6            6
	            7            8            9           10
C = A; C[A<=5.] = A(0,5,1):             1            2            3            4            5            6
	            7            8            9           10
C = A; C[A<=5.] = 0.:             0            0            0            0            0            6
	            7            8            9           10
C = A[A<=5.] + B[B<=5.]:             6            6            6            6            6
abs(DoubleVec(10,-5)):             5            4            3            2            1            0
	            1            2            3            4
atan2(DoubleVec(10,1),DoubleVec(10,10,-1)):       0.09967       0.2187       0.3588       0.5191       0.6947       0.8761
	        1.052        1.212        1.352        1.471
pow(DoubleVec(10,1),DoubleVec(10,2,0)):             1            4            9           16           25           36
	           49           64           81          100
cumsum(DoubleVec(10,1)):             1            3            6           10           15           21
	           28           36           45           55
delta(cumsum(DoubleVec(10,1))):             1            2            3            4            5            6
	            7            8            9           10
dot(A,B): 220
max(A): 9
min(A): 0
prod(A): 3.6288e+06
reverse(A):            10            9            8            7            6            5
	            4            3            2            1
sqrt(A):             1        1.414        1.732            2        2.236        2.449
	        2.646        2.828            3        3.162
sum(A): 55
