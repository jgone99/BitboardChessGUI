#include "moveexec.h"
#include "movegen.h"

void update_game_state(Position& position)
{
    Color color_to_move = position.color_to_move;
    Square king_square = Square(bit_scan_forward(position.pieces[color_to_move][KING]));
    Bitboard attacking_pieces = attacked_by(position, Square(bit_scan_forward(position.pieces[color_to_move][KING])), color_to_move ^ 1);
    
    if (!attacking_pieces)
    {
        position.state = NORMAL;
		return;
    }

    if (legal_moves(position, king_square, nullptr))
    {
        position.state = CHECK;
		return;
    }

    
    if (attacking_pieces & (attacking_pieces - 1))
    {
        position.state = CHECKMATE;
		return;
    }

	int attacker_square = bit_scan_forward(attacking_pieces);

    if (attacking_pieces & position.pieces[color_to_move ^ 1][KNIGHT])
    {
		Bitboard defenders = attacked_by(position, Square(attacker_square), color_to_move);

        while (defenders)
        {
            Square defender_square = Square(bit_scan_forward(defenders));
            Bitboard defend_moves = legal_moves(position, defender_square, nullptr);
            if (defend_moves & attacking_pieces)
            {
                position.state = CHECK;
                return;
            }
			defenders &= defenders - 1;
        }

		position.state = CHECKMATE;
		return;
    }

    Bitboard ray = 0ULL;
    for (int dir = NORTHWEST; dir <= WEST && !(ray & attacking_pieces); ++dir)
        ray = get_ray_attacks(position.all_occupancy, Direction(dir), king_square);
        
    Square intercept_square;
    while (ray)
    {
        intercept_square = Square(bit_scan_forward(ray));
        if (attacked_by(position, intercept_square, color_to_move) & ~(position.pieces[color_to_move][PAWN] | position.pieces[color_to_move][KING]))
        {
            position.state = CHECK;
            return;
		}
        ray &= ray - 1;
    }

	position.state = CHECKMATE;
}

void test_move(Move& move, Position& position)
{
    Bitboard from_bb = (1ULL << move.from);
    Bitboard to_bb = (1ULL << move.to);
    Color color_to_move = position.color_to_move;

    move.is_castling = move.piece_type == KING && (move.to - move.from == 2 || move.to - move.from == -2);

    //if (final_square_bb & ~current_position.empty)
    //{
    //	for (dest_type = nPawn; (dest_type <= nKing) && !(current_position.occupancy[dest_type] & final_square_bb); dest_type++);
    //	current_position.occupancy[!color_to_move] &= ~final_square_bb;
    //	current_position.occupancy[dest_type] &= ~final_square_bb;
    //	position_history_3fold.clear();
    //}

    Bitboard from_to_bb = from_bb ^ to_bb;

    position.pieces[color_to_move][move.piece_type] ^= from_to_bb;

    if (move.is_castling)
    {
        position.pieces[color_to_move][ROOK] ^= (to_bb > from_bb ? (Bitboard(0B101) << (ROOKS_KINGSIDE[color_to_move] - 2) ) : (Bitboard(0B1001) << (ROOKS_QUEENSIDE[color_to_move])));
        update_occupancies(position);
    }
    else
    {
        if (move.captured_type != -1)
            position.pieces[color_to_move ^ 1][move.captured_type] &= ~to_bb;
        update_occupancies(position);
    }

    position.color_to_move = Color(color_to_move ^ 1);
}

void make_move(const Move& move, Position& position)
{
    Bitboard from_bb = (1ULL << move.from);
    Bitboard to_bb = (1ULL << move.to);
    Color color_to_move = position.color_to_move;

    //if (final_square_bb & ~current_position.empty)
    //{
    //	for (dest_type = nPawn; (dest_type <= nKing) && !(current_position.occupancy[dest_type] & final_square_bb); dest_type++);
    //	current_position.occupancy[!color_to_move] &= ~final_square_bb;
    //	current_position.occupancy[dest_type] &= ~final_square_bb;
    //	position_history_3fold.clear();
    //}

    Bitboard from_to_bb = from_bb ^ to_bb;

    position.pieces[color_to_move][move.piece_type] ^= from_to_bb;

    if (move.is_castling)
        position.pieces[color_to_move][ROOK] ^= (to_bb > from_bb ? (Bitboard(0B101) << (ROOKS_KINGSIDE[color_to_move] - 2) ) : (Bitboard(0B1001) << (ROOKS_QUEENSIDE[color_to_move])));
    else if (move.captured_type != -1)
        position.pieces[color_to_move ^ 1][move.captured_type] &= ~to_bb;

    update_castle_rights(position, move);
    update_occupancies(position);

    position.color_to_move = Color(color_to_move ^ 1);

    update_game_state(position);
}

