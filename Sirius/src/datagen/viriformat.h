#pragma once

#include "marlinformat.h"

namespace viriformat
{

struct ViriMove
{
    explicit ViriMove(u16 data);
    explicit ViriMove(Move move);
    Move toMove() const;

private:
    u16 m_Data;
};

struct Game
{
    marlinformat::PackedBoard startpos;
    std::vector<std::pair<ViriMove, i16>> moves;

    static Game read(std::istream& is);
    void write(std::ostream& os) const;
};

}
