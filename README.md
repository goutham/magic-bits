magic-bits
==========

Magic-bitboards for Chess -- a C++ library to aid in developing super fast move-generation and evaluation routines in chess programs.

Efficient move generation can significantly influence the strength of a chess program, particularly since move generation is done so often -- at almost all internal nodes (and possibly most leaf/eval nodes) of an alpha-beta search tree. Sliding pieces (queen, rook, and bishop) have a lot of mobility and the squares they can move to is largely dependent on occupancy of many other self and opponent pieces on the board, making it harder to efficiently generate their moves compared to king, pawn or knight moves. This is where "magic bitboards", as is commonly known in the chess programming literature, comes in handy.

The magic bitboards technique, in short, implements a perfect-hashing scheme where a pre-computed magic number is multiplied with the corresponding occupancy bitboard, which is then right-shifted to obtain an index into a pre-computed attack-bitboards table. A great deal of information on magic bitboards can be found at  http://chessprogramming.wikispaces.com/Magic+Bitboards. This repository contains code to generate these magic numbers and attack bitboards tables (along with a few other things), and a SliderAttacks class that loads this data at initialization time and provides methods to efficiently generate queen, rook and bishop attacks.

## Status

COMPLETE.

## Usage

```
$ make
g++ -std=c++0x -O3 magic_bits.cpp -o magic_bits
g++ -std=c++0x -O3 -c slider_attacks.cpp -o slider_attacks.o
g++ -std=c++0x -O3 slider_attacks_test.cpp slider_attacks.o -o slider_attacks_test
$ ./magic_bits
```

Running ```magic_bits``` creates several files with suffix .magic. SliderAttacks class loads these files, and provides methods to generate all sliding piece (queen, rook and bishop) attacks.

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
