#pragma once

#include "../defs.h"
#include "combined_psqt.h"
#include "phase.h"

#include <iostream>

namespace eval
{

struct EvalState
{
	int phase;
	PackedScore materialPsqt;

	void init();
	void addPiece(Color color, PieceType piece, int square);
	void removePiece(Color color, PieceType piece, int square);
	void movePiece(Color color, PieceType piece, int src, int dst);
};

inline void EvalState::init()
{
	phase = TOTAL_PHASE;
    materialPsqt = PackedScore(0, 0);
}

inline void EvalState::addPiece(Color color, PieceType piece, int square)
{
    materialPsqt += combinedPsqtScore(color, piece, square);
	phase -= getPiecePhase(piece);
}

inline void EvalState::removePiece(Color color, PieceType piece, int square)
{
    materialPsqt -= combinedPsqtScore(color, piece, square);
	phase += getPiecePhase(piece);
}

inline void EvalState::movePiece(Color color, PieceType piece, int src, int dst)
{
    materialPsqt += combinedPsqtScore(color, piece, dst) - combinedPsqtScore(color, piece, src);
}

}
