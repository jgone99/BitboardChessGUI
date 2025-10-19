#include <sstream>
#include "position.h"

int max_rank = 8;
int max_file = 8;
Bitboard A_FILE = 0x0101010101010101;
Bitboard H_FILE = 0x8080808080808080;
Bitboard FIRST_RANK = 0x00000000000000FF;
Bitboard EIGHT_RANK = 0xFF00000000000000;
Bitboard SECOND_RANK = 0x000000000000FF00ULL;
Bitboard SEVENTH_RANK = 0x00FF000000000000ULL;
Bitboard A1_H8 = 0x8040201008040201;
Bitboard A8_H1 = 0x0102040810204080;
Bitboard AB_FILE = A_FILE | (A_FILE << 1);
Bitboard GH_FILE = H_FILE | (H_FILE >> 1);
Bitboard FORTH_RANK = FIRST_RANK << 24;
Bitboard FIFTH_RANK = FORTH_RANK << 8;

char WHITE_PIECE_CHAR[6]  { 'p', 'r', 'n', 'b', 'q', 'k' };
char BLACK_PIECE_CHAR[6]  { 'P', 'R', 'N', 'B', 'Q', 'K' };

Square ROOKS_KINGSIDE[2] { h1, h8 };
Square ROOKS_QUEENSIDE[2] { a1, a8 };
Bitboard KSIDE_BLOCK_MASK[2] = { (1ULL << f1) | (1ULL << g1), (1ULL << f8) | (1ULL << g8) };
Bitboard QSIDE_BLOCK_MASK[2] = { (1ULL << b1) | (1ULL << c1) | (1ULL << d1), (1ULL << b8) | (1ULL << c8) | (1ULL << d8) };

Square KSIDE_PASS_SQUARES[2][2] = { { f1, g1 }, { f8, g8 } };
Square QSIDE_PASS_SQUARES[2][2] = { { d1, c1 }, { d8, c8 } };
Square KSIDE_KING_DEST[2] = { g1, g8 };
Square QSIDE_KING_DEST[2] = { c1, c8 };

const Position starting_position
{
    // [color][[piece type] bitboards
    {
        { 0x000000000000FF00ULL, 0x0000000000000081ULL, 0x0000000000000042ULL, 0x0000000000000024ULL, 0x0000000000000008ULL, 0x0000000000000010ULL },
        { 0x00FF000000000000ULL, 0x8100000000000000ULL, 0x4200000000000000ULL, 0x2400000000000000ULL, 0x0800000000000000ULL, 0x1000000000000000ULL }
    },
    // occupancy bitboards for both colors
    {
        0x000000000000FFFFULL,
        0xFFFF000000000000ULL
    },
    // total occupancy bitboard
    0xFFFF00000000FFFFULL,
    // empty square bitboard
    0x0000FFFFFFFF0000ULL
};

void update_occupancies(Position& position) {
    position.occupancy[WHITE] = position.pieces[WHITE][PAWN] | position.pieces[WHITE][KNIGHT] | position.pieces[WHITE][BISHOP] | position.pieces[WHITE][ROOK] | position.pieces[WHITE][QUEEN] | position.pieces[WHITE][KING];
    position.occupancy[BLACK] = position.pieces[BLACK][PAWN] | position.pieces[BLACK][KNIGHT] | position.pieces[BLACK][BISHOP] | position.pieces[BLACK][ROOK] | position.pieces[BLACK][QUEEN] | position.pieces[BLACK][KING];
    position.all_occupancy = position.occupancy[WHITE] | position.occupancy[BLACK];
    position.empty = ~position.all_occupancy;
}

void update_castle_rights(Position& position, const Move &move)
{
    switch (move.piece_type)
    {
    case KING:
        position.castling_rights[move.color] = CastlingRights(position.castling_rights[move.color] & ~(KS | QS));
        break;
    case ROOK:
        if (move.from == ROOKS_KINGSIDE[move.color])
            position.castling_rights[move.color] = CastlingRights(position.castling_rights[move.color] & ~KS);
        else if (move.from == ROOKS_QUEENSIDE[move.color])
            position.castling_rights[move.color] = CastlingRights(position.castling_rights[move.color] & ~QS);
        break;
    default:
        break;
    }

    // Also check if a rook is *captured* on its original square
    if (move.captured_type == ROOK) {
        if (move.to == ROOKS_KINGSIDE[move.color ^ 1])
            position.castling_rights[move.color ^ 1] = CastlingRights(position.castling_rights[move.color ^ 1] & ~KS);
        else if (move.to == ROOKS_QUEENSIDE[move.color ^ 1])
            position.castling_rights[move.color ^ 1] = CastlingRights(position.castling_rights[move.color ^ 1] & ~QS);
    }
}

Position fen_to_pos(std::string fen)
{
    Position position = {};

    size_t meta_index;

    std::stringstream ss(fen.substr(0, meta_index = fen.find_first_of(' ')));
    std::stringstream ss_meta(fen.substr(meta_index));
    std::string token;

    int square = 63;

    while (std::getline(ss, token, '/'))
    {
        for (int k = static_cast<int>(token.length() - 1); k >= 0; k--)
        {
            char c = token[k];
            if (isdigit(c))
            {
                square -= (c - '0');
                continue;
            }

            switch (c)
            {
            case 'P':
                set_bit(position.pieces[WHITE][PAWN], square);
                break;
            case 'R':
                set_bit(position.pieces[WHITE][ROOK], square);
                break;
            case 'N':
                set_bit(position.pieces[WHITE][KNIGHT], square);
                break;
            case 'B':
                set_bit(position.pieces[WHITE][BISHOP], square);
                break;
            case 'Q':
                set_bit(position.pieces[WHITE][QUEEN], square);
                break;
            case 'K':
                set_bit(position.pieces[WHITE][KING], square);
                break;
            case 'p':
                set_bit(position.pieces[BLACK][PAWN], square);
                break;
            case 'r':
                set_bit(position.pieces[BLACK][ROOK], square);
                break;
            case 'n':
                set_bit(position.pieces[BLACK][KNIGHT], square);
                break;
            case 'b':
                set_bit(position.pieces[BLACK][BISHOP], square);
                break;
            case 'q':
                set_bit(position.pieces[BLACK][QUEEN], square);
                break;
            case 'k':
                set_bit(position.pieces[BLACK][KING], square);
                break;
            default:
                break;
            }
            square--;
        }
    }
    update_occupancies(position);

    std::getline(ss_meta, token, ' ');

    position.color_to_move = Color(token[0] == 'b');

    return position;
}

std::string pos_stringid(Position position)
{
    std::string stringid = "";
    for (int type = PAWN; type <= KING; type++)
    {
        stringid += std::to_string(position.pieces[WHITE][type] | position.pieces[BLACK][type]);
    }
    stringid += std::to_string(position.color_to_move);

    return stringid;
}
