#include "phase.h"

namespace eval
{

const int PHASE_WEIGHTS[] = {
	// none
	0,
	// king
	0,
	// queen
	QUEEN_PHASE,
	// rook
	ROOK_PHASE,
	// bishop
	BISHOP_PHASE,
	// knight
	KNIGHT_PHASE,
	// PAWN
	PAWN_PHASE
};

int getPiecePhase(PieceType piece)
{
	return PHASE_WEIGHTS[static_cast<int>(piece)];
}

int getFullEval(int mg, int eg, int phase)
{
	int phaseFactor = phase * 256 + TOTAL_PHASE / 2;
	return (mg * (256 - phaseFactor) + eg * phaseFactor) / 256;
}

}