
Test Class Regex
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) match(123First123456789)? YES
\0:	123First123456789	[123First123456789(0:17)]
\1:	123	[123First123456789(0:3)]
\2:	First	[123First123456789(3:5)]
\3:	123456789	[123First123456789(8:9)]
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) search(123First123456789)? YES
\0:	123First123456789	[123First123456789(0:17)]
\1:	123	[123First123456789(0:3)]
\2:	First	[123First123456789(3:5)]
\3:	123456789	[123First123456789(8:9)]
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) search(123First123456789,16,-16)? YES
\0:	t123456789	[123First123456789(7:10)]
\1:		[123First123456789(7:0)]
\2:	t	[123First123456789(7:1)]
\3:	123456789	[123First123456789(8:9)]
---
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) match(123First123456789)? YES
\0:	123First123456789	[123First123456789(0:17)]
\1:	123	[123First123456789(0:3)]
\2:	First	[123First123456789(3:5)]
\3:	123456789	[123First123456789(8:9)]
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) search(123First123456789)? YES
\0:	123First123456789	[123First123456789(0:17)]
\1:	123	[123First123456789(0:3)]
\2:	First	[123First123456789(3:5)]
\3:	123456789	[123First123456789(8:9)]
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) search(123First123456789,16,-16)? YES
\0:	t123456789	[123First123456789(7:10)]
\1:		[123First123456789(7:0)]
\2:	t	[123First123456789(7:1)]
\3:	123456789	[123First123456789(8:9)]
---
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) match(123First123456789)? YES
\0:	123First123456789	[123First123456789(0:17)]
\1:	123	[123First123456789(0:3)]
\2:	First	[123First123456789(3:5)]
\3:	123456789	[123First123456789(8:9)]
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) search(123First123456789)? YES
\0:	123First123456789	[123First123456789(0:17)]
\1:	123	[123First123456789(0:3)]
\2:	First	[123First123456789(3:5)]
\3:	123456789	[123First123456789(8:9)]
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) search(123First123456789,16,-16)? YES
\0:	t123456789	[123First123456789(7:10)]
\1:		[123First123456789(7:0)]
\2:	t	[123First123456789(7:1)]
\3:	123456789	[123First123456789(8:9)]
---
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) match(123First123456789)? YES
\0:	123First123456789	[123First123456789(0:17)]
\1:	123	[123First123456789(0:3)]
\2:	First	[123First123456789(3:5)]
\3:	123456789	[123First123456789(8:9)]
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) search(123First123456789)? YES
\0:	123First123456789	[123First123456789(0:17)]
\1:	123	[123First123456789(0:3)]
\2:	First	[123First123456789(3:5)]
\3:	123456789	[123First123456789(8:9)]
\([^a-zA-Z]*\)\([a-zA-Z]+\)\([^a-zA-Z]*\) search(123First123456789,16,-16)? YES
\0:	t123456789	[123First123456789(7:10)]
\1:		[123First123456789(7:0)]
\2:	t	[123First123456789(7:1)]
\3:	123456789	[123First123456789(8:9)]
---
Enter PATTERN: Enter STRING: \(abc\) match(abc###abc)? YES
\0:	abc	[abc###abc(0:3)]
\1:	abc	[abc###abc(0:3)]
\(abc\) search(abc###abc)? YES
\0:	abc	[abc###abc(0:3)]
\1:	abc	[abc###abc(0:3)]
\(abc\) search(abc###abc,8,-8)? YES
\0:	abc	[abc###abc(6:3)]
\1:	abc	[abc###abc(6:3)]
---
Enter STRING: \(abc\) match(###abc###)? NO
\(abc\) search(###abc###)? YES
\0:	abc	[###abc###(3:3)]
\1:	abc	[###abc###(3:3)]
\(abc\) search(###abc###,8,-8)? YES
\0:	abc	[###abc###(3:3)]
\1:	abc	[###abc###(3:3)]
---
Enter STRING: \(abc\) match(abc######)? YES
\0:	abc	[abc######(0:3)]
\1:	abc	[abc######(0:3)]
\(abc\) search(abc######)? YES
\0:	abc	[abc######(0:3)]
\1:	abc	[abc######(0:3)]
\(abc\) search(abc######,8,-8)? YES
\0:	abc	[abc######(0:3)]
\1:	abc	[abc######(0:3)]
---
Enter STRING: \(abc\) match(###)? NO
\(abc\) search(###)? NO
\(abc\) search(###,2,-2)? NO
---
Enter STRING: Enter PATTERN: Enter STRING: \(#*\)\(abc\)\(#*\) match(abc###abc)? YES
\0:	abc###	[abc###abc(0:6)]
\1:		[abc###abc(0:0)]
\2:	abc	[abc###abc(0:3)]
\3:	###	[abc###abc(3:3)]
\(#*\)\(abc\)\(#*\) search(abc###abc)? YES
\0:	abc###	[abc###abc(0:6)]
\1:		[abc###abc(0:0)]
\2:	abc	[abc###abc(0:3)]
\3:	###	[abc###abc(3:3)]
\(#*\)\(abc\)\(#*\) search(abc###abc,8,-8)? YES
\0:	abc	[abc###abc(6:3)]
\1:		[abc###abc(6:0)]
\2:	abc	[abc###abc(6:3)]
\3:		[abc###abc(9:0)]
---
Enter STRING: \(#*\)\(abc\)\(#*\) match(###abc###)? YES
\0:	###abc###	[###abc###(0:9)]
\1:	###	[###abc###(0:3)]
\2:	abc	[###abc###(3:3)]
\3:	###	[###abc###(6:3)]
\(#*\)\(abc\)\(#*\) search(###abc###)? YES
\0:	###abc###	[###abc###(0:9)]
\1:	###	[###abc###(0:3)]
\2:	abc	[###abc###(3:3)]
\3:	###	[###abc###(6:3)]
\(#*\)\(abc\)\(#*\) search(###abc###,8,-8)? YES
\0:	abc###	[###abc###(3:6)]
\1:		[###abc###(3:0)]
\2:	abc	[###abc###(3:3)]
\3:	###	[###abc###(6:3)]
---
Enter STRING: \(#*\)\(abc\)\(#*\) match(abc######)? YES
\0:	abc######	[abc######(0:9)]
\1:		[abc######(0:0)]
\2:	abc	[abc######(0:3)]
\3:	######	[abc######(3:6)]
\(#*\)\(abc\)\(#*\) search(abc######)? YES
\0:	abc######	[abc######(0:9)]
\1:		[abc######(0:0)]
\2:	abc	[abc######(0:3)]
\3:	######	[abc######(3:6)]
\(#*\)\(abc\)\(#*\) search(abc######,8,-8)? YES
\0:	abc######	[abc######(0:9)]
\1:		[abc######(0:0)]
\2:	abc	[abc######(0:3)]
\3:	######	[abc######(3:6)]
---
Enter STRING: \(#*\)\(abc\)\(#*\) match(###)? NO
\(#*\)\(abc\)\(#*\) search(###)? NO
\(#*\)\(abc\)\(#*\) search(###,2,-2)? NO
---
Enter STRING: \(#*\)\(abc\)\(#*\) match(...abc###abc...)? NO
\(#*\)\(abc\)\(#*\) search(...abc###abc...)? YES
\0:	abc###	[...abc###abc...(3:6)]
\1:		[...abc###abc...(3:0)]
\2:	abc	[...abc###abc...(3:3)]
\3:	###	[...abc###abc...(6:3)]
\(#*\)\(abc\)\(#*\) search(...abc###abc...,14,-14)? YES
\0:	abc	[...abc###abc...(9:3)]
\1:		[...abc###abc...(9:0)]
\2:	abc	[...abc###abc...(9:3)]
\3:		[...abc###abc...(12:0)]
---
Enter STRING: \(#*\)\(abc\)\(#*\) match(...###abc###...)? NO
\(#*\)\(abc\)\(#*\) search(...###abc###...)? YES
\0:	###abc###	[...###abc###...(3:9)]
\1:	###	[...###abc###...(3:3)]
\2:	abc	[...###abc###...(6:3)]
\3:	###	[...###abc###...(9:3)]
\(#*\)\(abc\)\(#*\) search(...###abc###...,14,-14)? YES
\0:	abc###	[...###abc###...(6:6)]
\1:		[...###abc###...(6:0)]
\2:	abc	[...###abc###...(6:3)]
\3:	###	[...###abc###...(9:3)]
---
Enter STRING: \(#*\)\(abc\)\(#*\) match(...abc######...)? NO
\(#*\)\(abc\)\(#*\) search(...abc######...)? YES
\0:	abc######	[...abc######...(3:9)]
\1:		[...abc######...(3:0)]
\2:	abc	[...abc######...(3:3)]
\3:	######	[...abc######...(6:6)]
\(#*\)\(abc\)\(#*\) search(...abc######...,14,-14)? YES
\0:	abc######	[...abc######...(3:9)]
\1:		[...abc######...(3:0)]
\2:	abc	[...abc######...(3:3)]
\3:	######	[...abc######...(6:6)]
---
Enter STRING: \(#*\)\(abc\)\(#*\) match(...###...)? NO
\(#*\)\(abc\)\(#*\) search(...###...)? NO
\(#*\)\(abc\)\(#*\) search(...###...,8,-8)? NO
---
Enter STRING: Enter PATTERN: Enter STRING: ^\(#*\)\(abc\)\(#*\)$ match(abc###abc)? NO
^\(#*\)\(abc\)\(#*\)$ search(abc###abc)? NO
^\(#*\)\(abc\)\(#*\)$ search(abc###abc,8,-8)? NO
---
Enter STRING: ^\(#*\)\(abc\)\(#*\)$ match(###abc###)? YES
\0:	###abc###	[###abc###(0:9)]
\1:	###	[###abc###(0:3)]
\2:	abc	[###abc###(3:3)]
\3:	###	[###abc###(6:3)]
^\(#*\)\(abc\)\(#*\)$ search(###abc###)? YES
\0:	###abc###	[###abc###(0:9)]
\1:	###	[###abc###(0:3)]
\2:	abc	[###abc###(3:3)]
\3:	###	[###abc###(6:3)]
^\(#*\)\(abc\)\(#*\)$ search(###abc###,8,-8)? YES
\0:	###abc###	[###abc###(0:9)]
\1:	###	[###abc###(0:3)]
\2:	abc	[###abc###(3:3)]
\3:	###	[###abc###(6:3)]
---
Enter STRING: ^\(#*\)\(abc\)\(#*\)$ match(abc######)? YES
\0:	abc######	[abc######(0:9)]
\1:		[abc######(0:0)]
\2:	abc	[abc######(0:3)]
\3:	######	[abc######(3:6)]
^\(#*\)\(abc\)\(#*\)$ search(abc######)? YES
\0:	abc######	[abc######(0:9)]
\1:		[abc######(0:0)]
\2:	abc	[abc######(0:3)]
\3:	######	[abc######(3:6)]
^\(#*\)\(abc\)\(#*\)$ search(abc######,8,-8)? YES
\0:	abc######	[abc######(0:9)]
\1:		[abc######(0:0)]
\2:	abc	[abc######(0:3)]
\3:	######	[abc######(3:6)]
---
Enter STRING: ^\(#*\)\(abc\)\(#*\)$ match(###)? NO
^\(#*\)\(abc\)\(#*\)$ search(###)? NO
^\(#*\)\(abc\)\(#*\)$ search(###,2,-2)? NO
---
Enter STRING: ^\(#*\)\(abc\)\(#*\)$ match(...abc###abc...)? NO
^\(#*\)\(abc\)\(#*\)$ search(...abc###abc...)? NO
^\(#*\)\(abc\)\(#*\)$ search(...abc###abc...,14,-14)? NO
---
Enter STRING: ^\(#*\)\(abc\)\(#*\)$ match(...###abc###...)? NO
^\(#*\)\(abc\)\(#*\)$ search(...###abc###...)? NO
^\(#*\)\(abc\)\(#*\)$ search(...###abc###...,14,-14)? NO
---
Enter STRING: ^\(#*\)\(abc\)\(#*\)$ match(...abc######...)? NO
^\(#*\)\(abc\)\(#*\)$ search(...abc######...)? NO
^\(#*\)\(abc\)\(#*\)$ search(...abc######...,14,-14)? NO
---
Enter STRING: ^\(#*\)\(abc\)\(#*\)$ match(...###...)? NO
^\(#*\)\(abc\)\(#*\)$ search(...###...)? NO
^\(#*\)\(abc\)\(#*\)$ search(...###...,8,-8)? NO
---
Enter STRING: 