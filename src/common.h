#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

typedef uint64_t U64;

static const int kSquares = 64;

static const char kRookMagics[] = "rook_magics.magic";
static const char kRookMasks[] = "rook_masks.magic";
static const char kRookShifts[] = "rook_shifts.magic";
static const char kRookOffsets[] = "rook_offsets.magic";
static const char kRookAttackTable[] = "rook_attack_table.magic";

static const char kBishopMagics[] = "bishop_magics.magic";
static const char kBishopMasks[] = "bishop_masks.magic";
static const char kBishopShifts[] = "bishop_shifts.magic";
static const char kBishopOffsets[] = "bishop_offsets.magic";
static const char kBishopAttackTable[] = "bishop_attack_table.magic";

#endif
