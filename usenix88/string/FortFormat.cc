/* Copyright (c) 1987 Peter A. Buhr */
	
#include <stream.h>
#include "string.h"

int search(string list[], int SizeOfList, string key) {
	int pos;

	for (int i = 0; ; i += 1) {
	if (i == SizeOfList) {
			pos = SizeOfList;
			break;
		} // exit
	if (key == list[i]) {
			pos = i;
			break;
		} // exit
	} // for
	return(pos);
} // search

string line, label, text, seqno;
string comment, alphanum, word;
string indent, indentval;
	
enum InDentType { InDent, OutDent, OutInDent, NoDent };
const int NoKeyWords = 7;
InDentType KeyType[NoKeyWords + 1];
string KeyWord[NoKeyWords];

void PrintLine() {
	text = indent + text;
 
	if (text.length() > 66) {
		cout << line << "\n"
			<< "--- ERROR --- FORTRAN TEXT > 66 CHARACTERS AFTER INDENTATION\n";
	} else {
		cout << label + text.substr(1,66) + seqno << "\n";
	} // if
} // PrintLine

main() {
 	indent    = "";
	indentval = "|  ";
 
	alphanum  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	comment = "C";
 
	KeyWord[0] = "IF";		KeyType[0] = InDent;
	KeyWord[1] = "ELSE";		KeyType[1] = OutInDent;
	KeyWord[2] = "ENDIF";		KeyType[2] = OutDent;
	KeyWord[3] = "WHILE";		KeyType[3] = InDent;
	KeyWord[4] = "ENDWHILE";	KeyType[4] = OutDent;
	KeyWord[5] = "DO";		KeyType[5] = InDent;
	KeyWord[6] = "CONTINUE";	KeyType[6] = OutDent;
					KeyType[NoKeyWords] = NoDent;

	while (cin >> line) {
		if (line.substr(1, 1) == comment) {
 			cout << comment << "\n";
			for (;;) {
				cout << line << "\n";
			if (!(cin >> line)) {
					cout << comment << "\n";
					goto finish;
				} // exit
			if (line.substr(1, 1) != comment) break;
			} // for
			cout << comment << "\n";
		} // if

		label = line.substr(1,6);	// label & cont., cols 1-6
		text  = line.substr(7,66);	// Fortran text,  cols 7-72
		seqno = line.substr(73,8);	// sequence no.,  cols 73-80
 
		text = (text.trim()).ltrim();	// remove leading & trailing blanks
 
		word = text.span(alphanum);	// extract 1st word on line
 
		switch (KeyType[search(KeyWord, NoKeyWords, word)]) {
		case InDent:
			PrintLine();
			indent += indentval;
			break;
		case OutDent:
			indent = indent.remove(1, indentval.length());
			PrintLine();
			break;
		case OutInDent:
			indent = indent.remove(1, indentval.length());
			PrintLine();
			indent += indentval;
			break;
		case NoDent:
			PrintLine();
			break;
		} // switch
	} // for
    finish: ;
} // main
