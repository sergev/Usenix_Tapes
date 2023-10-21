#define DEFINE_CLASS(classname,basename,version,identification,initor1,initor2)\
overload classname\
_reader;\
static void classname\
_reader(istream& strm, Object& where) { new classname(strm,*(classname*)&where); }\
static void classname\
_reader(fileDescTy& fd, Object& where) { new classname(fd,*(classname*)&where); }\
const Class class_\
classname = Class( class_\
basename, "\
classname\
", version, identification, sizeof(classname),classname\
_reader, classname\
_reader, initor1, initor2);\
const Class* classname::isA()	{ return &class_\
classname; }
