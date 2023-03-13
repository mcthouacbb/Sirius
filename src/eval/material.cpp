#include "material.h"

namespace eval
{

const int PIECE_VALUES_MG[] = {
	// none
	0,
	// king
	0,
	// queen
	900,
	// rook
	500,
	// bishop
	330,
	// knight
	320,
	// pawn
	100
};

const int PIECE_VALUES_EG[] = {
	// none
	0,
	// king
	0,
	// queen
	900,
	// rook
	500,
	// bishop
	330,
	// knight
	320,
	// pawn
	100
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