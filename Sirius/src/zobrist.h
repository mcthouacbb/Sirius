#pragma once

#include <cstdint>
#include "defs.h"

namespace zobrist
{

void init();

}

struct ZKey
{
	uint64_t value;

	void flipSideToMove();
	void addPiece(PieceType piece, Color color, uint32_t square);
	void removePiece(PieceType piece, Color color, uint32_t square);
	void movePiece(PieceType piece, Color color, uint32_t src, uint32_t dst);

	void updateCastlingRights(uint32_t rights);
	void updateEP(uint32_t epFile);

	bool operator==(const ZKey& other) const = default;
	bool operator!=(const ZKey& other) const = default;
};