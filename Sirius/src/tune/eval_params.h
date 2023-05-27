#pragma once

#include "../board.h"

#include "../defs.h"

namespace tune
{

struct EvalData
{
	int phaseWeights[6];
	int materialMG[6];
	int materialEG[6];
	int psqtMG[6][32];
	int psqtEG[6][32];
};

struct EvalCache
{
	int totalPhase;
};

constexpr size_t NUM_PARAMS = sizeof(EvalData) / sizeof(int);
constexpr double K_VAL = 0.96;

union EvalParams
{
	EvalData data;
	int params[NUM_PARAMS];
};

constexpr EvalParams defaultParams = {
	{
		// phase weights
		{0, 4, 2, 1, 1, 0},
		// material middlegame
		{0, 900, 500, 330, 320, 100},
		// material endgame
		{0, 900, 500, 330, 320, 100},
		// psqt middlegame
		{
			// king
			{
				-30,-40,-40,-50,
				-30,-40,-40,-50,
				-30,-40,-40,-50,
				-30,-40,-40,-50,
				-20,-30,-30,-40,
				-10,-20,-20,-20,
				 20, 20,  0,  0,
				 20, 30, 10,  0
			},
			// queen
			{
				-20,-10,-10, -5,
				-10,  0,  0,  0,
				-10,  0,  5,  5,
				 -5,  0,  5,  5,
				  0,  0,  5,  5,
				-10,  5,  5,  5,
				-10,  0,  5,  0,
				-20,-10,-10, -5
			},
			// rook
			{
				  0,  0,  0,  0,
				  5, 10, 10, 10,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				  0,  0,  0,  5
			},
			// bishop
			{
				-20,-10,-10,-10,
				-10,  0,  0,  0,
				-10,  0,  5, 10,
				-10,  5,  5, 10,
				-10,  0, 10, 10,
				-10, 10, 10, 10,
				-10,  5,  0,  0,
				-20,-10,-10,-10
			},
			// knight
			{
				-50,-40,-30,-30,
				-40,-20,  0,  0,
				-30,  0, 10, 15,
				-30,  5, 15, 20,
				-30,  0, 15, 20,
				-30,  5, 10, 15,
				-40,-20,  0,  5,
				-50,-40,-30,-30
			},
			// pawn
			{
				 0,  0,  0,  0,
				50, 50, 50, 50,
				10, 10, 20, 30,
				 5,  5, 10, 25,
				 0,  0,  0, 20,
				 5, -5,-10,  0,
				 5, 10, 10,-20,
				 0,  0,  0,  0
			}
		},
		// psqt endgame
		{
			// king
			{
				-50,-40,-30,-20,
				-30,-20,-10,  0,
				-30,-10, 20, 30,
				-30,-10, 30, 40,
				-30,-10, 30, 40,
				-30,-10, 20, 30,
				-30,-30,  0,  0,
				-50,-30,-30,-30
			},
			// queen
			{
				-20,-10,-10, -5,
				-10,  0,  0,  0,
				-10,  0,  5,  5,
				 -5,  0,  5,  5,
				  0,  0,  5,  5,
				-10,  5,  5,  5,
				-10,  0,  5,  0,
				-20,-10,-10, -5
			},
			// rook
			{
				  0,  0,  0,  0,
				  5, 10, 10, 10,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				  0,  0,  0,  5
			},
			// bishop
			{
				-20,-10,-10,-10,
				-10,  0,  0,  0,
				-10,  0,  5, 10,
				-10,  5,  5, 10,
				-10,  0, 10, 10,
				-10, 10, 10, 10,
				-10,  5,  0,  0,
				-20,-10,-10,-10
			},
			// knight
			{
				-50,-40,-30,-30,
				-40,-20,  0,  0,
				-30,  0, 10, 15,
				-30,  5, 15, 20,
				-30,  0, 15, 20,
				-30,  5, 10, 15,
				-40,-20,  0,  5,
				-50,-40,-30,-30
			},
			// pawn
			{
				 0,  0,  0,  0,
				50, 50, 50, 50,
				10, 10, 20, 30,
				 5,  5, 10, 25,
				 0,  0,  0, 20,
				 5, -5,-10,  0,
				 5, 10, 10,-20,
				 0,  0,  0,  0
			}
		}
	}
};

void printParams(const EvalParams& params, std::ostream& os);
bool isDegenerate(const EvalParams& params);
EvalCache genCache(const EvalParams& params);
int evaluate(const Board& board, const EvalParams& params, const EvalCache& cache);


}