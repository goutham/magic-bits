magic-bits
==========

Magic-bitboards for Chess -- a C++ library to aid in developing super fast move-generation and evaluation routines in chess programs.

## Usage

```
$ make
g++ -std=c++0x -O3 magic_bits.cpp -o magic_bits
g++ -std=c++0x -O3 -c slider_attacks.cpp -o slider_attacks.o
g++ -std=c++0x -O3 slider_attacks_test.cpp slider_attacks.o -o slider_attacks_test
$ ./magic_bits
```

This will generate several files ending with .magic that are loaded by SliderAttacks class. SliderAttacks class can be used to generate all sliding piece (queen, rook and bishop) attacks.

```cpp
// A dummy move generator to show SliderAttacks usage.
#include "slider_attacks.h"

class MoveGenerator {
  public:
    MoveGenerator() {
      slider_attacks_.Initialize();
    }
    
    vector<Move> GenerateQueenMoves(const Board& board, const int queen_index) {
      U64 queen_attacks = slider_attacks_.QueenAttacks(board.occupancy_bitboard,
                                                       queen_index);
      queen_attacks &= ~board.moving_side_bitboard();  // discard self-piece captures
      return GenerateMovesFromAttackBitBoard(queen_attacks, queen_index);
    }
    
    // Methods to generate moves for other pieces
    // ...

  private:
    SliderAttacks slider_attacks_;
};
```

## Links

Information on magic bitboards: http://chessprogramming.wikispaces.com/Magic+Bitboards
