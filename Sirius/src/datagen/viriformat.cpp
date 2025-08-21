#include "viriformat.h"

namespace viriformat
{

ViriMove::ViriMove(uint16_t data)
    : m_Data(data)
{
}

ViriMove::ViriMove(Move move)
    : m_Data(0)
{
    m_Data |= move.fromSq().value();
    m_Data |= move.toSq().value() << 6;
    switch (move.promotion())
    {
        // knight is 0
        case Promotion::BISHOP:
            m_Data |= 1 << 12;
            break;
        case Promotion::ROOK:
            m_Data |= 2 << 12;
            break;
        case Promotion::QUEEN:
            m_Data |= 3 << 12;
            break;
    }

    switch (move.type())
    {
        // none is 0
        case MoveType::ENPASSANT:
            m_Data |= 1 << 14;
            break;
        case MoveType::CASTLE:
            m_Data |= 2 << 14;
            break;
        case MoveType::PROMOTION:
            m_Data |= 3 << 14;
            break;
    }
}

Move ViriMove::toMove() const
{
    int fromSq = m_Data & 0x3F;
    int toSq = (m_Data >> 6) & 0x3F;
    Promotion promotion = Promotion::KNIGHT;
    switch ((m_Data >> 12) & 0x3)
    {
        case 1:
            promotion = Promotion::BISHOP;
            break;
        case 2:
            promotion = Promotion::ROOK;
            break;
        case 3:
            promotion = Promotion::QUEEN;
            break;
    }

    MoveType type = MoveType::NONE;
    switch (m_Data >> 14)
    {
        case 1:
            type = MoveType::ENPASSANT;
            break;
        case 2:
            type = MoveType::CASTLE;
            break;
        case 3:
            type = MoveType::PROMOTION;
            break;
    }

    return Move(Square(fromSq), Square(toSq), type, promotion);
}

constexpr uint32_t NULL_TERMINATOR = 0;

void Game::write(std::ostream& os) const
{
    os.write(reinterpret_cast<const char*>(&startpos), sizeof(marlinformat::PackedBoard));
    os.write(reinterpret_cast<const char*>(moves.data()), 4 * moves.size());
    os.write(reinterpret_cast<const char*>(&NULL_TERMINATOR), sizeof(NULL_TERMINATOR));
}

Game Game::read(std::istream& is)
{
    Game result;
    is.read(reinterpret_cast<char*>(&result.startpos), sizeof(marlinformat::PackedBoard));
    while (true)
    {
        uint32_t moveData;
        is.read(reinterpret_cast<char*>(&moveData), 4);

        if (moveData == NULL_TERMINATOR)
            break;

        ViriMove move(moveData & 0xFFFF);
        int16_t score = static_cast<int16_t>(moveData >> 16);
        result.moves.push_back({move, score});
    }
    return result;
}

}
