#pragma once

#include <cstdint>
#include <array>
#include "defs.h"

namespace zobrist
{

struct Keys
{
    uint64_t blackToMove;
    std::array<std::array<std::array<uint64_t, 64>, 6>, 2> pieceSquares;
    std::array<uint64_t, 16> castlingRights;
    std::array<uint64_t, 8> epFiles;
};

extern Keys keys;

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

inline void ZKey::flipSideToMove()
{
    value ^= zobrist::keys.blackToMove;
}

inline void ZKey::addPiece(PieceType piece, Color color, uint32_t square)
{
    value ^= zobrist::keys.pieceSquares[static_cast<int>(color)][static_cast<int>(piece)][square];
}

inline void ZKey::removePiece(PieceType piece, Color color, uint32_t square)
{
    value ^= zobrist::keys.pieceSquares[static_cast<int>(color)][static_cast<int>(piece)][square];
}

inline void ZKey::movePiece(PieceType piece, Color color, uint32_t src, uint32_t dst)
{
    value ^= zobrist::keys.pieceSquares[static_cast<int>(color)][static_cast<int>(piece)][src] ^ zobrist::keys.pieceSquares[static_cast<int>(color)][static_cast<int>(piece)][dst];
}

inline void ZKey::updateCastlingRights(uint32_t castlingRights)
{
    value ^= zobrist::keys.castlingRights[castlingRights];
}

inline void ZKey::updateEP(uint32_t epFile)
{
    value ^= zobrist::keys.epFiles[epFile];
}
