#pragma once

#include "castling.h"
#include "defs.h"
#include "util/multi_array.h"
#include "util/prng.h"
#include <cstdint>

namespace zobrist
{

struct Keys
{
    uint64_t blackToMove;
    MultiArray<uint64_t, 2, 6, 64> pieceSquares;
    std::array<uint64_t, 16> castlingRights;
    std::array<uint64_t, 8> epFiles;
};

constexpr Keys generateZobristKeys()
{
    Keys keys = {};

    PRNG prng;
    prng.seed(8367428251681ull);

    for (int color = 0; color < 2; color++)
        for (int piece = 0; piece < 6; piece++)
            for (int square = 0; square < 64; square++)
                keys.pieceSquares[color][5 - piece][square] = prng.next64();

    for (int i = 0; i < 16; i++)
        keys.castlingRights[i] = prng.next64();

    for (int i = 0; i < 8; i++)
        keys.epFiles[i] = prng.next64();

    keys.blackToMove = prng.next64();
    return keys;
}

constexpr Keys keys = generateZobristKeys();

}

struct ZKey
{
    uint64_t value;

    void flipSideToMove();
    void addPiece(PieceType piece, Color color, Square square);
    void removePiece(PieceType piece, Color color, Square square);
    void movePiece(PieceType piece, Color color, Square src, Square dst);

    void updateCastlingRights(CastlingRights rights);
    void updateEP(uint32_t epFile);

    bool operator==(const ZKey& other) const = default;
    bool operator!=(const ZKey& other) const = default;
};

inline void ZKey::flipSideToMove()
{
    value ^= zobrist::keys.blackToMove;
}

inline void ZKey::addPiece(PieceType piece, Color color, Square square)
{
    value ^=
        zobrist::keys.pieceSquares[static_cast<int>(color)][static_cast<int>(piece)][square.value()];
}

inline void ZKey::removePiece(PieceType piece, Color color, Square square)
{
    value ^=
        zobrist::keys.pieceSquares[static_cast<int>(color)][static_cast<int>(piece)][square.value()];
}

inline void ZKey::movePiece(PieceType piece, Color color, Square src, Square dst)
{
    value ^= zobrist::keys.pieceSquares[static_cast<int>(color)][static_cast<int>(piece)][src.value()]
        ^ zobrist::keys.pieceSquares[static_cast<int>(color)][static_cast<int>(piece)][dst.value()];
}

inline void ZKey::updateCastlingRights(CastlingRights castlingRights)
{
    value ^= zobrist::keys.castlingRights[castlingRights.value()];
}

inline void ZKey::updateEP(uint32_t epFile)
{
    value ^= zobrist::keys.epFiles[epFile];
}
