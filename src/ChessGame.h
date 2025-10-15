#pragma once
#include <string>
#include <map>
#include <vector>

typedef unsigned long long U64;

enum PieceType
{
    nPawn = 0,		// all pawns
    nRook = 1,		// all rooks
    nKnight = 2,	// all knights
    nBishop = 3,	// all bishops
    nQueen = 4,		// all queens
    nKing = 5		// all kings
};

enum enumColor
{
    white,
    black
};

enum enumSquare
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

class ChessGame
{
public:
    enum enumGameState
	{
		NORMAL,
		CHECK,
		CHECKMATE,
		REPETITION,
		STALEMATE
	};

    enum enumIncludeMove
	{
		EMPTY	= 0x01,
		CAPTURE = 0x02,
		DEFEND	= 0x04
	};

    enum Direction {
		NORTH		= 0,
		NORTHEAST	= 1,
		EAST		= 2,
		SOUTHEAST	= 3,
		SOUTH		= 4,
		SOUTHWEST	= 5,
		WEST		= 6,
		NORTHWEST	= 7
	};

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

	U64 ray_attacks[8][64];

	struct Position
	{
		U64 pieces[2][6];
		U64 occupancy[2];
		U64 all_occupancy;
		U64 empty;
		enumColor color_to_move = white;
		enumGameState state = NORMAL;
		U64 checking_path_bb = 0ULL;
	};

	struct Move
	{
		enumSquare from;
		enumSquare to;
		PieceType piece_type;
		enumColor color;
		int captured_type;
		int promotion = -1; // -1 means no promotion
		bool is_castling = false;
		bool is_en_passant = false;
	};

	std::vector<Position> position_history;
    std::vector<Move> legal_moves_list;

	const static char white_piece_char[6];
	const static char black_piece_char[6];

	// De Bruijn sequence to 64-index mapping
	const static int index64[64];

	// De Bruijn sequence over alphabet {0, 1}. B(2, 6)
	const static U64 debruijn64 = 0x03f79d71b4cb0a89;

	const static U64 rank_mask_west_offsets[8];
	const static U64 file_mask_north_offsets[8];

	const static int max_rank = 8;
	const static int max_file = 8;

	const static U64 a_file = 0x0101010101010101;
	const static U64 h_file = 0x8080808080808080;
	const static U64 first_rank = 0x00000000000000FF;
	const static U64 eighth_rank = 0xFF00000000000000;
	const static U64 second_rank = 0x000000000000FF00ULL;
	const static U64 seventh_rank = 0x00FF000000000000ULL;
	const static U64 a1_h8 = 0x8040201008040201;
	const static U64 a8_h1 = 0x0102040810204080;
	const static U64 ab_file = a_file | (a_file << 1);
	const static U64 gh_file = h_file | (h_file >> 1);
	const static U64 forth_rank = first_rank << 24;
	const static U64 fifth_rank = forth_rank << 8;

	const static int rook_direction[4];
	const static int bishop_direction[4];

	ChessGame(Position position = starting_position);
	ChessGame(std::string fen);

	std::map<std::string, int> position_history_3fold;

	Position current_position{};

	void start();
	void message(std::string);
	void init_rays();
	
	U64 get_positive_ray_attacks(U64 occupied, Direction dir, enumSquare square);
	U64 get_negative_ray_attacks(U64 occupied, Direction dir, enumSquare square);
	U64 diagonal_attacks(U64 occupied, enumSquare square);
	U64 anti_diagonal_attacks(U64 occupancy, enumSquare square);
	U64 file_attacks(U64 occupancy, enumSquare square);
	U64 rank_attacks(U64 occupancy, enumSquare square);
	U64 rook_attacks(U64 occupancy, enumSquare square);
	U64 bishop_attacks(U64 occupancy, enumSquare square);
	U64 queen_attacks(U64 occupancy, enumSquare square);
	U64 knight_attacks(U64 knights);
	U64 knight_moves(enumSquare square, int color);
	U64 king_attacks(U64 king);
	U64 king_moves(enumSquare square, int color);
	U64 static single_push(U64 pawns, U64 empty, int color);
	U64 static double_push(U64 pawns, U64 empty, int color);
	U64 pawn_attacks(U64 square, int color);
	
	
	U64 pawn_moves(enumSquare square, int color);
	U64 rook_moves(enumSquare square, int color);
	U64 bishop_moves(enumSquare square, int color);
	U64 queen_moves(enumSquare square, int color);

