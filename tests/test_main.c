#include "board.h"
#include "piece.h"
#include "utility.h"
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include <assert.h>

void test_example() {
    // Example test case
    assert(1 == 1);
}

void test_piece_to_string() {
    Piece p = wPAWN;
    const char* str = pieceToString(p);
    assert(str != NULL);
}

void test_are_allies_true() {
    Piece p1 = wPAWN;
    Piece p2 = wPAWN;
    assert(areAllies(&p1, &p2));
}

void test_are_allies_false() {
    Piece p1 = wPAWN;
    Piece p2 = bPAWN;
    assert(!areAllies(&p1, &p2));
}

void test_get_class() {
    Piece p = bBISHOP;
    int classVal = getClass(&p);
    assert(classVal == 3); // 3 is usually bishop in chess piece enums
}

int main() {
    test_example();
    test_get_class();
    test_are_allies_false();
    test_are_allies_true();
    test_piece_to_string();


    return 0;
}
