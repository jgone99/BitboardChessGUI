#include "ChessGame.h"
#include <iostream>
#include <sstream>
#include <string>
#include <Windows.h>
#include <stdlib.h>

const ChessGame::Position ChessGame::starting_position
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

// De Bruijn sequence to 64-index mapping (this technique blew my mind)
const int ChessGame::INDEX64[64]
{
	0, 47,  1, 56, 48, 27,  2, 60,
	57, 49, 41, 37, 28, 16,  3, 61,
	54, 58, 35, 52, 50, 42, 21, 44,
	38, 32, 29, 23, 17, 11,  4, 62,
	46, 55, 26, 59, 40, 36, 15, 53,
	34, 51, 20, 43, 31, 22, 10, 45,
	25, 39, 14, 33, 19, 30,  9, 24,
	13, 18,  8, 12,  7,  6,  5, 63
};

const char ChessGame::BLACK_PIECE_CHAR[6] { 'p', 'r', 'n', 'b', 'q', 'k' };
const char ChessGame::WHITE_PIECE_CHAR[6] { 'P', 'R', 'N', 'B', 'Q', 'K' };

// Pre-computes the attacking rays for the sliding pieces. A middle-ground approach between a per-move calculation and a magic bitboard solution.
void ChessGame::init_rays()
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

Bitboard ChessGame::pawn_attacks(Bitboard squares, int color)
{
    return color == BLACK ? south_east_one(squares) | south_west_one(squares) : north_east_one(squares) | north_west_one(squares);
}

Bitboard ChessGame::get_positive_ray_attacks(Bitboard occupied, Direction dir, Square square)
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

Bitboard ChessGame::get_negative_ray_attacks(Bitboard occupied, Direction dir, Square square)
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

Bitboard ChessGame::diagonal_attacks(Bitboard occupancy, Square square)
{
	return get_positive_ray_attacks(occupancy, NORTHEAST, square) | get_negative_ray_attacks(occupancy, SOUTHWEST, square);
}

Bitboard ChessGame::anti_diagonal_attacks(Bitboard occupancy, Square square)
{
	return get_positive_ray_attacks(occupancy, NORTHWEST, square) | get_negative_ray_attacks(occupancy, SOUTHEAST, square);
}

Bitboard ChessGame::file_attacks(Bitboard occupancy, Square square)
{
	return get_positive_ray_attacks(occupancy, NORTH, square) | get_negative_ray_attacks(occupancy, SOUTH, square);
}

Bitboard ChessGame::rank_attacks(Bitboard occupancy, Square square)
{
	return get_positive_ray_attacks(occupancy, EAST, square) | get_negative_ray_attacks(occupancy, WEST, square);
}

Bitboard ChessGame::rook_attacks(Bitboard occupancy, Square square)
{
	return rank_attacks(occupancy, square) | file_attacks(occupancy, square);
}

Bitboard ChessGame::bishop_attacks(Bitboard occupancy, Square square)
{
	return diagonal_attacks(occupancy, square) | anti_diagonal_attacks(occupancy, square);
}

Bitboard ChessGame::queen_attacks(Bitboard occupancy, Square square)
{
	return rook_attacks(occupancy, square) | bishop_attacks(occupancy, square);
}

