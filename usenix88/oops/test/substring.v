
Test Class SubString
void SubString::operator=(const String&): 0xxx456789
void SubString::operator=(const SubString&): 0xxx456789
void SubString::operator=(const char*): 0xxx456789
0123456789
void replace(const char* st, unsigned ln):
0*123456789
*0123456789
0123456789*
023456789
123456789
012345678
0123456789
01023456789
00123456789
123456789
01123456789
bool SubString::operator<(const String&): 1
bool SubString::operator<(const SubString& ss): 1
bool SubString::operator<(const char* cs): 1
friend bool operator<(const char* cs, const SubString& ss): 1
String SubString::operator&(const String&): 00123456789
String SubString::operator&(const SubString&): 0189
String SubString::operator&(const char*): 01*
friend String operator&(const char*, const SubString&): *01
