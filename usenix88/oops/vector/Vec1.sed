s/^class \([A-Z][a-z]*\)Vec : public Vector {/PROLOGUE(\1)/p
s#^[	 ][	 ]*void[	 ][	 ]*/\*\([A-Za-z]*\)::\*/lengthErr(const \([A-Za-z]*\)&);.*#TYPE1_lengthErr_TYPE2(\1,\2)#p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Vec([a-zA-Z ]*len[^,]*);.*/TYPEVec_CTOR_I(\1)/p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Vec(.*len,.*from,.*by.*);.*/TYPEVec_CTOR_I_TYPE_TYPE(\1)/p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Vec(const [a-z]*\*,.*len);.*/TYPEVec_CTOR_TYPEPTR_I(\1)/p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Vec(const \1Vec&);.*/TYPEVec_CTOR_TYPEVec(\1)/p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Vec(const \1Slice&);.*/TYPEVec_CTOR_TYPESlice(\1)/p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Slice(\1Vec&[a-z ]*,.*pos,.*lgt,.*stride.*);.*/TYPESlice_CTOR_TYPEVec_I_I_I(\1)/p
s#^[	 ][	 ]*void[	 ][	 ]*/\*\([A-Z][a-z]*\)\([A-Z][a-z]*\)::\*/operator=(const \1\([A-Z][a-z]*\)&);.*#TYPE\2_ASN_TYPE\3(\1)#p
s#^[	 ][	 ]*void[	 ][	 ]*/\*\([A-Z][a-z]*\)\([A-Z][a-z]*\)::\*/operator=([a-z]*);.*#TYPE\2_ASN_TYPE(\1)#p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Slice(const \1Pick&);.*/TYPESlice_CTOR_TYPEPick(\1)/p
s/^[	 ][	 ]*\([A-Z][a-z]*\)Slice(const \1Slct&);.*/TYPESlice_CTOR_TYPESlct(\1)/p
