//-*-C++-*-
/* Copyright (c) 1987 Peter A. Buhr */
	
class VbyteHeap;

class HandleNode {
	friend class VbyteHeap;
	
	HandleNode *flink;				// forward link
	HandleNode *blink;				// backward link

	inline HandleNode();				// constructor for header node
	void AddThisAfter(HandleNode *);
	void DeleteNode();
    public:
	char *s;					// pointer to byte string
	unsigned short int lnth;			// length of byte string
	
	inline HandleNode(VbyteHeap *);			// constructor for nodes in the handle list
	inline ~HandleNode();				// destructor for handle nodes
	inline void MoveThisAfter(HandleNode *);	// move current handle after parameter handle
	inline void MoveThisBefore(HandleNode *);	// move current handle before parameter handle
	void OrderedMoveThisAfter(HandleNode*);		// move current handle after parameter handle, maintaining order
	void OrderedMoveThisBefore(HandleNode *);	// move current handle before parameter handle, maintaining order
}; // HandleNode

class VbyteHeap {
	friend class HandleNode;

	int NoOfCompactions;				// number of compactions of the byte area
	int NoOfExtensions;				// number of extensions in the size of the byte area
	int NoOfReductions;				// number of reductions in the size of the byte area
		
	int InitSize;					// initial number of bytes in the byte-string area
	int CurrSize;					// current number of bytes in the byte-string area
	char *StartVbyte;				// pointer to the 1st byte of the start of the byte-string area
	char *EndVbyte;					// pointer to the next byte after the end of the currently used portion of byte-string area
	void *ExtVbyte;					// pointer to the next byte after the end of the byte-string area
	
	void compaction();				// compaction of the byte area
	void garbage();					// garbage collect the byte area
	void extend(int);				// extend the size of the byte area
	void reduce(int);				// reduce the size of the byte area
    public:
	HandleNode Header;				// header node for handle list
	
	inline VbyteHeap(int = 1000);
	inline ~VbyteHeap();
	void ByteCopy(char *, int, int, char *, int, int);
	int ByteCmp(char *, int, int, char *, int, int);
	char *VbyteAlloc(int);
}; // VbyteHeap

/* Create a handle node. The handle is not linked into the handle
list.  This is the responsibilitiy of the handle creator.  */

inline HandleNode::HandleNode() {
	s = 0;
	lnth = 0;
} // HandleNode

/* Create a handle node. The handle is linked into the handle list at
the end. This means that this handle will NOT be in order by string
address, but this is not a problem because a string with length zero
does nothing during garbage collection. */

inline HandleNode::HandleNode(VbyteHeap *vh) {
	s = 0;
	lnth = 0;
	AddThisAfter(vh->Header.blink);
} // HandleNode

/* Delete a node from the handle list by unchaining it from the list.
If the handle node was allocated dynamically, it is the responsibility
of the creator to destroy it.  */

inline HandleNode::~HandleNode() {
	DeleteNode();
} // ~HandleNode

/* Move an existing HandleNode node n after the current HandleNode
node */

inline void HandleNode::MoveThisAfter(HandleNode *h) {
	//cerr << "enter:MoveThisAfter, this:" << (int)this << " h:" << (int)h << "\n";
	if (this != h) {
		DeleteNode();
		AddThisAfter(h);
	} // if
	//cerr << "exit:MoveThisAfter\n";
} // MoveThisAfter

/* Move an existing HandleNode node n before the current HandleNode
node */

inline void HandleNode::MoveThisBefore(HandleNode *h) {
	//cerr << "enter:MoveThisBefore, this:" << (int)this << " h:" << (int)h << "\n";
	if (this != h) {
		DeleteNode();
		AddThisAfter(h->blink);
	} // if
	//cerr << "exit:MoveThisBefore\n";
} // MoveThisBefore

/* Allocate the storage for the variable sized area and intialize the
heap variables.  */

inline VbyteHeap::VbyteHeap(int Size) {
	NoOfCompactions = NoOfExtensions = NoOfReductions = 0;
	InitSize = CurrSize = Size;
	StartVbyte = EndVbyte = new char[CurrSize];
	ExtVbyte = (void *)(StartVbyte + CurrSize);
	Header.flink = Header.blink = &Header;
} // VbyteHeap

/* Release the dynamically allocated storage for the byte area.  */

inline VbyteHeap::~VbyteHeap() {
	delete[CurrSize] StartVbyte;
} // ~VbyteHeap

