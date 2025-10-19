#ifndef BITBOARD_H
#define BITBOARD_H

typedef unsigned long long Bitboard;

// De Bruijn sequence to 64-index mapping
extern const int INDEX64[64];

// De Bruijn sequence over alphabet {0, 1}. B(2, 6)
extern const Bitboard DEBRUIJN64;

inline void set_bit(Bitboard& bitboard, int square) { bitboard |= (1ULL << square); }
inline Bitboard to_bb(int sq) { return 1ULL << sq; }
inline Bitboard bitboard_union(Bitboard bitboard1, Bitboard bitboard2) { return bitboard1 | bitboard2; }
inline int bit_scan_forward(Bitboard bitboard) { return bitboard ? (INDEX64[((bitboard ^ (bitboard - 1)) * DEBRUIJN64) >> 58]) : -1; }
inline int bit_scan_reverse(Bitboard bitboard) {
    bitboard |= bitboard >> 1;
    bitboard |= bitboard >> 2;
    bitboard |= bitboard >> 4;
    bitboard |= bitboard >> 8;
    bitboard |= bitboard >> 16;
    bitboard |= bitboard >> 32;
    return INDEX64[(bitboard * DEBRUIJN64) >> 58];
}

inline Bitboard north_shift(Bitboard bitboard, int shift) { return bitboard << (8 * shift); }
inline Bitboard north_east_shift(Bitboard bitboard, int shift) { return bitboard << (9 * shift); }
inline Bitboard east_shift(Bitboard bitboard, int shift) { return bitboard << 1; }
inline Bitboard south_east_shift(Bitboard bitboard, int shift) { return bitboard >> (7 * shift); }
inline Bitboard south_shift(Bitboard bitboard, int shift) { return bitboard >> (8 * shift); }
inline Bitboard south_west_shift(Bitboard bitboard, int shift) { return bitboard >> (9 * shift); }
inline Bitboard west_shift(Bitboard bitboard, int shift) { return bitboard >> (1 * shift); }
inline Bitboard north_west_shift(Bitboard bitboard, int shift) { return bitboard << (7 * shift); }

int pop_count(Bitboard bitboard);

#endif // BITBOARD_H
