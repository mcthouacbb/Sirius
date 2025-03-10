#pragma once

#include "../defs.h"

namespace eval
{

#define S(mg, eg) PackedScore(mg, eg)

constexpr PackedScore MATERIAL[6] = {S(  61,  132), S( 291,  444), S( 310,  453), S( 386,  813), S( 805, 1497), S(0, 0)};

constexpr PackedScore PSQT[6][64] = {
	{
		S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
		S(  67,   75), S(  28,   92), S(  28,   95), S(  52,   76), S(  61,   69), S(  31,   94), S(  29,  104), S(  64,   82),
		S(  10,    9), S(   1,   38), S(  14,   -1), S(  31,  -33), S(  18,  -27), S(   5,   -4), S( -16,   31), S(  -3,   16),
		S(  16,    9), S(   5,   14), S(   8,   -8), S(  12,  -24), S(   0,  -22), S(  -3,   -9), S(  -9,   14), S( -13,   12),
		S(   9,    0), S(   3,   18), S(   8,  -15), S(  16,  -15), S(  10,  -14), S(  -3,   -8), S( -15,   16), S( -14,    2),
		S(   2,   -2), S(  11,   13), S(  17,   -4), S(  -1,    1), S(  -6,    0), S( -12,   -4), S( -23,   13), S( -26,    0),
		S(  16,   -3), S(  30,    8), S(  15,   -8), S(   7,    5), S(  -6,   -1), S(  -6,   -4), S( -14,   18), S( -17,    4),
		S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
	},
	{
		S( -75,  -99), S( -79,  -26), S( -27,  -17), S(  11,  -14), S( -32,    2), S( -67,    6), S(-103,    8), S( -88,  -55),
		S( -12,  -15), S(  22,    0), S(  29,   -5), S(   7,    3), S(   8,    8), S(   2,   12), S(  -4,   18), S( -10,    2),
		S(  13,   -8), S(   6,    0), S(  48,  -11), S(  39,    7), S(  22,   15), S(  12,   12), S(   2,    6), S(  -8,    4),
		S(  32,    0), S(  33,    8), S(  43,   11), S(  31,   18), S(  24,   24), S(  31,   12), S(  19,    5), S(   8,   13),
		S(  10,   15), S(  16,   15), S(  24,   16), S(  18,   27), S(  24,   18), S(  18,   18), S(  20,    3), S(   5,    6),
		S(  12,   -1), S(  17,    2), S(  16,   -2), S(  12,   16), S(   5,   14), S(   1,    0), S(   2,   -5), S( -11,   -4),
		S(   4,    1), S(  13,   -8), S(   4,   -5), S(   9,    0), S(   3,    0), S(  -7,   -2), S(  -8,   -5), S( -20,   -5),
		S( -11,  -18), S( -14,   -6), S(  -6,   -9), S(   1,    2), S(  -6,    0), S( -21,   -7), S( -16,  -11), S( -47,   -7),
	},
	{
		S( -46,  -13), S( -30,    4), S(  -4,   -3), S( -75,   13), S( -64,   16), S( -52,    4), S( -24,    7), S( -33,    9),
		S( -22,   -7), S( -49,    7), S( -11,    2), S(   1,   -2), S( -17,    9), S(   4,   -1), S( -15,   -4), S(  -4,   -4),
		S(  15,   11), S(  13,    3), S(  26,    5), S(  13,    8), S(  19,    4), S(  -2,    1), S(   9,    8), S(   5,    3),
		S(  -8,    5), S(   9,    8), S(   9,   14), S(  20,   17), S(  21,   24), S(  18,    9), S(   5,   12), S(  -8,    7),
		S(  17,   -6), S(   2,   10), S(   8,   11), S(  11,   17), S(  21,   13), S(   1,   17), S(   0,    8), S(   1,    0),
		S(  21,  -11), S(  25,   -6), S(   5,   -4), S(   9,   12), S(   0,    8), S(   1,   -1), S(  15,    3), S(   2,   -7),
		S(  27,  -21), S(  19,  -22), S(  24,   -9), S(   5,   -3), S(  -2,    1), S(  10,  -16), S(   2,  -23), S(  12,  -11),
		S(  24,  -40), S(  15,  -15), S( -10,    2), S(  15,   -4), S(   0,   -3), S(  -2,    3), S(  12,   -4), S(   5,  -18),
	},
	{
		S(  14,    7), S( -25,   35), S( -23,   28), S( -11,   17), S( -27,   26), S( -23,   31), S(  -8,   22), S(  -3,   19),
		S(  31,    1), S(  31,    8), S(  11,   15), S(   7,   15), S(  16,   17), S(   3,   29), S(  -1,   25), S(   3,   16),
		S(   8,    1), S(  33,    2), S(  15,    3), S(  24,    1), S(   2,   11), S(   0,   19), S(  10,   16), S(  -9,   18),
		S(   3,    5), S(   3,   12), S(   9,    8), S(   8,    0), S(   2,    9), S(   6,   13), S(   4,   14), S(  -7,   15),
		S(  -9,   -1), S(  12,    3), S( -10,    7), S(   6,    0), S(  -5,    3), S(  -8,    6), S( -10,    6), S( -13,    6),
		S(   9,  -24), S(  24,  -26), S(  -1,  -15), S(   2,  -18), S( -10,  -10), S( -12,  -11), S( -11,  -12), S( -17,   -9),
		S(  -7,  -28), S(  22,  -36), S(   2,  -23), S(   3,  -22), S(  -3,  -19), S(  -4,  -16), S( -14,  -14), S( -16,  -20),
		S(  -7,  -34), S(  -5,  -17), S(  -1,  -20), S(   8,  -27), S(   1,  -23), S(  -6,  -16), S(  -6,  -20), S(  -7,  -17),
	},
	{
		S(  -4,   -8), S(  37,  -36), S( -23,   31), S( -21,   23), S(  11,  -10), S( -21,   12), S( -40,    8), S( -28,    8),
		S(  40,    3), S(   2,   14), S( -19,   32), S( -30,   52), S( -20,   36), S(  -4,    8), S(  -2,  -10), S(   7,   -8),
		S(  14,    6), S(   7,   13), S(   0,   20), S( -12,   28), S(   3,   17), S(   8,   10), S(   6,   -8), S(  18,  -12),
		S(   8,   14), S(   4,   31), S(   8,    8), S( -10,   31), S(  -6,   27), S(   8,    5), S(   5,   13), S(   5,    4),
		S(  11,    7), S(  13,   11), S(   1,   14), S(   3,   24), S(  -5,   32), S(  -2,   21), S(   3,   16), S(   2,    5),
		S(  16,  -21), S(  15,   -6), S(   5,    4), S(  -6,   13), S(  -4,   11), S(  -2,   13), S(  10,   -9), S(  11,  -22),
		S(  10,  -49), S(  12,  -63), S(   8,  -40), S(   5,  -12), S(   4,   -6), S(   5,  -18), S(   4,  -23), S(   9,  -37),
		S(   6,  -50), S( -14,  -45), S( -19,  -24), S(  -4,  -24), S(  -3,  -19), S(  -7,  -17), S( -10,  -19), S( -11,  -18),
	},
	{
		S( -38,  -52), S( -44,   14), S( -79,    9), S( -67,    6), S(   8,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6),
		S( -74,   34), S( -26,   63), S( -32,   49), S(  14,   21), S(   8,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6),
		S(   9,   17), S(  32,   50), S(  39,   40), S(  16,   40), S(   8,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6),
		S( -33,   16), S( -11,   44), S(  -5,   35), S( -27,   31), S(   8,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6),
		S( -48,    5), S( -20,   28), S(  -5,   16), S( -26,   14), S(   8,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6),
		S( -11,  -14), S(  15,    5), S(   2,   -4), S(  -5,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6),
		S(  27,  -31), S(  27,   -7), S(   9,  -16), S(  -3,  -20), S(   8,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6),
		S(  15,  -61), S(  22,  -26), S(   1,  -29), S(  10,  -48), S(   8,   -6), S(   8,   -6), S(   8,   -6), S(   8,   -6),
	},
};

constexpr PackedScore MOBILITY[4][28] = {
	{S( -17,  -27), S( -38,  -49), S( -17,  -17), S(  -9,    0), S(   2,    9), S(   7,   19), S(  15,   22), S(  24,   26), S(  35,   19), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0)},
	{S( -26,  -44), S( -35,  -59), S( -21,  -30), S( -14,  -12), S(  -4,   -2), S(   1,    8), S(   4,   16), S(   8,   20), S(   9,   22), S(  12,   22), S(  14,   22), S(  20,   16), S(  17,   24), S(  27,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0)},
	{S( -26,  -52), S( -32,  -75), S( -19,  -50), S(  -6,  -29), S(  -3,  -14), S(  -4,   -3), S(  -3,    3), S(   0,   10), S(   3,   13), S(   7,   18), S(   6,   27), S(   9,   33), S(  12,   36), S(  17,   36), S(  26,   33), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0)},
	{S( -33,   18), S( -64,  -68), S( -90, -108), S( -31, -198), S( -28,  -71), S( -21,  -11), S( -12,  -21), S( -10,    0), S(  -9,   19), S(  -6,   28), S(  -4,   31), S(   0,   33), S(   0,   44), S(   4,   41), S(   5,   46), S(   7,   46), S(   8,   49), S(  11,   49), S(  10,   50), S(  16,   42), S(  24,   32), S(  33,   15), S(  37,   15), S(  47,   -5), S(  59,  -12), S(  54,  -21), S(  34,  -16), S( -14,  -33)}
};

