#ifndef SLIDER_ATTACKS
#define SLIDER_ATTACKS

#include "common.h"

#include <vector>

struct Magic {
  U64 mask;
  U64 magic;
  int shift;
  int offset;
};

// Comprises methods to generate rook, bishop and queen attacks
// from a square, for a given piece occupancy bitboard. magic_bits
// should have been run to generate *.magic files for this to work.
// Initialize() method must be called before using other methods.
class SliderAttacks {
 public:
  void Initialize();

  U64 RookAttacks(const U64 bitboard, const int index) const {
    const Magic& m = rook_magics_[index];
    return rook_attack_table_[AttackTableIndex(bitboard, m)];
  }

  U64 BishopAttacks(const U64 bitboard, const int index) const {
    const Magic& m = bishop_magics_[index];
    return bishop_attack_table_[AttackTableIndex(bitboard, m)];
  }

  U64 QueenAttacks(const U64 bitboard, const int index) const {
    return RookAttacks(bitboard, index) | BishopAttacks(bitboard, index);
  }

 private:
  U64 AttackTableIndex(const U64 bitboard, const Magic& m) const {
    U64 occupancy = bitboard & m.mask;
    return ((occupancy * m.magic) >> (kSquares - m.shift)) + m.offset;
  }

  Magic rook_magics_[kSquares];
  Magic bishop_magics_[kSquares];
  std::vector<U64> rook_attack_table_;
  std::vector<U64> bishop_attack_table_;
};

#endif
