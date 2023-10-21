Testing Heap
OrderedCltn[
100
70
76
22
101
54
60
2
5
601]

Test asHeap function
OrderedCltn as Heap: 
Heap[
2
601
76
5
70
54
60
22
100
101]
Test isEqual function
SUCCESS
SUCCESS
SUCCESS
c = Heap[
(1 @ 1)
(1 @ 3)
(1 @ 3)
(1 @ 2)]

b = Heap[
(1 @ 1)
(1 @ 3)
(1 @ 3)
(1 @ 2)]

b.first(): (1 @ 1)
b.last(): (1 @ 3)
b = Heap[
(1 @ 1)
(4 @ 5)
(1 @ 3)
(1 @ 2)
(1 @ 3)]

remove min from b (1 @ 1)
remove max from b (4 @ 5)
b= Heap[
(0 @ 1)
(9 @ 8)
(5 @ 6)
(0 @ 1)
(1 @ 1)
(1 @ 2)
(1 @ 2)
(1 @ 4)
(3 @ 2)
(1 @ 3)
(1 @ 2)
(1 @ 3)]


occurrencesOf((1,2)): 3
b.sort(): OrderedCltn[
(0 @ 1)
(0 @ 1)
(1 @ 1)
(1 @ 2)
(1 @ 2)
(1 @ 2)
(3 @ 2)
(1 @ 3)
(1 @ 3)
(1 @ 4)
(5 @ 6)
(9 @ 8)]

b.asSet(): Set[
(1 @ 1)
(1 @ 4)
(1 @ 3)
(3 @ 2)
(9 @ 8)
(0 @ 1)
(1 @ 2)
(5 @ 6)]


Testing remove(Object&)
Heap[
5
80
65
14
8
12
10
57
59
37
39
50
20
45
34
27
30
18
31
17
36
28
38
30
45
16
15
32
13
25
15]


Heap[
5
80
65
14
8
12
13
57
59
37
39
50
20
45
34
27
30
18
31
17
36
28
38
30
45
16
15
32
15
25]


Heap[
5
59
65
14
8
12
13
57
31
37
39
50
20
45
34
27
30
18
25
17
36
28
38
30
45
16
15
32
15]


Heap[
5
59
65
16
8
12
13
57
31
37
39
50
20
45
34
27
30
18
25
17
36
28
38
30
45
16
15
32
15]


Heap[
5
59
65
15
8
12
13
57
31
37
39
50
20
45
34
27
30
16
25
17
36
28
38
30
45
16
15
32]


