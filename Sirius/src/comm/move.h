#pragma once

#include "../board.h"
#include "../movegen.h"

namespace comm
{

struct MoveStrFind
{
    enum class Result
    {
        FOUND,
        INVALID,
        NOT_FOUND,
        AMBIGUOUS
    } result;
    Move move;
    int len;
    static constexpr int NO_INDEX = 256;

// invalid = {nullptr, moveStr};
// not found = {end, moveStr + moveLen};
// ambiguous = {end + 1, moveStr + moveLen};
};

MoveStrFind findMoveFromPCN(const MoveList& legalMoves, const char* moveStr);
MoveStrFind findMoveFromSAN(const Board& board, const MoveList& legalMoves, const char* moveStr);

std::string convMoveToPCN(Move move);
std::string convMoveToSAN(const Board& board, const MoveList& legalMoves, Move move);

}
