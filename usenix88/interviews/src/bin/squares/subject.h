/*
 * The subject is a simple list of squares.
 */

#ifndef subject_h
#define subject_h

class SquareData {
    friend class Squares;
    friend class SquaresView;

    float cx, cy, size;
    SquareData* next;
};

class SquaresViewList {
    friend class Squares;

    class SquaresView* view;
    SquaresViewList* next;
};

class Squares {
    SquareData* head;
    SquaresViewList* views;
public:
    Squares();
    ~Squares();
    void Add(float cx, float cy, float size);
    void Add();
    SquareData* Head () { return head; }
    void Attach(SquaresView*);
    void Detach(SquaresView*);
    void Notify();
};

#endif
