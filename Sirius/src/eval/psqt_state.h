#pragma once

#include "../defs.h"
#include "../util/enum_array.h"
#include "combined_psqt.h"

#include <iostream>

class Board;

namespace eval
{

constexpr i32 BUCKET_COUNT = 2;
inline i32 getKingBucket(Square kingSq)
{
    return kingSq.file() >= FILE_E;
}

struct Accumulator
{
    std::array<ScorePair, BUCKET_COUNT> materialPsqt;

    void addPiece(Color color, PieceType piece, Square square)
    {
        for (i32 bucket = 0; bucket < BUCKET_COUNT; bucket++)
            materialPsqt[bucket] += combinedPsqtScore(bucket, color, piece, square);
    }

    void removePiece(Color color, PieceType piece, Square square)
    {
        for (i32 bucket = 0; bucket < BUCKET_COUNT; bucket++)
            materialPsqt[bucket] -= combinedPsqtScore(bucket, color, piece, square);
    }

    void movePiece(Color color, PieceType piece, Square src, Square dst)
    {
        for (i32 bucket = 0; bucket < BUCKET_COUNT; bucket++)
            materialPsqt[bucket] += combinedPsqtScore(bucket, color, piece, dst)
                - combinedPsqtScore(bucket, color, piece, src);
    }
};

struct PsqtState
{
    ColorArray<Accumulator> accumulators;

    void init();
    ScorePair evaluate(const Board& board) const;
    void addPiece(Color color, PieceType piece, Square square);
    void removePiece(Color color, PieceType piece, Square square);
    void movePiece(Color color, PieceType piece, Square src, Square dst);
};

inline void PsqtState::init()
{
    accumulators.fill({{ScorePair(0, 0), ScorePair(0, 0)}});
}

inline void PsqtState::addPiece(Color color, PieceType piece, Square square)
{
    accumulators[color].addPiece(color, piece, square);
}

inline void PsqtState::removePiece(Color color, PieceType piece, Square square)
{
    accumulators[color].removePiece(color, piece, square);
}

inline void PsqtState::movePiece(Color color, PieceType piece, Square src, Square dst)
{
    accumulators[color].movePiece(color, piece, src, dst);
}

}
