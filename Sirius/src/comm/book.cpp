#include "book.h"
#include "move.h"
#include "../movegen.h"
#include <algorithm>
#include <deque>
#include <cstring>

void Book::loadFromPGN(const char* pgn)
{
    const char* currChar = pgn;
    while (isdigit(*currChar) || isspace(*currChar) || *currChar == '.')
        currChar++;
    Board board;

    while (true)
    {
        while (*currChar != '*')
        {
            MoveList moves;
            genMoves<MoveGenType::LEGAL>(board, moves);

            comm::MoveStrFind find = comm::findMoveFromSAN(board, moves, currChar);
            if (find.result == comm::MoveStrFind::Result::INVALID)
                throw std::runtime_error("Invalid");
            if (find.result == comm::MoveStrFind::Result::NOT_FOUND)
                throw std::runtime_error("Not found");
            if (find.result == comm::MoveStrFind::Result::AMBIGUOUS)
                throw std::runtime_error("Ambiguous");

            auto it = m_Entries.find(board.zkey().value);

            if (it != m_Entries.end())
            {
                if (std::find(it->second.begin(), it->second.end(), BookEntry{find.move}) == it->second.end())
                    it->second.push_back({find.move});
            }
            else
            {
                m_Entries.insert({board.zkey().value, {{find.move}}});
            }

            board.makeMove(find.move);

            currChar += find.len;

            while (isdigit(*currChar) || isspace(*currChar) || *currChar == '.' || *currChar == '+')
                currChar++;
        }
        currChar++;
        if (*currChar == '\0')
            break;
        currChar++;

        while (isdigit(*currChar) || isspace(*currChar) || *currChar == '.')
            currChar++;

        board.setToFen(Board::defaultFen);
    }
}

const std::vector<BookEntry>* Book::lookup(ZKey key) const
{
    auto it = m_Entries.find(key.value);
    return it == m_Entries.end() ? nullptr : &it->second;
}
