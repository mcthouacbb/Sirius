#pragma once

#include "../defs.h"
#include "combined_psqt.h"
#include "phase.h"

#include <iostream>

class Board;

namespace eval
{

constexpr int BUCKET_COUNT = 2;
inline int getKingBucket(Square kingSq)
{
    return kingSq.file() >= FILE_E;
}

struct Accumulator
{
    std::array<ScorePair, BUCKET_COUNT> materialPsqt;

    void addPiece(Color color, PieceType piece, Square square)
    {
        for (int bucket = 0; bucket < BUCKET_COUNT; bucket++)
            materialPsqt[bucket] += combinedPsqtScore(bucket, color, piece, square);
    }

    void removePiece(Color color, PieceType piece, Square square)
    {
        for (int bucket = 0; bucket < BUCKET_COUNT; bucket++)
            materialPsqt[bucket] -= combinedPsqtScore(bucket, color, piece, square);
    }

    void movePiece(Color color, PieceType piece, Square src, Square dst)
    {
        for (int bucket = 0; bucket < BUCKET_COUNT; bucket++)
            materialPsqt[bucket] += combinedPsqtScore(bucket, color, piece, dst)
                - combinedPsqtScore(bucket, color, piece, src);
    }
};

struct PsqtState
{
    int phase;
    std::array<Accumulator, 2> accumulators;

    void init();
    ScorePair evaluate(const Board& board) const;
    void addPiece(Color color, PieceType piece, Square square);
    void removePiece(Color color, PieceType piece, Square square);
    void movePiece(Color color, PieceType piece, Square src, Square dst);
};

inline void PsqtState::init()
{
    phase = TOTAL_PHASE;
    accumulators.fill({{ScorePair(0, 0), ScorePair(0, 0)}});
}

inline void PsqtState::addPiece(Color color, PieceType piece, Square square)
{
    accumulators[static_cast<int>(color)].addPiece(color, piece, square);
    phase -= getPiecePhase(piece);
}

inline void PsqtState::removePiece(Color color, PieceType piece, Square square)
{
    accumulators[static_cast<int>(color)].removePiece(color, piece, square);
    phase += getPiecePhase(piece);
}

inline void PsqtState::movePiece(Color color, PieceType piece, Square src, Square dst)
{
    accumulators[static_cast<int>(color)].movePiece(color, piece, src, dst);
}

}