Bitboard ChessGame::knight_attacks(Bitboard knights) {
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

Bitboard ChessGame::king_attacks(Bitboard king) {
    Bitboard attacks = east_one(king) | west_one(king);
    king |= attacks;
    attacks |= north_one(king) | south_one(king);
    return attacks;
}

Bitboard ChessGame::knight_moves(Square square, int color)
{
    Bitboard attacks = knight_attacks(1ULL << square);
	
	return attacks & ~current_position.occupancy[color];
}

Bitboard ChessGame::king_moves(Square square, int color)
{
    Bitboard attacks = king_attacks(1ULL << square);

    return (attacks & ~current_position.occupancy[color]) | castling_moves(Color(color));
}

Bitboard ChessGame::single_push(Bitboard pawns, Bitboard empty, int color)
{
	return _rotl64(pawns, 8 - (color << 4)) & empty;
}

Bitboard ChessGame::double_push(Bitboard pawns, Bitboard empty, int color)
{
	return _rotl64(pawns, 16 - (color << 5)) & empty;
}

Bitboard ChessGame::pawn_moves(Square squares, int color)
{
    Bitboard sq_bb = 1ULL << squares;
    Bitboard pawns = current_position.pieces[color][PAWN] & sq_bb;
    Bitboard not_moved = pawns & (color == WHITE ? SECOND_RANK : SEVENTH_RANK);
    Bitboard attacks = pawn_attacks(sq_bb, color) & current_position.occupancy[!color];

	return single_push(pawns, current_position.empty, color) | double_push(not_moved, current_position.empty, color) | attacks;
}

Bitboard ChessGame::rook_moves(Square square, int color)
{
	return rook_attacks(current_position.all_occupancy, square) & ~current_position.occupancy[color];
}

Bitboard ChessGame::bishop_moves(Square square, int color)
{
	return bishop_attacks(current_position.all_occupancy, square) & ~current_position.occupancy[color];
}

Bitboard ChessGame::queen_moves(Square square, int color)
{
	return queen_attacks(current_position.all_occupancy, square) & ~current_position.occupancy[color];
}

bool ChessGame::is_friendly_square(Square square)
{
    return bool((1ULL << square) & current_position.occupancy[current_position.color_to_move]);
}

bool ChessGame::is_attacked(Position position, Square square, int attacking_color)
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

bool ChessGame::is_king_in_check(Position position, int attacking_color)
{
    return is_attacked(position, (Square)bit_scan_forward(position.pieces[attacking_color ^ 1][KING]), attacking_color);
}

ChessGame::ChessGame(Position position)
{
	current_position = position;
	position_history_3fold[pos_stringid(position)] = 1;
	init_rays();
	update_occupancies(position);
}

ChessGame::ChessGame(std::string fen) : ChessGame(fen_to_pos(fen))
{
}

Bitboard ChessGame::moves(Square square, Position position)
{
    Bitboard sq = (1ULL << square);
    int color = (position.occupancy[BLACK] & sq) > 0;

	int type;
    for (type = PAWN; (type < KING) && !(sq & position.pieces[color][type]); ++type);

    Bitboard moves;

	switch (type)
	{
    case PAWN:
		moves = pawn_moves(square, color);
		break;
    case ROOK:
		moves = rook_moves(square, color);
		break;
    case KNIGHT:
		moves = knight_moves(square, color);
		break;
    case BISHOP:
		moves = bishop_moves(square, color);
		break;
    case QUEEN:
		moves = queen_moves(square, color);
		break;
    case KING:
		moves = king_moves(square, color);
		break;
	default:
		break;
	}

	return moves;
}

Bitboard ChessGame::legal_moves(Square square)
{
    legal_moves_list.clear();

    if (~current_position.all_occupancy & (1ULL << square)) return 0ULL;

    Bitboard peusdo_legal_moves = moves(square, current_position);
    Bitboard legal_moves = 0ULL;

	while (peusdo_legal_moves)
	{
        Square to_square = (Square)bit_scan_forward(peusdo_legal_moves);
		peusdo_legal_moves &= peusdo_legal_moves - 1;
		Position new_position = current_position;
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
            legal_moves_list.push_back(move);
        }
    }

	return legal_moves;
}

void ChessGame::test_move(Move& move, Position& position)
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
        position.pieces[color_to_move][ROOK] ^= (to_bb > from_bb ? (1ULL << ROOKS_KINGSIDE[color_to_move]) : (1ULL << ROOKS_QUEENSIDE[color_to_move])) | (to_bb > from_bb ? from_bb << 1 : from_bb >> 1);
    else if (move.captured_type != -1)
		position.pieces[color_to_move ^ 1][move.captured_type] ^= to_bb;

    update_castle_rights(position, move);
	update_occupancies(position);

    position.color_to_move = Color(color_to_move ^ 1);

	//print_position(current_position);
}

