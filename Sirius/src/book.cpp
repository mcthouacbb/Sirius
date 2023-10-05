#include "book.h"
#include "comm/move.h"
#include "movegen.h"
#include <algorithm>
#include <deque>
#include <cstring>

void Book::loadFromPGN(const char* pgn)
{
	const char* currChar = pgn;
	while (isdigit(*currChar) || isspace(*currChar) || *currChar == '.')
		currChar++;
	BoardState rootState;
	Board board(rootState);

	Move moves[256];
	Move* end;
	std::deque<BoardState> prevStates;
	while (true)
	{
		while (*currChar != '*')
		{
			end = genMoves<MoveGenType::LEGAL>(board, moves);

			comm::MoveStrFind find = comm::findMoveFromSAN(board, moves, end, currChar);
			// std::cout.write(currChar, std::min(static_cast<int>(strlen(currChar)), 10)) << std::endl << std::endl;
			if (!find.move)
				throw std::runtime_error("Invalid");
			if (find.move == end)
				throw std::runtime_error("Not found");
			if (find.move == end + 1)
				throw std::runtime_error("Ambiguous");

			auto it = m_Entries.find(board.zkey().value);

			if (it != m_Entries.end())
			{
				if (std::find(it->second.begin(), it->second.end(), BookEntry{*find.move}) == it->second.end())
					it->second.push_back({*find.move});
			}
			else
			{
				m_Entries.insert({board.zkey().value, {{*find.move}}});
			}

			prevStates.push_back({});
			board.makeMove(*find.move, prevStates.back());

			currChar = find.end;

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
