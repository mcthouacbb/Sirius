#pragma once

#include "marlinformat.h"

namespace viriformat
{

struct ViriMove
{
    explicit ViriMove(uint16_t data);
    explicit ViriMove(Move move);
    Move toMove() const;

private:
    uint16_t m_Data;
};

struct Game
{
    marlinformat::PackedBoard startpos;
    std::vector<std::pair<ViriMove, int16_t>> moves;

    static Game read(std::istream& is);
    void write(std::ostream& os) const;
};

}
