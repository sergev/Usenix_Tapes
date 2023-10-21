s#^[	 ][	 ]*void[	 ][	 ]*/\*\([A-Za-z]*\)::\*/lengthErr(const \([A-Za-z]*\)&);.*#TYPE1_lengthErr_TYPE2(\1,\2)#p
s/^friend[	 ][	 ]*\([A-Z][a-z]*\)Vec[	 ][	 ]*operator\([&|^]\)(const \1Vec&,const \1Vec&);.*/FRIEND_BitVec_OP_BitVec__BitVec(\1,\2)/p
s/^friend[	 ][	 ]*void[	 ][	 ]*operator\([&|^]=\)(\([A-Z][a-z]*\)Vec&,const \2Vec&);.*/FRIEND_BitVec_ASNOP_BitVec(\2,\1)/p
s/^friend[	 ][	 ]*\([A-Z][a-z]*\)Vec[	 ][	 ]*operator\([&|^]\)(const \1Slice&,const \1Slice&);.*/FRIEND_BitSlice_OP_BitSlice__BitVec(\1,\2)/p
s/^friend[	 ][	 ]*void[	 ][	 ]*operator\([&|^]=\)(\([A-Z][a-z]*\)Slice&,const \2Slice&);.*/FRIEND_BitSlice_ASNOP_BitSlice(\2,\1)/p
