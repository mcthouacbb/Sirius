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

struct Accumulator
{
    bool needsRefresh;
    int bucket;
    PackedScore materialPsqt;

    void refresh(const Board& board, Color c);

    void addPiece(Color color, PieceType piece, int square)
    {
        materialPsqt += combinedPsqtScore(bucket, color, piece, square);
    }

    void removePiece(Color color, PieceType piece, int square)
    {
        materialPsqt -= combinedPsqtScore(bucket, color, piece, square);
    }

    void movePiece(Color color, PieceType piece, int src, int dst)
    {
        materialPsqt += combinedPsqtScore(bucket, color, piece, dst) - combinedPsqtScore(bucket, color, piece, src);
    }
};

struct PsqtState
{
    int phase;
    mutable std::array<Accumulator, 2> accumulators;

    void init();
    PackedScore evaluate(const Board& board) const;
    void addPiece(Color color, PieceType piece, int square);
    void removePiece(Color color, PieceType piece, int square);
    void movePiece(Color color, PieceType piece, int src, int dst);

};

inline void PsqtState::init()
{
    phase = TOTAL_PHASE;
    accumulators.fill({false, -1, PackedScore(0, 0)});
}

inline void PsqtState::addPiece(Color color, PieceType piece, int square)
{
    auto& acc = accumulators[static_cast<int>(color)];
    if (!acc.needsRefresh)
        accumulators[static_cast<int>(color)].addPiece(color, piece, square);
    phase -= getPiecePhase(piece);
}

inline void PsqtState::removePiece(Color color, PieceType piece, int square)
{
    auto& acc = accumulators[static_cast<int>(color)];
    if (!acc.needsRefresh)
        acc.removePiece(color, piece, square);
    phase += getPiecePhase(piece);
}

inline void PsqtState::movePiece(Color color, PieceType piece, int src, int dst)
{
    auto& acc = accumulators[static_cast<int>(color)];
    if (piece == PieceType::KING && getKingBucket(src) != getKingBucket(dst))
        acc.needsRefresh = true;
    else if (!acc.needsRefresh)
        acc.movePiece(color, piece, src, dst);
}

}