constexpr PackedScore THREAT_BY_PAWN[6] = {S(   5,  -22), S(  66,   30), S(  61,   61), S(  78,   27), S(  74,   -8), S(   0,    0)};
constexpr PackedScore THREAT_BY_KNIGHT[2][6] = {
	{S(   7,   29), S(  22,   30), S(  40,   45), S(  73,   15), S(  56,  -25), S(   0,    0)},
	{S(  -7,    8), S(  10,   28), S(  28,   29), S(  64,   34), S(  57,    0), S(   0,    0)}
};
constexpr PackedScore THREAT_BY_BISHOP[2][6] = {
	{S(  -2,   37), S(  39,   35), S(  -9,   28), S(  68,   17), S(  67,   60), S(   0,    0)},
	{S(  -4,    6), S(  18,   22), S( -23,  -20), S(  44,   47), S(  45,  124), S(   0,    0)}
};
constexpr PackedScore THREAT_BY_ROOK[2][6] = {
	{S(   0,   43), S(  18,   61), S(  26,   57), S(   9,  -44), S(  74,   12), S(   0,    0)},
	{S(  -7,    7), S(   3,   14), S(  13,    1), S(   9,  -90), S(  65,   47), S(   0,    0)}
};
constexpr PackedScore THREAT_BY_QUEEN[2][6] = {
	{S(   6,   10), S(  23,   20), S(   9,   49), S(  19,   15), S(   9,  -49), S(  97,   48)},
	{S(  -2,   10), S(   0,    5), S(  -8,   17), S(  -5,    3), S( -17,  -73), S( 113,   67)}
};
constexpr PackedScore THREAT_BY_KING[6] = {S( -24,   48), S(   5,   52), S(  32,   42), S(  74,    9), S(   0,    0), S(   0,    0)};
constexpr PackedScore PUSH_THREAT = S(  14,   20);

