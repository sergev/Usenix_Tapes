/*
 * Implementation of File class.
 */

#include <string.h>
#include <InterViews/defs.h>
#include <InterViews/Graphic/file.h>

static const int FROM_BEGINNING = 0;
static const int FROM_CURPOS = 1;
static const int FROM_END = 2;

extern int unlink(const char*);

File::File (char* filename) {
    name = new char [strlen(filename) + 1];
    strcpy(name, filename);
    fd = fopen(name, "r+");
    if (fd == NULL) {
	fd = fopen(name, "w+");
	if (fd == NULL) {
	    fflush(stdout);
	    fprintf(stderr, "couldn't open %s\n", name);
	    exit(2);
	}
    }
}

File::~File () {
    if (fd != NULL) {
	(void) fclose(fd);
    }
    delete name;
}

char* File::GetName () {
    return name;
}

boolean File::Exists () {
    return fopen(name, "r") != NULL;
}

boolean File::Exists (char* filename) {
    return fopen(filename, "r") != NULL;
}

boolean File::Read (short& i) {
    int tmp;

    boolean ok = fread((char*) &tmp, sizeof(int), 1, fd) == 1;
    i = short(tmp);
    return ok;
}

boolean File::Read (int& i)  {
    return fread((char*) &i, sizeof(int), 1, fd) == 1; 
}

boolean File::Read (float& f) {
    double tmp;
    
    boolean ok = fread((char*) &tmp, sizeof(double), 1, fd) == 1; 
    f = float(tmp);
    return ok;
}

boolean File::Read (char& c) {
    int tmp;

    boolean ok = fread((char*) &tmp, sizeof(int), 1, fd) == 1;
    c = char(tmp);
    return ok;
}

boolean File::Read (short* i, int count) {
    return fread((char*) i, sizeof(short), count, fd) == count; 
}

boolean File::Read (int* i, int count) {
    return fread((char*) i, sizeof(int), count, fd) == count; 
}

boolean File::Read (long* i, int count) {
    return fread((char*) i, sizeof(long), count, fd) == count; 
}

boolean File::Read (float* f, int count) {
    return fread((char*) f, sizeof(float), count, fd) == count; 
}

boolean File::Read (char* string) {
    // will read up to a NULL character or count characters
    // assumes that there is room to read all count characters
    // string will be NULL terminated iff NULL was read
    char c;

    while (fread((char*) &c, sizeof(char), 1, fd) == 1) {
	*string = c;
	string++;
	if ( c == NULL ) {
	    return true;
	}
    }
    return false;
}

boolean File::Read (char* string, int count) {
    return fread((char*) string, sizeof(char), count, fd) == count; 
}

boolean File::Write (int i) {
    return fwrite((char*) &i, sizeof(int), 1, fd) == 1; 
}

boolean File::Write (float f) {
    double tmp = double(f);
    return fwrite((char*) &tmp, sizeof(double), 1, fd) == 1; 
}

boolean File::Write (short* i, int count) {
    return fwrite((char*) i, sizeof(short), count, fd) == count;
}

boolean File::Write (int* i, int count) {
    return fwrite((char*) i, sizeof(int), count, fd) == count; 
}

boolean File::Write (long* i, int count) {
    return fwrite((char*) i, sizeof(long), count, fd) == count; 
}

boolean File::Write (float* f, int count) {
    return fwrite((char*) f, sizeof(float), count, fd) == count; 
}

boolean File::Write (char* string) {
    // write up to and including NULL character
    // beware of non-terminated strings!

    int len = strlen( string ) + 1;
    return fwrite((char*) string, sizeof(char), len, fd) == len;
}

boolean File::Write (char* string, int count) {
    return fwrite((char*) string, sizeof(char), count, fd) == count; 
}


boolean File::SeekTo (long offset) {
    return fseek(fd, offset, FROM_BEGINNING) >= 0; 
}	

boolean File::SeekToBegin () {
    return fseek(fd, 0, FROM_BEGINNING) >= 0; 
}

boolean File::SeekToEnd ()  {
    return fseek(fd, 0, FROM_END) >= 0; 
}

long File::CurOffset () {
    return ftell(fd); 
}

boolean File::IsEmpty () {
    int dummy;

    if (Read(dummy)) {
	rewind(fd);
	return false;
    } else {
	return true;
    }
}

boolean File::Erase () {
    return 
	unlink(name) == 0 && fclose(fd) != EOF && 
	(fd = fopen(name, "w+")) != NULL;
}
