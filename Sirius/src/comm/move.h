#pragma once

#include "../board.h"

namespace comm
{

struct MoveStrFind
{
    const Move* move;
    const char* end;

// invalid = {nullptr, moveStr};
// not found = {end, moveStr + moveLen};
// ambiguous = {end + 1, moveStr + moveLen};
};

MoveStrFind findMoveFromPCN(const Move* begin, const Move* end, const char* moveStr);
MoveStrFind findMoveFromSAN(const Board& board, const Move* begin, const Move* end, const char* moveStr);

std::string convMoveToPCN(Move move);
std::string convMoveToSAN(const Board& board, const Move* begin, const Move* end, Move move);

}
