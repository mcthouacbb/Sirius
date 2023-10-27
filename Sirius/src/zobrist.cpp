#include "zobrist.h"
#include "prng.h"
#include <iostream>
#include <array>

#include <bitset>

namespace zobrist
{

uint64_t blackToMove;
std::array<std::array<std::array<uint64_t, 64>, 6>, 2> pieceSquares;
std::array<uint64_t, 16> castlingRights;
std::array<uint64_t, 8> epFiles;

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

}

void ZKey::flipSideToMove()
{
    value ^= zobrist::blackToMove;
}

void ZKey::addPiece(PieceType piece, Color color, uint32_t square)
{
    value ^= zobrist::pieceSquares[static_cast<int>(color)][static_cast<int>(piece) - 1][square];
}

void ZKey::removePiece(PieceType piece, Color color, uint32_t square)
{
    value ^= zobrist::pieceSquares[static_cast<int>(color)][static_cast<int>(piece) - 1][square];
}

void ZKey::movePiece(PieceType piece, Color color, uint32_t src, uint32_t dst)
{
    value ^= (zobrist::pieceSquares[static_cast<int>(color)][static_cast<int>(piece) - 1][src] ^ zobrist::pieceSquares[static_cast<int>(color)][static_cast<int>(piece) - 1][dst]);
}

void ZKey::updateCastlingRights(uint32_t castlingRights)
{
    value ^= zobrist::castlingRights[castlingRights];
}

void ZKey::updateEP(uint32_t epFile)
{
    value ^= zobrist::epFiles[epFile];
}