void ChessGame::start()
{
	std::string input;
	bool game_over = false;

	while (!game_over)
	{
		system("cls");
		print_board(current_position);
		std::cout << "=========== " << (current_position.color_to_move ? "black" : "white") << " to move" << " ===========\n";
		std::cout << "enter a square (e.g. a1): ";
		getline(std::cin, input);

		//system("cls");

		if (input.compare("h") == 0)
		{
			system("cls");
			std::cout << "help page\n";
			getline(std::cin, input);
		}
		else
		{
			int initial_square = 0x0;
			int final_square;
            Bitboard square_moves = 0x0;
			bool valid_square = (input.length() == 2) && islower(input[0]) && isdigit(input[1]) && ((initial_square = max_file * (input[1] - '1') + (input[0] - 'a')) >= 0) && (initial_square <= 63);

            if (valid_square && ((1ULL << initial_square) & current_position.occupancy[current_position.color_to_move]) && (square_moves = legal_moves((Square)initial_square)))
			{
				system("cls");

				print_board(current_position, square_moves);

				std::cout << "enter a destination square: ";

				getline(std::cin, input);

				valid_square = input.length() == 2 && islower(input[0]) && isdigit(input[1]) && (final_square = max_file * (input[1] - '1') + (input[0] - 'a')) >= 0 && final_square <= 63;

				if (valid_square && ((1ULL << final_square) & square_moves))
				{
                    Bitboard from_bb = 1ULL << initial_square;
                    Bitboard to_bb = 1ULL << final_square;
					Move move;
                    move.from = Square(initial_square);
                    move.to = Square(final_square);
					move.color = current_position.color_to_move;
					move.piece_type = get_type(initial_square, move.color);
					int captured_type = -1;
					if (to_bb & current_position.occupancy[current_position.color_to_move ^ 1])
					{
                        for (captured_type = PAWN; captured_type <= KING && !(to_bb & current_position.pieces[current_position.color_to_move ^ 1][captured_type]); ++captured_type);
					}
					move.captured_type = captured_type;

                    make_move(move);
					//update_game_status();

					//std::cout << "size: " << position_history_3fold.size() << std::endl;

					game_over = current_position.state == CHECKMATE || current_position.state == REPETITION || current_position.state == STALEMATE;
					//std::cout << current_position.state << std::endl;
					//print_bitboard(all_legal_moves(current_position, current_position.color_to_move));
					//print_bitboard(current_position.checking_path_bb);
				}
				else
				{
					message("invalid destination square");
				}
			}
			else
			{
				//std::cout << valid_square << std::endl;

				//print_bitboard((1ULL << initial_square) & current_position.piece_bitboards[current_position.color_to_move]);
				//print_bitboard(square_moves);

				//getline(std::cin, input);

				message("invalid initial square");
			}
		}
	}

	system("cls");
	print_board(current_position);

	switch (current_position.state)
	{
	case CHECKMATE:
		std::cout << "=========== " << (!current_position.color_to_move ? "black" : "white") << " won by checkmate" << " ===========\n";
		break;
	case STALEMATE:
		std::cout << "=========== draw by stalemate ===========\n";
		break;
	case REPETITION:
		std::cout << "=========== draw by 3-fold repetition ===========\n";
		break;
	default:
		break;
	}
}

int ChessGame::pop_count(Bitboard bitboard)
{
	int count = 0;
	while (bitboard) {
		count++;
		bitboard &= bitboard - 1;
	}
	return count;
}

void ChessGame::update_occupancies(Position& position) {
    position.occupancy[WHITE] = position.pieces[WHITE][PAWN] | position.pieces[WHITE][KNIGHT] | position.pieces[WHITE][BISHOP] | position.pieces[WHITE][ROOK] | position.pieces[WHITE][QUEEN] | position.pieces[WHITE][KING];
    position.occupancy[BLACK] = position.pieces[BLACK][PAWN] | position.pieces[BLACK][KNIGHT] | position.pieces[BLACK][BISHOP] | position.pieces[BLACK][ROOK] | position.pieces[BLACK][QUEEN] | position.pieces[BLACK][KING];
    position.all_occupancy = position.occupancy[WHITE] | position.occupancy[BLACK];
    position.empty = ~position.all_occupancy;
}

Bitboard ChessGame::castling_moves(Color color)
{
    if (is_king_in_check(current_position, color ^ 1)) return 0ULL;

    Bitboard occ = current_position.all_occupancy;
    Bitboard result = 0ULL;

    if (current_position.castling_rights[color] & KS)
    {
        if (!(occ & KSIDE_BLOCK_MASK[color]) &&
            !is_attacked(current_position, KSIDE_PASS_SQUARES[color][0], color ^ 1) &&
            !is_attacked(current_position, KSIDE_PASS_SQUARES[color][1], color ^ 1))
        {
            result |= (1ULL << KSIDE_KING_DEST[color]);
        }
    }

    if (current_position.castling_rights[color] & QS)
    {
        if (!(occ & QSIDE_BLOCK_MASK[color]) &&
            !is_attacked(current_position, QSIDE_PASS_SQUARES[color][0], color ^ 1) &&
            !is_attacked(current_position, QSIDE_PASS_SQUARES[color][1], color ^ 1))
        {
            result |= (1ULL << QSIDE_KING_DEST[color]);
        }
    }

    return result;
}