constexpr PackedScore ISOLATED_PAWN[8] = {S(  -8,    6), S(  -4,  -16), S( -12,   -9), S( -11,  -16), S( -11,  -16), S(  -7,   -8), S(  -1,  -15), S( -10,    7)};
constexpr PackedScore DOUBLED_PAWN[8] = {S(   1,  -63), S(  12,  -38), S(   1,  -29), S(   1,  -21), S(  -3,  -14), S( -11,  -20), S(   6,  -42), S(   7,  -75)};
constexpr PackedScore BACKWARDS_PAWN[8] = {S(   0,    0), S(  -8,  -14), S(  -3,  -14), S(  -8,  -12), S(   1,  -16), S(  30,   -8), S(   0,    0), S(   0,    0)};
constexpr PackedScore PAWN_PHALANX[8] = {S(   0,    0), S(   4,   -4), S(  11,   -3), S(  20,    7), S(  40,   40), S(  99,  225), S( -21,  348), S(   0,    0)};
constexpr PackedScore DEFENDED_PAWN[8] = {S(   0,    0), S(   0,    0), S(  19,    7), S(  11,    7), S(  16,   21), S(  28,   64), S( 132,   77), S(   0,    0)};
constexpr PackedScore CANDIDATE_PASSER[2][8] = {
	{S(   0,    0), S( -37,  -14), S( -20,   -7), S(   0,   28), S(  28,   56), S(  60,   92), S(   0,    0), S(   0,    0)},
	{S(   0,    0), S( -20,   -7), S(  -9,   14), S(  -5,   31), S(  20,   44), S(  34,  159), S(   0,    0), S(   0,    0)}
};

