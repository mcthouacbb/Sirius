#include "zobrist.h"
#include "prng.h"
#include <iostream>

#include <bitset>

namespace zobrist
{

uint64_t blackToMove;
uint64_t pieceSquares[2][6][64];
uint64_t castlingRights[16];
uint64_t epFiles[8];

void init()
{
	PRNG prng;
	prng.seed(8367428251681ull);

	for (int color = 0; color < 2; color++)
		for (int piece = 0; piece < 6; piece++)
			for (int square = 0; square < 64; square++)
				pieceSquares[color][5 - piece][square] = prng.next64();

	for (int i = 0; i < 16; i++)
		castlingRights[i] = prng.next64();

	for (int i = 0; i < 8; i++)
		epFiles[i] = prng.next64();

	blackToMove = prng.next64();
}

/*const char* pieces[7] = {
	"NONE", "KING", "QUEEN", "ROOK", "BISHOP", "KNIGHT", "PAWN"
};*/

}

void ZKey::flipSideToMove()
{
	// std::cout << "FLIP SIDE TO MOVE" << std::endl;
	value ^= zobrist::blackToMove;
}

void ZKey::addPiece(PieceType piece, Color color, uint32_t square)
{
	// std::cout << "ADD PIECE " << zobrist::pieces[(int)piece] << ' ' << (int)color << ' ' << square << std::endl;
	value ^= zobrist::pieceSquares[static_cast<int>(color)][static_cast<int>(piece) - 1][square];
}

void ZKey::removePiece(PieceType piece, Color color, uint32_t square)
{
	// std::cout << "REMOVE PIECE " << zobrist::pieces[(int)piece] << ' ' << (int)color << ' ' << square << std::endl;
	value ^= zobrist::pieceSquares[static_cast<int>(color)][static_cast<int>(piece) - 1][square];
}

void ZKey::movePiece(PieceType piece, Color color, uint32_t src, uint32_t dst)
{
	// std::cout << "MOVE PIECE " << zobrist::pieces[(int)piece] << ' ' << (int)color << ' ' << src << "->" << dst << std::endl;
	value ^= (zobrist::pieceSquares[static_cast<int>(color)][static_cast<int>(piece) - 1][src] ^ zobrist::pieceSquares[static_cast<int>(color)][static_cast<int>(piece) - 1][dst]);
}

void ZKey::updateCastlingRights(uint32_t castlingRights)
{
	// std::cout << "CHANGE CASTLE " << castlingRights << std::endl;
	value ^= zobrist::castlingRights[castlingRights];
}

void ZKey::updateEP(uint32_t epFile)
{
	// std::cout << "UPDATE EP " << epFile << std::endl;
	value ^= zobrist::epFiles[epFile];
}