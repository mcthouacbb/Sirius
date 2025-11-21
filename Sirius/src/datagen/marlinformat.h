#pragma once

#include "../board.h"

namespace marlinformat
{

struct U4Array32
{
    std::array<u8, 16> data;

    u8 get(usize index) const
    {
        return (data[index / 2] >> (4 * (index % 2))) & 0xF;
    }

    void set(usize index, u8 value)
    {
        assert(value < 16);
        data[index / 2] |= (value << (4 * (index % 2)));
    }
};

enum class WDL : u8
{
    BLACK_WIN,
    DRAW,
    WHITE_WIN
};

struct PackedBoard
{
    u64 occ;
    U4Array32 pieces;
    u8 stmEpSquare;
    u8 halfMoveClock;
    u16 fullMoveNumber;
    i16 score;
    WDL wdl;

    u8 padding;
};

struct MarlinFormatUnpack
{
    Board board;
    i32 score;
    WDL wdl;
};

PackedBoard packBoard(const Board& board, i32 score, WDL wdl);
MarlinFormatUnpack unpackBoard(const PackedBoard& packedBoard);

}