constexpr PackedScore PASSED_PAWN[2][2][8] = {
	{
		{S(   0,    0), S(   0,    0), S(   0,    0), S( -38,  -42), S( -17,   25), S(  -4,  158), S(  38,  233), S(   0,    0)},
		{S(   0,    0), S(   0,    0), S(   0,    0), S( -25,  -55), S(   7,  -22), S(  30,   25), S(  58,   35), S(   0,    0)}
	},
	{
		{S(   0,    0), S(   0,    0), S(   0,    0), S( -29,  -58), S(   1,  -24), S(  33,   13), S(  67,   -1), S(   0,    0)},
		{S(   0,    0), S(   0,    0), S(   0,    0), S( -30,  -64), S(   2,  -35), S(  10,   -7), S(  -7,  -14), S(   0,    0)}
	}
};
constexpr PackedScore OUR_PASSER_PROXIMITY[8] = {S(   0,    0), S(  37,   86), S(   5,   70), S(   3,   44), S(   6,   32), S(  12,   24), S(  22,   21), S(  17,   19)};
constexpr PackedScore THEIR_PASSER_PROXIMITY[8] = {S(   0,    0), S( -40,    9), S(  21,    1), S(  15,   28), S(  20,   47), S(  14,   80), S(  16,   80), S(  17,   69)};

