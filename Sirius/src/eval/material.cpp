#include "material.h"

namespace eval
{

const int PIECE_VALUES_MG[] = {
	// none
	0,
	// pawn
	PAWN_VALUE_MG,
	// knight
	KNIGHT_VALUE_MG,
	// bishop
	BISHOP_VALUE_MG,
	// rook
	ROOK_VALUE_MG,
	// queen
	QUEEN_VALUE_MG,
	// king
	0
};

const int PIECE_VALUES_EG[] = {
	// none
	0,
	// pawn
	PAWN_VALUE_EG,
	// knight
	KNIGHT_VALUE_EG,
	// bishop
	BISHOP_VALUE_EG,
	// rook
	ROOK_VALUE_EG,
	// queen
	QUEEN_VALUE_EG,
	// king
	0
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
