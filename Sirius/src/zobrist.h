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
    u64 blackToMove;
    MultiArray<u64, 2, 6, 64> pieceSquares;
    std::array<u64, 16> castlingRights;
    std::array<u64, 8> epFiles;
};

constexpr Keys generateZobristKeys()
{
    Keys keys = {};

    PRNG prng;
    prng.seed(8367428251681ull);

    for (i32 color = 0; color < 2; color++)
        for (i32 piece = 0; piece < 6; piece++)
            for (i32 square = 0; square < 64; square++)
                keys.pieceSquares[color][5 - piece][square] = prng.next64();

    for (i32 i = 0; i < 16; i++)
        keys.castlingRights[i] = prng.next64();

    for (i32 i = 0; i < 8; i++)
        keys.epFiles[i] = prng.next64();

    keys.blackToMove = prng.next64();
    return keys;
}

constexpr Keys keys = generateZobristKeys();

}

struct ZKey
{
    u64 value;

    void flipSideToMove();
    void addPiece(PieceType piece, Color color, Square square);
    void removePiece(PieceType piece, Color color, Square square);
    void movePiece(PieceType piece, Color color, Square src, Square dst);

    void updateCastlingRights(CastlingRights rights);
    void updateEP(u32 epFile);

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
        zobrist::keys.pieceSquares[static_cast<i32>(color)][static_cast<i32>(piece)][square.value()];
}

inline void ZKey::removePiece(PieceType piece, Color color, Square square)
{
    value ^=
        zobrist::keys.pieceSquares[static_cast<i32>(color)][static_cast<i32>(piece)][square.value()];
}

inline void ZKey::movePiece(PieceType piece, Color color, Square src, Square dst)
{
    value ^= zobrist::keys.pieceSquares[static_cast<i32>(color)][static_cast<i32>(piece)][src.value()]
        ^ zobrist::keys.pieceSquares[static_cast<i32>(color)][static_cast<i32>(piece)][dst.value()];
}

inline void ZKey::updateCastlingRights(CastlingRights castlingRights)
{
    value ^= zobrist::keys.castlingRights[castlingRights.value()];
}

inline void ZKey::updateEP(u32 epFile)
{
    value ^= zobrist::keys.epFiles[epFile];
}
