
Test Class String
String::String(char& c, unsigned l=1, unsigned extra): x
xxxxxxxxxxxxxxxxxxxxxxxxx
String::String(): 0 15
String::String(unsigned storage): 0 100
String::String(const char*): 0123456789 25
String::String(const char*, unsigned extra): 0123456789 20
String::String(const String&): 0123456789
String::String(const String&, unsigned extra): 0123456789
String::String(const SubString&):
SubString String::operator()(unsigned pos, unsigned lgt): 123 18
String::String(const SubString&,  unsigned extra): 123 13
operator String::constCharPtTy(): 456789
SubString String::operator()(const Range& r): 123
char& String::operator[](unsigned i): void String::operator=(const char*): 9123456789
unsigned String::length(): 0
unsigned String::size(): 0
unsigned String::capacity(): 13
unsigned String::reSize(unsigned new_capacity): 0
void String::operator=(const String&): 0123456789
0123456789
void String::operator=(const SubString&): 123456789
012345678
123456789
bool String::operator==(const String& s): 1
bool String::operator<(const SubString ss): 1
bool operator<(const char* cs): 1
friend bool operator<(const char* cs, const String& s): 1
String String::operator&(const String& s): 01234567890123456789
String String::operator&(const SubString& ss): 0123456789123456789
String String::operator&(const char* cs): 0123456789xxx
friend String operator&(const char* cs, const String& s): xxx0123456789
friend String operator&(const char*, const SubString&): xxx12345678
String& String::operator&=(const String&): 01234567890123456789
String& String::operator&=(const SubString&): 0123456789123456789
0123456789123456789
String& String::operator&=(const char* cs): 0123456789xxx0x
char& String::at(unsigned i): 9123456789
void String::toLower(): abcabc
void String::toUpper(): ABCABC
