s/^friend[	 ][	 ]*\([A-Z][a-z]*\)Vec[	 ][	 ]*operator\([-!~]\)(const \1Slice&);.*/FRIEND_OP_TYPESlice__TYPEVec(\1,\2)/p
s/^friend[	 ][	 ]*\([A-Z][a-z]*\)Vec[	 ][	 ]*operator\([+-][+-]\)(\1Slice&);.*/FRIEND_INCDECOP_TYPESlice__TYPEVec(\1,\2)/p
s/^friend[	 ][	 ]*\([A-Z][a-z]*\)Vec[	 ][	 ]*operator\([-+*/%&|^]\)(const \1Slice&,const \1Slice&);.*/FRIEND_TYPESlice_OP_TYPESlice__TYPEVec(\1,\2)/p
s/^friend[	 ][	 ]*\([A-Z][a-z]*\)Vec[	 ][	 ]*operator\([-+*/%&|^]\)(const \1Slice&,[a-z]*);.*/FRIEND_TYPESlice_OP_TYPE__TYPEVec(\1,\2)/p
s/^friend[	 ][	 ]*\([A-Z][a-z]*\)Vec[	 ][	 ]*operator\([-+*/%&|^]\)([a-z]*[A-Za-z ]*,const \1Slice&[A-Za-z ]*);.*/FRIEND_TYPE_OP_TYPESlice__TYPEVec(\1,\2)/p
s/^friend[	 ]*BitVec[	 ]*operator\([<>!=]=*\)(const \([A-Z][a-z]*\)Slice&[A-Za-z ]*,const \2Slice&[A-Za-z ]*);.*/FRIEND_TYPESlice_OP_TYPESlice__BitVec(\2,\1)/p
s/^friend[	 ]*BitVec[	 ]*operator\([<>!=]=*\)(const \([A-Z][a-z]*\)Slice&,[a-z]*);.*/FRIEND_TYPESlice_OP_TYPE__BitVec(\2,\1)/p
s/^friend[	 ]*BitVec[	 ]*operator\([<>!=]=*\)([a-z]*[A-Za-z ]*,const \([A-Z][a-z]*\)Slice&[A-Za-z ]*);.*/FRIEND_TYPE_OP_TYPESlice__BitVec(\2,\1)/p
s/^[	 ][	 ]*void[	 ][	 ]*operator\([-+*/%&|^]=\)(const \([A-Z][a-z]*\)Slice&);.*/TYPESlice_ASNOP_TYPESlice(\2,\1)/p
s#^[	 ][	 ]*void[	 ][	 ]*/\*\([A-Z][a-z]*\)Slice::\*/operator\([-+*/%&|^]=\)([a-z]*);.*#TYPESlice_ASNOP_TYPE(\1,\2)#p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Vec[	 ][	 ]*apply(mathFunTy);.*/TYPESlice_APPLY_FUN__TYPEVec(\1)/p
s/^friend[	 ][	 ]*[A-Za-z]*[	 ][	 ]*\([a-z]*\)(const \([A-Z][a-z]*\)Slice&[A-Za-z ]*);.*/FRIEND_\1_TYPESlice(\2,\1)/p
s/^friend[	 ][	 ]*[A-Za-z]*[	 ][	 ]*\([a-z2]*\)(const \([A-Z][a-z]*\)Slice&,const \2Slice&);.*/FRIEND_\1_TYPESlice_TYPESlice(\2,\1)/p
