#ifndef POSITION_H
#define POSITION_H

#include <string>
#include "bitboard.h"

enum PieceType
{
    PAWN = 0,
    ROOK = 1,
    KNIGHT = 2,
    BISHOP = 3,
    QUEEN = 4,
    KING = 5
};

enum Color
{
    WHITE,
    BLACK
};

enum CastlingRights
{
    NONE    = 0x00,
    KS      = 0x01,
    QS      = 0x10,
    BOTH    = 0x11
};

enum Square
{
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

enum GameState
{
    NORMAL,
    CHECK,
    CHECKMATE,
    REPETITION,
    STALEMATE
};

// Struct containing all information needed to restore a position
struct Position
{
    // 2D array of bitboards representing all pieces and colors (e.g. pieces[color][piece type])
    Bitboard pieces[2][6];
    // Bitboards representing the occupied squares of each color regardless of piece type
    Bitboard occupancy[2];
    // Bitboard representing all occupied squares regardless of piece type or color
    Bitboard all_occupancy;
    // Bitboard representing all empty squares
    Bitboard empty;
    Color color_to_move = WHITE;
    GameState state = NORMAL;
    CastlingRights castling_rights[2] { BOTH, BOTH };
};

struct Move
{
    // Initial square for moving piece
    Square from;
    // Final square for moving piece
    Square to;
    // Moving piece type
    PieceType piece_type;
    // Moving piece color
    Color color;
    // Captured piece type (if applicable)
    int captured_type;
    int promotion = -1; // -1 means no promotion
    bool is_castling = false;
    bool is_en_passant = false;
};

// Useful constants
extern int max_rank;
extern int max_file;
extern Bitboard A_FILE;
extern Bitboard H_FILE;
extern Bitboard FIRST_RANK;
extern Bitboard EIGHT_RANK;
extern Bitboard SECOND_RANK;
extern Bitboard SEVENTH_RANK;
extern Bitboard A1_H8;
extern Bitboard A8_H1;
extern Bitboard AB_FILE;
extern Bitboard GH_FILE;
extern Bitboard FORTH_RANK;
extern Bitboard FIFTH_RANK;

// For console game. printable piece characters
extern char WHITE_PIECE_CHAR[6];
extern char BLACK_PIECE_CHAR[6];

extern Square ROOKS_KINGSIDE[2];
extern Square ROOKS_QUEENSIDE[2];
extern Bitboard KSIDE_BLOCK_MASK[2];
extern Bitboard QSIDE_BLOCK_MASK[2];

// Squares the king passes through that must not be attacked
extern Square KSIDE_PASS_SQUARES[2][2];
extern Square QSIDE_PASS_SQUARES[2][2];

extern Square KSIDE_KING_DEST[2];;
extern Square QSIDE_KING_DEST[2];;

extern const Position starting_position;

// Update occupancy bitboards when position changes
void update_occupancies(Position& position);
void update_castle_rights(Position& position, const Move& move);
Position fen_to_pos(std::string fen);
std::string pos_stringid(Position position);

#endif // POSITION_H
