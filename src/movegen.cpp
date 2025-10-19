
#include "movegen.h"
#include "moveexec.h"

const int DIRECTION_OFFSETS[8][2]
{
    { 0, 1 },
    { 1, 1 },
    { 1, 0 },
    { 1, -1},
    { 0, -1},
    { -1, -1 },
    { -1, 0 },
    { -1, 1}
};

Bitboard ray_attacks[8][64];

// Pre-computes the attacking rays for the sliding pieces. A middle-ground approach between a per-move calculation and a magic bitboard solution.
void init_rays()
{
    for (int sq = 0; sq < 64; ++sq) {
        int rank = sq / 8;
        int file = sq % 8;

        for (int dir = 0; dir < 8; ++dir) {
            int dr = DIRECTION_OFFSETS[dir][1];
            int df = DIRECTION_OFFSETS[dir][0];

            int r = rank + dr;
            int f = file + df;
            Bitboard ray = 0ULL;

            while (r >= 0 && r < 8 && f >= 0 && f < 8) {
                ray |= (1ULL << (r * 8 + f));
                r += dr;
                f += df;
            }
            ray_attacks[dir][sq] = ray;
        }
    }
}

Bitboard pawn_attacks(Bitboard squares, int color)
{
    return color == BLACK ? south_east_one(squares) | south_west_one(squares) : north_east_one(squares) | north_west_one(squares);
}

Bitboard get_positive_ray_attacks(Bitboard occupied, Direction dir, Square square)
{
    Bitboard attacks = ray_attacks[dir][square];
    Bitboard blockers = attacks & occupied;
    if (blockers)
    {
        square = (Square)bit_scan_forward(blockers);
        attacks ^= ray_attacks[dir][square];
    }
    return attacks;
}

Bitboard get_negative_ray_attacks(Bitboard occupied, Direction dir, Square square)
{
    Bitboard attacks = ray_attacks[dir][square];
    Bitboard blockers = attacks & occupied;
    if (blockers)
    {
        square = (Square)bit_scan_reverse(blockers);
        attacks ^= ray_attacks[dir][square];
    }
    return attacks;
}

Bitboard diagonal_attacks(Bitboard occupancy, Square square)
{
    return get_positive_ray_attacks(occupancy, NORTHEAST, square) | get_negative_ray_attacks(occupancy, SOUTHWEST, square);
}

Bitboard anti_diagonal_attacks(Bitboard occupancy, Square square)
{
    return get_positive_ray_attacks(occupancy, NORTHWEST, square) | get_negative_ray_attacks(occupancy, SOUTHEAST, square);
}

Bitboard file_attacks(Bitboard occupancy, Square square)
{
    return get_positive_ray_attacks(occupancy, NORTH, square) | get_negative_ray_attacks(occupancy, SOUTH, square);
}

Bitboard rank_attacks(Bitboard occupancy, Square square)
{
    return get_positive_ray_attacks(occupancy, EAST, square) | get_negative_ray_attacks(occupancy, WEST, square);
}

Bitboard rook_attacks(Bitboard occupancy, Square square)
{
    return rank_attacks(occupancy, square) | file_attacks(occupancy, square);
}

Bitboard bishop_attacks(Bitboard occupancy, Square square)
{
    return diagonal_attacks(occupancy, square) | anti_diagonal_attacks(occupancy, square);
}

Bitboard queen_attacks(Bitboard occupancy, Square square)
{
    return rook_attacks(occupancy, square) | bishop_attacks(occupancy, square);
}

Bitboard knight_attacks(Bitboard knights) {
    Bitboard west, east, attacks;
    east = east_one(knights);
    west = west_one(knights);
    attacks = (east | west) << 16;
    attacks |= (east | west) >> 16;
    east = east_one(east);
    west = west_one(west);
    attacks |= (east | west) << 8;
    attacks |= (east | west) >> 8;
    return attacks;
}

