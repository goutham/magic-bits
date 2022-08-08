# magic-bits

A C++ header-only library to aid in developing super fast move-generation and evaluation routines in chess programs.

## Background

Efficient move generation can significantly influence the strength of a chess program since moves are generated at almost all internal nodes and possibly leaf / eval nodes of an alpha-beta search tree. Sliding pieces (queen, rook and bishop) have a lot of mobility and the squares they can occupy is dependent on the occupancy of many other self and opponent pieces on the board. This makes it hard to generate moves for sliding pieces efficiently using naive techniques, and "magic bitboards", as is commonly known in the chess programming literature, comes in handy.

The magic bitboards technique, in short, implements a perfect-hashing scheme where a pre-computed magic number is multiplied with the corresponding occupancy bitboard, which is then right-shifted to obtain an index into a pre-computed attack-bitboards table. More details about magic bitboards can be found at https://www.chessprogramming.org/Magic_Bitboards.

## Installation

Requires C++17 (for example, `g++` with flag `-std=c++17`) or higher to compile. The library itself is a self-contained C++ header file to be included in your program:

```cpp
#include "magic-bits/include/magic_bits.hpp"
```

## Usage

```cpp
// -- move_generator.cpp --

// Include the header file to use the magic_bits::Attacks class.
#include "magic-bits/include/magic_bits.hpp"

namespace {
...
// Instantiate an object of magic_bits::Attacks class once in your program.
// If it is used in multiple files, instantiate once (for example in main())
// and pass it around.
const magic_bits::Attacks attacks;
...
}

// An example function that generates queen moves.
vector<Move> GenerateQueenMoves(const Board& board, int queen_index) {
  // Get the queen attack bitboard.
  uint64_t queen_attacks = attacks.Queen(board.occupancy_bitboard, queen_index);

  // Discard self-piece captures.
  queen_attacks &= ~board.moving_side_bitboard();

  // Convert the moves to a format appropriate in your program:
  // For each move, queen_index will be the "from" index and each of the set
  // bits in `queen_attacks` bitboard form the "to" indices.
  return CreateMoveVectorFromAttackBitBoard(queen_attacks, queen_index);
}
```

A real world usage example can be found in https://github.com/goutham/nakshatra.
