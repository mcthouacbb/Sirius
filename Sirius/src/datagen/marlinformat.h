#pragma once

#include "../board.h"

namespace marlinformat
{

struct U4Array32
{
    std::array<uint8_t, 16> data;

    uint8_t get(size_t index) const
    {
        return (data[index / 2] >> (4 * (index % 2))) & 0xF;
    }

    void set(size_t index, uint8_t value)
    {
        assert(value < 16);
        data[index / 2] |= (value << (4 * (index % 2)));
    }
};

enum class WDL : uint8_t
{
    BLACK_WIN,
    DRAW,
    WHITE_WIN
};

struct PackedBoard
{
    uint64_t occ;
    U4Array32 pieces;
    uint8_t stmEpSquare;
    uint8_t halfMoveClock;
    uint16_t fullMoveNumber;
    int16_t score;
    WDL wdl;

    uint8_t padding;
};

struct MarlinFormatUnpack
{
    Board board;
    int score;
    WDL wdl;
};

PackedBoard packBoard(const Board& board, int score, WDL wdl);
MarlinFormatUnpack unpackBoard(const PackedBoard& packedBoard);

}