Bitboard king_attacks(Bitboard king) {
    Bitboard attacks = east_one(king) | west_one(king);
    king |= attacks;
    attacks |= north_one(king) | south_one(king);
    return attacks;
}

Bitboard knight_moves(const Position& position, Square square, int color)
{
    Bitboard attacks = knight_attacks(1ULL << square);

    return attacks & ~position.occupancy[color];
}

Bitboard king_moves(const Position& position, Square square, int color)
{
    Bitboard attacks = king_attacks(1ULL << square);

    return (attacks & ~position.occupancy[color]) | castling_moves(position, Color(color));
}

Bitboard single_push(const Bitboard pawns, Bitboard empty, int color)
{
    return _rotl64(pawns, 8 - (color << 4)) & empty;
}

Bitboard double_push(const Bitboard pawns, Bitboard empty, int color)
{
    return _rotl64(pawns, 16 - (color << 5)) & empty;
}

Bitboard pawn_moves(const Position& position, Square squares, int color)
{
    Bitboard sq_bb = 1ULL << squares;
    Bitboard pawns = position.pieces[color][PAWN] & sq_bb;
    Bitboard not_moved = pawns & (color == WHITE ? SECOND_RANK : SEVENTH_RANK);
    Bitboard attacks = pawn_attacks(sq_bb, color) & position.occupancy[!color];

    return single_push(pawns, position.empty, color) | double_push(not_moved, position.empty, color) | attacks;
}

Bitboard rook_moves(const Position& position, Square square, int color)
{
    return rook_attacks(position.all_occupancy, square) & ~position.occupancy[color];
}

Bitboard bishop_moves(const Position& position, Square square, int color)
{
    return bishop_attacks(position.all_occupancy, square) & ~position.occupancy[color];
}

Bitboard queen_moves(const Position& position, Square square, int color)
{
    return queen_attacks(position.all_occupancy, square) & ~position.occupancy[color];
}

Bitboard castling_moves(const Position& position, Color color)
{
    if (is_king_in_check(position, color ^ 1)) return 0ULL;

    Bitboard occ = position.all_occupancy;
    Bitboard result = 0ULL;

    if (position.castling_rights[color] & KS)
    {
        if (!(occ & KSIDE_BLOCK_MASK[color]) &&
            !is_attacked(position, KSIDE_PASS_SQUARES[color][0], color ^ 1) &&
            !is_attacked(position, KSIDE_PASS_SQUARES[color][1], color ^ 1))
        {
            result |= (1ULL << KSIDE_KING_DEST[color]);
        }
    }

    if (position.castling_rights[color] & QS)
    {
        if (!(occ & QSIDE_BLOCK_MASK[color]) &&
            !is_attacked(position, QSIDE_PASS_SQUARES[color][0], color ^ 1) &&
            !is_attacked(position, QSIDE_PASS_SQUARES[color][1], color ^ 1))
        {
            result |= (1ULL << QSIDE_KING_DEST[color]);
        }
    }

    return result;
}

Bitboard moves(Square square, const Position& position)
{
    Bitboard sq = (1ULL << square);
    int color = (position.occupancy[BLACK] & sq) > 0;

    int type;
    for (type = PAWN; (type < KING) && !(sq & position.pieces[color][type]); ++type);

    Bitboard moves;

    switch (type)
    {
    case PAWN:
        moves = pawn_moves(position, square, color);
        break;
    case ROOK:
        moves = rook_moves(position, square, color);
        break;
    case KNIGHT:
        moves = knight_moves(position, square, color);
        break;
    case BISHOP:
        moves = bishop_moves(position, square, color);
        break;
    case QUEEN:
        moves = queen_moves(position, square, color);
        break;
    case KING:
        moves = king_moves(position, square, color);
        break;
    default:
        break;
    }

    return moves;
}