void ChessGame::update_castle_rights(Position& position, const Move &move)
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

void ChessGame::print_bitboard(Bitboard bitboard)
{
	for (int rank = max_rank - 1; rank >= 0; rank--)
	{
		std::cout << rank + 1 << "   ";

		for (int file = 0; file < max_file; file++)
		{
			int square = max_rank * rank + file;

			std::cout << " " << ((bitboard & (1ULL << square)) ? 1 : 0) << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "\n     a  b  c  d  e  f  g  h\n";
}

void ChessGame::print_board(Position position, Bitboard moves)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	std::cout << "\n  " << char(201) << char(205) << char(205) << char(205) << char(203)
		<< char(205) << char(205) << char(205) << char(203)
		<< char(205) << char(205) << char(205) << char(203)
		<< char(205) << char(205) << char(205) << char(203)
		<< char(205) << char(205) << char(205) << char(203)
		<< char(205) << char(205) << char(205) << char(203)
		<< char(205) << char(205) << char(205) << char(203)
		<< char(205) << char(205) << char(205) << char(187) << std::endl;

	for (int rank = max_rank - 1; rank >= 0; rank--)
	{
		std::cout << rank + 1 << ' ' << char(186);
		for (int file = 0; file < max_file; file++)
		{
            Bitboard sq_bitboard = (1ULL << (rank * max_rank + file));
			int piece_type;
            bool color = sq_bitboard & position.occupancy[BLACK];
			bool is_empty_sq = sq_bitboard & position.empty;
			std::cout << ' ';

			if (is_empty_sq)
			{
				if (sq_bitboard & moves)
				{
					std::cout << char(254) << ' ' << char(186);
					continue;
				}
				else
				{
					std::cout << "  " << char(186);
					continue;
				}
			}

            for (piece_type = PAWN; (piece_type <= KING) && !(sq_bitboard & position.pieces[color][piece_type]); ++piece_type);

			if (sq_bitboard & moves)
			{
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			}

			//std::cout << piece_type << std::endl;
            std::cout << (!color ? WHITE_PIECE_CHAR[piece_type] : BLACK_PIECE_CHAR[piece_type]) << ' ';

			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

			std::cout << char(186);
		}

		rank > 0 ? (std::cout << "\n  " << char(204) << char(205) << char(205) << char(205) << char(206)
			<< char(205) << char(205) << char(205) << char(206)
			<< char(205) << char(205) << char(205) << char(206)
			<< char(205) << char(205) << char(205) << char(206)
			<< char(205) << char(205) << char(205) << char(206)
			<< char(205) << char(205) << char(205) << char(206)
			<< char(205) << char(205) << char(205) << char(206)
			<< char(205) << char(205) << char(205) << char(185) << std::endl) : 
			(std::cout << "\n  " << char(200) << char(205) << char(205) << char(205) << char(202)
			<< char(205) << char(205) << char(205) << char(202)
			<< char(205) << char(205) << char(205) << char(202)
			<< char(205) << char(205) << char(205) << char(202)
			<< char(205) << char(205) << char(205) << char(202)
			<< char(205) << char(205) << char(205) << char(202)
			<< char(205) << char(205) << char(205) << char(202)
			<< char(205) << char(205) << char(205) << char(188) << std::endl);
	}
	std::cout << "    a   b   c   d   e   f   g   h\n";
}

void ChessGame::print_position(Position position)
{
	for (int k = 0; k < 2; ++k) {
		for (int i = 0; i < 6; ++i) {
			print_bitboard(position.pieces[k][i]);
		}
	}
	print_bitboard(position.empty);
	std::cout << position.color_to_move << std::endl;
}

bool ChessGame::is_valid_square(Square square)
{
    Bitboard bb = (1ULL << square);
    return bb && (current_position.occupancy[current_position.color_to_move] & bb);
}

void ChessGame::message(std::string message)
{
	std::string input;
	system("cls");
	std::cout << message << std::endl;
	getline(std::cin, input);
}

PieceType ChessGame::get_type(int square, int color)
{
    Bitboard square_bb = (1ULL << square);
	int type;
    for (type = 0; type < KING && !(square_bb & current_position.pieces[color][type]); ++type);
	return (PieceType)type;
}

void ChessGame::make_move(Move move)
{
    test_move(move, current_position);
}

//void ChessGame::update_game_status()
//{
//	std::cout << pos_stringid(current_position) << std::endl;
//	if ((position_history_3fold[pos_stringid(current_position)] += 1) > 2)
//	{
//		current_position.state = REPETITION;
//		return;
//	}
//
//	bool is_black = current_position.color_to_move;
//	int king_square = bit_scan_forward(current_position.pieces[is_black][KING]);
//	U64 king_check_mask = queen_moves_mask(king_square, current_position, is_black);
//
//	U64 rook_check;
//	U64 bishop_check;
//	U64 knight_check;
//
//	U64 checks = (rook_check = king_check_mask & rook_mask(king_square) & (current_position.pieces[!is_black][ROOK] | current_position.occupancy[QUEEN]))
//		| (bishop_check = king_check_mask & bishop_mask(king_square) & current_position.occupancy[!is_black] & (current_position.pieces[0][BISHOP] | current_position.pieces[1][BISHOP] | current_position.pieces[0][QUEEN] | current_position.pieces[1][QUEEN]))
//		| (knight_check = knight_moves_mask(king_square, current_position, is_black) & current_position.pieces[is_black][KNIGHT]);
//
//	bool king_can_move = king_moves(current_position, is_black);
//
//	if (checks)
//	{
//		bool double_check = checks & (checks - 1);
//		if (king_can_move)
//		{
//			current_position.state = CHECK;
//		}
//		else
//		{
//			if (double_check)
//			{
//				current_position.state = CHECKMATE;
//			}
//			else
//			{
//				U64 pin_path = 0ULL;
//				U64 moves_bb = 0ULL;
//
//				if (knight_check) pin_path = knight_check;
//				if (bishop_check) pin_path = king_check_mask & bishop_mask(king_square) & bishop_moves_mask(bit_scan_forward(bishop_check), current_position, !is_black) | bishop_check;
//				if (rook_check) pin_path = king_check_mask & rook_mask(king_square) & rook_moves_mask(bit_scan_forward(rook_check), current_position, !is_black) | rook_check;
//
//				for (int type = PAWN; type <= QUEEN; type++)
//				{
//					U64 type_bb = current_position.pieces[is_black][type];
//					while (type_bb)
//					{
//						int square = bit_scan_forward(type_bb);
//						moves_bb |= moves(square, current_position, is_black) & pin_path;
//						type_bb &= type_bb - 1;
//					}
//				}
//
//				current_position.state = moves_bb ? CHECK : CHECKMATE;
//				current_position.checking_path_bb = pin_path;
//			}
//		}
//	}
//	else if (!king_can_move)
//	{
//		U64 bb = 0ULL;
//
//		for (int type = PAWN; type <= QUEEN; type++)
//		{
//			U64 type_bb = current_position.pieces[is_black][type];
//			while (type_bb)
//			{
//				int square = bit_scan_forward(type_bb);
//
//				bb |= moves(square, current_position, is_black);
//
//				type_bb &= type_bb - 1;
//			}
//		}
//
//		current_position.state = bb ? NORMAL : STALEMATE;
//	}
//	else
//	{
//		current_position.state = NORMAL;
//	}
//}

ChessGame::Position ChessGame::fen_to_pos(std::string fen)
{
	ChessGame::Position position = {};

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

std::string ChessGame::pos_stringid(Position position)
{
	std::string stringid = "";
    for (int type = PAWN; type <= KING; type++)
	{
        stringid += std::to_string(position.pieces[WHITE][type] | position.pieces[BLACK][type]);
	}
	stringid += std::to_string(position.color_to_move);

	return stringid;
}

/*
	Magic Bitboard implementation (WIP)
*/

/*
void ChessGame::init_magics(bool is_rook, U64 piece_table[], Magic magics[])
{
	int size = 0;
	U64 b = 0;
	U64 occ[4096];
	U64 ref[4096];
	for (int sq = a1; sq <= h8; sq++)
	{
		U64 edges_ex = (((fifth_rank | eighth_rank) & ~rank_mask(sq)) | ((a_file | h_file) & ~file_mask(sq)));

		Magic& m = magics[sq];
		m.mask = (is_rook ? rook_mask_ex(sq) : bishop_mask_ex(sq)) & ~edges_ex;
		m.shift = 64 - pop_count(m.mask);
		m.attacks = sq == a1 ? piece_table : magics[sq - 1].attacks + size;

		b = size = 0;

		do
		{
			occ[size] = b;
			ref[size] = is_rook ? rook_attack(sq, b) : bishop_attack(sq, b);
			size++;
			b = (b - m.mask) & m.mask;
		} while (b);
	}
}
*/
