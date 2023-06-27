#include "material.h"

namespace eval
{

const int PIECE_VALUES_MG[] = {
	// none
	0,
	// king
	0,
	// queen
	QUEEN_VALUE_MG,
	// rook
	ROOK_VALUE_MG,
	// bishop
	BISHOP_VALUE_MG,
	// knight
	KNIGHT_VALUE_MG,
	// pawn
	PAWN_VALUE_MG
};

const int PIECE_VALUES_EG[] = {
	// none
	0,
	// king
	0,
	// queen
	QUEEN_VALUE_EG,
	// rook
	ROOK_VALUE_EG,
	// bishop
	BISHOP_VALUE_EG,
	// knight
	KNIGHT_VALUE_EG,
	// pawn
	PAWN_VALUE_EG
};

int getPieceValueMG(PieceType piece)
{
	return PIECE_VALUES_MG[static_cast<int>(piece)];
}

int getPieceValueEG(PieceType piece)
{
	return PIECE_VALUES_EG[static_cast<int>(piece)];
}

}