Bitboard legal_moves(const Position& position, Square square, std::vector<Move>* legal_moves_list)
{
    if (legal_moves_list) legal_moves_list->clear();

    if (~position.all_occupancy & (1ULL << square)) return 0ULL;

    Bitboard peusdo_legal_moves = moves(square, position);
    Bitboard legal_moves = 0ULL;

    while (peusdo_legal_moves)
    {
        Square to_square = (Square)bit_scan_forward(peusdo_legal_moves);
        peusdo_legal_moves &= peusdo_legal_moves - 1;
        Position new_position = position;
        // Make the move on a copy of the position
        Bitboard from_bb = 1ULL << square;
        Bitboard to_bb = 1ULL << to_square;
        int piece_type;
        int captured_type = -1;
        for (piece_type = PAWN; piece_type <= KING && !(from_bb & new_position.pieces[new_position.color_to_move][piece_type]); ++piece_type);

        if (to_bb & new_position.occupancy[new_position.color_to_move ^ 1])
        {
            for (captured_type = PAWN; captured_type <= KING && !(to_bb & new_position.pieces[new_position.color_to_move ^ 1][captured_type]); ++captured_type);
        }

        Move move;
        move.from = square;
        move.to = to_square;
        move.piece_type = (PieceType)piece_type;
        move.color = new_position.color_to_move;
        move.captured_type = captured_type;

        test_move(move, new_position);

        if (!is_king_in_check(new_position, new_position.color_to_move))
        {
            legal_moves |= to_bb;
            if (legal_moves_list)
                legal_moves_list->push_back(move);
        }
    }

    return legal_moves;
}

bool is_valid_square(const Position& position, Square square)
{
    Bitboard bb = (1ULL << square);
    return bb && (position.occupancy[position.color_to_move] & bb);
}

bool is_friendly_square(const Position& position, Square square)
{
    return bool((1ULL << square) & position.occupancy[position.color_to_move]);
}

bool is_attacked(const Position& position, Square square, int attacking_color)
{
    Bitboard sq_bb = 1ULL << square;
    Bitboard pawns = position.pieces[attacking_color][PAWN];
    if (pawn_attacks(sq_bb, attacking_color ^ 1) & pawns) return true;

    Bitboard knights = position.pieces[attacking_color][KNIGHT];
    if (knight_attacks(sq_bb) & knights) return true;

    Bitboard bishops_queens = position.pieces[attacking_color][BISHOP] | position.pieces[attacking_color][QUEEN];
    if (bishop_attacks(position.all_occupancy, square) & bishops_queens) return true;

    Bitboard rooks_queens = position.pieces[attacking_color][ROOK] | position.pieces[attacking_color][QUEEN];
    if (rook_attacks(position.all_occupancy, square) & rooks_queens) return true;

    Bitboard king = position.pieces[attacking_color][KING];
    if (king_attacks(sq_bb) & king) return true;

    return false;
}

Bitboard attacked_by(const Position& position, Square square, int attacking_color)
{
    Bitboard sq_bb = 1ULL << square;
    Bitboard pawns = position.pieces[attacking_color][PAWN];
    Bitboard knights = position.pieces[attacking_color][KNIGHT];
    Bitboard bishops_queens = position.pieces[attacking_color][BISHOP] | position.pieces[attacking_color][QUEEN];
    Bitboard rooks_queens = position.pieces[attacking_color][ROOK] | position.pieces[attacking_color][QUEEN];
    Bitboard king = position.pieces[attacking_color][KING];

    return pawn_attacks(sq_bb, attacking_color) & pawns |
           knight_attacks(sq_bb) & knights |
           bishop_attacks(position.all_occupancy, square) & bishops_queens |
           rook_attacks(position.all_occupancy, square) & rooks_queens |
           king_attacks(sq_bb) & king;
}

bool is_king_in_check(const Position& position, int attacking_color)
{
    return is_attacked(position, Square(bit_scan_forward(position.pieces[attacking_color ^ 1][KING])), attacking_color);
}

