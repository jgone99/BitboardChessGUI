#pragma once
#include <string>
#include <map>
#include <vector>

typedef unsigned long long Bitboard;


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

class ChessGame
{
public:
    enum GameState
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

    // For initial pre-computation of ray_attacks
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

    const Square ROOKS_KINGSIDE[2] { h1, h8 };
    const Square ROOKS_QUEENSIDE[2] { a1, a8 };
    const Bitboard KSIDE_BLOCK_MASK[2] = { (1ULL << f1) | (1ULL << g1), (1ULL << f8) | (1ULL << g8) };
    const Bitboard QSIDE_BLOCK_MASK[2] = { (1ULL << b1) | (1ULL << c1) | (1ULL << d1), (1ULL << b8) | (1ULL << c8) | (1ULL << d8) };

    // Squares the king passes through that must not be attacked
    const Square KSIDE_PASS_SQUARES[2][2] = { { f1, g1 }, { f8, g8 } };
    const Square QSIDE_PASS_SQUARES[2][2] = { { d1, c1 }, { d8, c8 } };

    const Square KSIDE_KING_DEST[2] = { g1, g8 };
    const Square QSIDE_KING_DEST[2] = { c1, c8 };

    // Pre-computed ray attacks for sliding pieces in all 8 directions
    Bitboard ray_attacks[8][64];

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

    const static Position starting_position;

	std::vector<Position> position_history;
    std::vector<Move> legal_moves_list;

    // For console game. printable piece characters
    const static char WHITE_PIECE_CHAR[6];
    const static char BLACK_PIECE_CHAR[6];

	// De Bruijn sequence to 64-index mapping
    const static int INDEX64[64];

	// De Bruijn sequence over alphabet {0, 1}. B(2, 6)
    const static Bitboard DEBRUIJN64 = 0x03f79d71b4cb0a89;

    // Useful constants
    const static int max_rank = 8;
    const static int max_file = 8;
    const Bitboard A_FILE = 0x0101010101010101;
    const Bitboard H_FILE = 0x8080808080808080;
    const Bitboard FIRST_RANK = 0x00000000000000FF;
    const Bitboard EIGHT_RANK = 0xFF00000000000000;
    const Bitboard SECOND_RANK = 0x000000000000FF00ULL;
    const Bitboard SEVENTH_RANK = 0x00FF000000000000ULL;
    const Bitboard A1_H8 = 0x8040201008040201;
    const Bitboard A8_H1 = 0x0102040810204080;
    const Bitboard AB_FILE = A_FILE | (A_FILE << 1);
    const Bitboard GH_FILE = H_FILE | (H_FILE >> 1);
    const Bitboard FORTH_RANK = FIRST_RANK << 24;
    const Bitboard FIFTH_RANK = FORTH_RANK << 8;

    std::map<std::string, int> position_history_3fold;
    Position current_position;

	ChessGame(Position position = starting_position);
	ChessGame(std::string fen);

	void start();
	void message(std::string);

    // Optimizatin Option: pre-compute with magic bitboards
    // Pre-computes the attacking rays for the sliding pieces. A middle-ground approach between a per-move calculation and a magic bitboard solution
	void init_rays();
    bool is_friendly_square(Square square);
    bool is_valid_square(Square square);
    PieceType get_type(int square, int color);
    void make_move(Move move);

    // Update occupancy bitboards when position changes
    void static update_occupancies(Position& position);
    Bitboard castling_moves(Color color);
    void update_castle_rights(Position& position, const Move& move);
	static Position fen_to_pos(std::string fen);
	static std::string pos_stringid(Position position);

    // Generate legal moves of a piece given a square
    Bitboard legal_moves(Square square);

    inline static void set_bit(Bitboard& bitboard, int square) { bitboard |= (1ULL << square); }

    void static print_bitboard(Bitboard bitboard);
    void static print_board(Position position, Bitboard moves = 0x0);
	void static print_position(Position position);

private:
    inline Bitboard to_bb(int sq) { return 1ULL << sq; }
    inline Bitboard piece_bb(const Position& pos, int color, int pt) { return pos.pieces[color][pt]; }
    inline Bitboard color_bb(const Position& pos, int color) { return pos.occupancy[color]; }
    inline Bitboard opponent_bb(const Position& pos, int color) { return pos.occupancy[!color]; }
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

    inline Bitboard north_one(Bitboard bitboard) { return (bitboard << 8); }
    inline Bitboard north_east_one(Bitboard bitboard) { return (bitboard << 9) & ~A_FILE; }
    inline Bitboard east_one(Bitboard bitboard) { return (bitboard << 1) & ~A_FILE; }
    inline Bitboard south_east_one(Bitboard bitboard) { return (bitboard >> 7) & ~A_FILE; }
    inline Bitboard south_one(Bitboard bitboard) { return (bitboard >> 8); }
    inline Bitboard south_west_one(Bitboard bitboard) { return (bitboard >> 9) & ~H_FILE; }
    inline Bitboard west_one(Bitboard bitboard) { return (bitboard >> 1) & ~H_FILE; }
    inline Bitboard north_west_one(Bitboard bitboard) { return (bitboard << 7) & ~H_FILE; }

    int pop_count(Bitboard bitboard);
    Bitboard get_positive_ray_attacks(Bitboard occupied, Direction dir, Square square);
    Bitboard get_negative_ray_attacks(Bitboard occupied, Direction dir, Square square);
    Bitboard diagonal_attacks(Bitboard occupied, Square square);
    Bitboard anti_diagonal_attacks(Bitboard occupancy, Square square);
    Bitboard single_push(Bitboard pawns, Bitboard empty, int color);
    Bitboard double_push(Bitboard pawns, Bitboard empty, int color);

    Bitboard file_attacks(Bitboard occupancy, Square square);
    Bitboard rank_attacks(Bitboard occupancy, Square square);

    // Piece attacks
    Bitboard rook_attacks(Bitboard occupancy, Square square);
    Bitboard bishop_attacks(Bitboard occupancy, Square square);
    Bitboard queen_attacks(Bitboard occupancy, Square square);
    Bitboard knight_attacks(Bitboard knights);
    Bitboard king_attacks(Bitboard king);
    Bitboard pawn_attacks(Bitboard square, int color);

    // Piece moves
    Bitboard pawn_moves(Square square, int color);
    Bitboard rook_moves(Square square, int color);
    Bitboard bishop_moves(Square square, int color);
    Bitboard knight_moves(Square square, int color);
    Bitboard queen_moves(Square square, int color);
    Bitboard king_moves(Square square, int color);

    Bitboard moves(Square square, Position position);
    void test_move(Move& move, Position& position);
    bool is_attacked(Position position, Square square, int attacking_color);
    bool is_king_in_check(Position position, int attacking_color);

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

