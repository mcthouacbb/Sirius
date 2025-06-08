#pragma once

#include "../board.h"
#include "../movegen.h"

namespace uci
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

MoveStrFind findMoveFromUCI(const Board& board, const MoveList& legalMoves, const char* moveStr);
MoveStrFind findMoveFromSAN(const Board& board, const MoveList& legalMoves, const char* moveStr);

std::string convMoveToUCI(const Board& board, Move move);
std::string convMoveToSAN(const Board& board, const MoveList& legalMoves, Move move);

}
