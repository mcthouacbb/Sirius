#pragma once

#include "../defs.h"
#include "combined_psqt.h"
#include "phase.h"

#include <iostream>

class Board;

namespace eval
{

constexpr int BUCKET_COUNT = 2;
inline int getKingBucket(int kingSq)
{
    return fileOf(kingSq) >= 4;
}

struct PsqtState
{
    int phase;
    std::array<int, 2> buckets;
    bool needsRefresh;
    PackedScore materialPsqt;

    void init();
    void refresh(const Board& board);
    void addPiece(Color color, PieceType piece, int square);
    void removePiece(Color color, PieceType piece, int square);
    void movePiece(Color color, PieceType piece, int src, int dst);

};

inline void PsqtState::init()
{
    phase = TOTAL_PHASE;
    buckets.fill(-1);
    needsRefresh = true;
    materialPsqt = PackedScore(0, 0);
}

inline void PsqtState::addPiece(Color color, PieceType piece, int square)
{
    if (!needsRefresh)
        materialPsqt += combinedPsqtScore(buckets[static_cast<int>(color)], color, piece, square);
    phase -= getPiecePhase(piece);
}

inline void PsqtState::removePiece(Color color, PieceType piece, int square)
{
    if (!needsRefresh)
        materialPsqt -= combinedPsqtScore(buckets[static_cast<int>(color)], color, piece, square);
    phase += getPiecePhase(piece);
}

inline void PsqtState::movePiece(Color color, PieceType piece, int src, int dst)
{
    if (piece == PieceType::KING && getKingBucket(src) != getKingBucket(dst))
    {
        needsRefresh = true;
    }
    else if (!needsRefresh)
    {
        materialPsqt += combinedPsqtScore(buckets[static_cast<int>(color)], color, piece, dst) - combinedPsqtScore(buckets[static_cast<int>(color)], color, piece, src);
    }
}

}