    bool is_friendly_square(enumSquare square);
	bool is_attacked(Position position, enumSquare square, int attacking_color);
	bool is_king_in_check(Position position, int attacking_color);

    inline static U64 to_bb(int sq) { return 1ULL << sq; }
	PieceType get_type(int square, int color);
    void make_move(Move move);

	inline static U64 piece_bb(const Position& pos, int color, int pt) { return pos.pieces[color][pt]; }
	inline static U64 color_bb(const Position& pos, int color) { return pos.occupancy[color]; }
	inline static U64 opponent_bb(const Position& pos, int color) { return pos.occupancy[!color]; }
	void static update_occupancies(Position& pos) { 
		pos.occupancy[white] = pos.pieces[white][nPawn] | pos.pieces[white][nKnight] | pos.pieces[white][nBishop] | pos.pieces[white][nRook] | pos.pieces[white][nQueen] | pos.pieces[white][nKing];
		pos.occupancy[black] = pos.pieces[black][nPawn] | pos.pieces[black][nKnight] | pos.pieces[black][nBishop] | pos.pieces[black][nRook] | pos.pieces[black][nQueen] | pos.pieces[black][nKing];
		pos.all_occupancy = pos.occupancy[white] | pos.occupancy[black];
		pos.empty = ~pos.all_occupancy;
	}

	static Position fen_to_pos(std::string fen);
	static std::string pos_stringid(Position position);
	inline static U64 get_bit(U64 bitboard, int square) { return bitboard &= (1ULL << square); }
	inline static void set_bit(U64& bitboard, int square) { bitboard |= (1ULL << square); }
	inline static U64 bitboard_union(U64 bitboard1, U64 bitboard2) { return bitboard1 | bitboard2; }
	inline static int bit_scan_forward(U64 bitboard) { return bitboard ? (index64[((bitboard ^ (bitboard - 1)) * debruijn64) >> 58]) : -1; }
	inline static int bit_scan_reverse(U64 bitboard) {
		bitboard |= bitboard >> 1;
		bitboard |= bitboard >> 2;
		bitboard |= bitboard >> 4;
		bitboard |= bitboard >> 8;
		bitboard |= bitboard >> 16;
		bitboard |= bitboard >> 32;
		return index64[(bitboard * debruijn64) >> 58];
	}
	inline static U64 north_one(U64 bitboard) { return (bitboard << 8); }
	inline static U64 north_east_one(U64 bitboard) { return (bitboard << 9) & ~a_file; }
	inline static U64 east_one(U64 bitboard) { return (bitboard << 1) & ~a_file; }
	inline static U64 south_east_one(U64 bitboard) { return (bitboard >> 7) & ~a_file; }
	inline static U64 south_one(U64 bitboard) { return (bitboard >> 8); }
	inline static U64 south_west_one(U64 bitboard) { return (bitboard >> 9) & ~h_file; }
	inline static U64 west_one(U64 bitboard) { return (bitboard >> 1) & ~h_file; }
	inline static U64 north_west_one(U64 bitboard) { return (bitboard << 7) & ~h_file; }
	
	U64 moves(enumSquare square, Position position);
	U64 legal_moves(enumSquare square);

	static int pop_count(U64 bitboard);

	void static print_bitboard(U64 bitboard);
	void static print_board(Position position, U64 moves = 0x0);
	void static print_position(Position position);

    bool is_valid_square(enumSquare square);

	const static Position starting_position;

private:
    void test_move(Move move, Position& position);

	// Magic Bitnboard implementation (WIP)
	
	//static void init_magics(bool is_rook, U64 piece_table[], Magic magics[]);
	//struct Magic
	//{
	//	U64* attacks;
	//	U64 mask;
	//	U64 magic;
	//	int shift;

	//	unsigned index(U64 occ)
	//	{
	//		return unsigned(((occ & mask) * magic) >> shift);
	//	}
	//};

	//U64 rook_table[0x19000];
	//U64 bishop_table[0x1480];

	//Magic rook_magics[64];
	//Magic bishop_magics[64];
};