constexpr PackedScore PAWN_STORM[2][4][8] = {
	{
		{S(  15,   15), S(-132,  -48), S( -10,  -19), S(  11,  -17), S(   1,    1), S(  -5,   10), S(  -4,   10), S(   0,    0)},
		{S(  10,    4), S( -23, -100), S(  40,  -48), S(   7,  -14), S(   0,   -5), S( -12,    6), S(  -9,    9), S(   0,    0)},
		{S(  -4,    7), S(  59,  -99), S(  64,  -48), S(  21,  -18), S(   1,    1), S(  -8,    8), S( -10,   14), S(   0,    0)},
		{S(   8,   -6), S( 113,  -87), S(  36,  -23), S(  13,   -1), S(   0,    5), S(  -7,    0), S(  -4,    3), S(   0,    0)}
	},
	{
		{S(   0,    0), S(   0,    0), S( -12,   32), S( -20,    7), S( -10,   10), S(   8,    7), S(  -5,   10), S(   0,    0)},
		{S(   0,    0), S(   0,    0), S(  27,   -3), S( -15,    1), S(  -7,    9), S(  -7,   12), S( -10,    8), S(   0,    0)},
		{S(   0,    0), S(   0,    0), S(  42,  -17), S(  -8,   -6), S(  -2,    6), S(   5,    9), S(  -5,   10), S(   0,    0)},
		{S(   0,    0), S(   0,    0), S(   4,    0), S(   7,    5), S(  -8,    3), S(  -3,   -1), S(   5,   -6), S(   0,    0)}
	}
};
constexpr PackedScore PAWN_SHIELD[4][8] = {
	{S(  17,    1), S(  -7,   24), S( -14,   15), S(  -2,    8), S(   0,    2), S( -39,  -22), S( -78,  -35), S(   0,    0)},
	{S(  16,    0), S( -13,   11), S( -10,    8), S(  11,   -3), S(   8,  -12), S( -14,  -16), S( -55,  -29), S(   0,    0)},
	{S(   5,    8), S( -16,    8), S(  16,    5), S(   2,   -2), S(  -3,   -1), S(   3,  -20), S( -15,  -42), S(   0,    0)},
	{S(  13,   -4), S(   4,   -3), S(  -3,   -2), S(   2,   -5), S(   6,   -8), S(  28,  -21), S( -19,  -19), S(   0,    0)}
};
constexpr PackedScore SAFE_KNIGHT_CHECK = S(  91,   -2);
constexpr PackedScore SAFE_BISHOP_CHECK = S(  17,   22);
constexpr PackedScore SAFE_ROOK_CHECK = S(  72,    9);
constexpr PackedScore SAFE_QUEEN_CHECK = S(  29,   31);
constexpr PackedScore UNSAFE_KNIGHT_CHECK = S(  10,    0);
constexpr PackedScore UNSAFE_BISHOP_CHECK = S(  16,   10);
constexpr PackedScore UNSAFE_ROOK_CHECK = S(  21,   -1);
constexpr PackedScore UNSAFE_QUEEN_CHECK = S(   6,   10);
constexpr PackedScore KING_ATTACKER_WEIGHT[4] = {S(  17,   -3), S(  13,   -3), S(  14,  -15), S(   4,    4)};
constexpr PackedScore KING_ATTACKS[14] = {S( -38,    8), S( -44,    6), S( -49,    4), S( -52,    7), S( -51,    6), S( -44,    4), S( -29,    0), S( -11,   -9), S(  17,  -20), S(  38,  -15), S(  58,  -27), S(  60,   19), S(  86,  -22), S(  72,   26)};
constexpr PackedScore WEAK_KING_RING[17] = {S( -30,   48), S( -29,   27), S( -24,   24), S( -21,   20), S( -14,   13), S(  -5,    7), S(   5,    1), S(  20,   -3), S(  36,   -6), S(  50,   -9), S(  62,  -12), S(  65,   -8), S(  77,  -11), S(  79,  -13), S(  78,   -9), S(  98,  -14), S( 107,  -15)};

constexpr PackedScore MINOR_BEHIND_PAWN = S(   6,   13);
constexpr PackedScore KNIGHT_OUTPOST = S(  23,   17);
constexpr PackedScore BISHOP_PAWNS[7] = {S(   4,   21), S(   6,   20), S(   4,   12), S(   2,    4), S(  -2,   -5), S(  -2,  -18), S(  -6,  -30)};
constexpr PackedScore BISHOP_PAIR = S(  20,   62);
constexpr PackedScore LONG_DIAG_BISHOP = S(  16,    8);
constexpr PackedScore ROOK_OPEN[2] = {S(  26,    3), S(  14,    4)};

constexpr PackedScore TEMPO = S(  32,   36);

constexpr PackedScore COMPLEXITY_PAWNS = S(   0,    8);
constexpr PackedScore COMPLEXITY_PASSERS = S(   0,    1);
constexpr PackedScore COMPLEXITY_PAWNS_BOTH_SIDES = S(   0,   69);
constexpr PackedScore COMPLEXITY_PAWN_ENDGAME = S(   0,   65);
constexpr PackedScore COMPLEXITY_OFFSET = S(   0, -132);

#undef S


}