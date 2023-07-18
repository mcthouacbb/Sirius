#pragma once

#include "../defs.h"
#include "material.h"
#include "phase.h"

namespace eval
{

struct EvalState
{
	int phase;

	int materialMG[2];
	int materialEG[2];

	// int psqtMG[2];
	// int psqtEG[2];

	void init();
	void addPiece(Color color, PieceType piece, int square);
	void removePiece(Color color, PieceType piece, int square);
	void movePiece(Color color, PieceType piece, int src, int dst);
};

inline void EvalState::init()
{
	phase = TOTAL_PHASE;
	materialMG[0] = materialMG[1] = 0;
	materialEG[0] = materialEG[1] = 0;
}

inline void EvalState::addPiece(Color color, PieceType piece, int)
{
	materialMG[static_cast<int>(color)] += getPieceValueMG(piece);
	materialEG[static_cast<int>(color)] += getPieceValueEG(piece);

	phase -= getPiecePhase(piece);
}

inline void EvalState::removePiece(Color color, PieceType piece, int)
{
	materialMG[static_cast<int>(color)] -= getPieceValueMG(piece);
	materialEG[static_cast<int>(color)] -= getPieceValueEG(piece);

	phase += getPiecePhase(piece);
}

inline void EvalState::movePiece(Color, PieceType, int, int)
{
	// material doesn't change
}

}