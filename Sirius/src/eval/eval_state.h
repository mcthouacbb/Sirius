#pragma once

#include "../defs.h"
#include "material.h"
#include "phase.h"
#include "psqt.h"

namespace eval
{

struct EvalState
{
	int phase;

	int materialMG[2];
	int materialEG[2];

	int psqtMG[2];
	int psqtEG[2];

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
	psqtMG[0] = psqtMG[1] = 0;
	psqtEG[0] = psqtEG[1] = 0;
}

inline void EvalState::addPiece(Color color, PieceType piece, int square)
{
	materialMG[static_cast<int>(color)] += getPieceValueMG(piece);
	materialEG[static_cast<int>(color)] += getPieceValueEG(piece);

	if (color == Color::WHITE)
		square ^= 0b111000;
	
	psqtMG[static_cast<int>(color)] += getPSQTMG(piece, square);
	psqtEG[static_cast<int>(color)] += getPSQTEG(piece, square);
	
	phase -= getPiecePhase(piece);
}

inline void EvalState::removePiece(Color color, PieceType piece, int square)
{
	materialMG[static_cast<int>(color)] -= getPieceValueMG(piece);
	materialEG[static_cast<int>(color)] -= getPieceValueEG(piece);

	if (color == Color::WHITE)
		square ^= 0b111000;
	
	psqtMG[static_cast<int>(color)] -= getPSQTMG(piece, square);
	psqtEG[static_cast<int>(color)] -= getPSQTEG(piece, square);
	
	phase += getPiecePhase(piece);
}

inline void EvalState::movePiece(Color color, PieceType piece, int src, int dst)
{
	if (color == Color::WHITE)
	{
		src ^= 0b111000;
		dst ^= 0b111000;
	}
	psqtMG[static_cast<int>(color)] -= getPSQTMG(piece, src);
	psqtEG[static_cast<int>(color)] -= getPSQTEG(piece, src);

	psqtMG[static_cast<int>(color)] += getPSQTMG(piece, dst);
	psqtEG[static_cast<int>(color)] += getPSQTEG(piece, dst);
	// material doesn't change
}

}