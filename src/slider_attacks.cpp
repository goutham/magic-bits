#include "common.h"
#include "slider_attacks.h"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

template <typename T>
void ReadFromFile(const std::string& filename, std::vector<T>* v) {
  std::ifstream ifs(filename.c_str(), std::ios::in);
  if (!ifs.is_open()) {
    std::cerr << "Unable to open file " << filename << std::endl;
    exit(-1);
  }
  std::string s;
  while (ifs >> s) {
    v->push_back(T(std::strtoull(s.c_str(), nullptr, 16)));
  }
  ifs.close();
}

void SliderAttacks::Initialize() {
  std::vector<U64> rook_masks;
  ReadFromFile(kRookMasks, &rook_masks);
  assert(rook_masks.size() == kSquares);

  std::vector<U64> bishop_masks;
  ReadFromFile(kBishopMasks, &bishop_masks);
  assert(bishop_masks.size() == kSquares);

  std::vector<U64> rook_magics;
  ReadFromFile(kRookMagics, &rook_magics);
  assert(rook_magics.size() == kSquares);

  std::vector<U64> bishop_magics;
  ReadFromFile(kBishopMagics, &bishop_magics);
  assert(bishop_magics.size() == kSquares);

  std::vector<int> rook_offsets;
  ReadFromFile(kRookOffsets, &rook_offsets);
  assert(rook_offsets.size() == kSquares);

  std::vector<int> bishop_offsets;
  ReadFromFile(kBishopOffsets, &bishop_offsets);
  assert(bishop_offsets.size() == kSquares);

  std::vector<int> rook_shifts;
  ReadFromFile(kRookShifts, &rook_shifts);
  assert(rook_shifts.size() == kSquares);

  std::vector<int> bishop_shifts;
  ReadFromFile(kBishopShifts, &bishop_shifts);
  assert(bishop_shifts.size() == kSquares);

  ReadFromFile(kRookAttackTable, &rook_attack_table_);
  rook_attack_table_.shrink_to_fit();

  ReadFromFile(kBishopAttackTable, &bishop_attack_table_);
  bishop_attack_table_.shrink_to_fit();

  for (int i = 0; i < kSquares; ++i) {
    rook_magics_[i] = { rook_masks.at(i), rook_magics.at(i),
                        rook_shifts.at(i), rook_offsets.at(i) };
    bishop_magics_[i] = { bishop_masks.at(i), bishop_magics.at(i),
                          bishop_shifts.at(i), bishop_offsets.at(i) };
  }
}
