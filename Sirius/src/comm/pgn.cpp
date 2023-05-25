#include "pgn.h"
#include "move.h"
#include "../movegen.h"

#include <deque>

namespace comm
{

PGNFile::PGNFile(const char* filename)
	: m_File(filename), m_Str(std::istreambuf_iterator<char>{m_File}, std::istreambuf_iterator<char>())
{
	m_Curr = m_Str.c_str();
}

PGNFile::~PGNFile()
{
	
}

PGNGame PGNFile::parseGame()
{
	PGNGame game;
	while (!isdigit(*m_Curr))
	{
		if (*m_Curr == '[')
		{
			m_Curr++;
			const char* nameStart = m_Curr;
			while (isalnum(*m_Curr))
				m_Curr++;

			const char* nameEnd = m_Curr;

			while (isspace(*m_Curr))
				m_Curr++;

			// m_Curr == '"'
			m_Curr++;
			const char* valueStart = m_Curr;
			while (*m_Curr != '"')
				m_Curr++;

			const char* valueEnd = m_Curr;

			// m_Curr = ""]"
			m_Curr += 2;

			game.header.tags.insert({std::string(nameStart, nameEnd - nameStart), std::string(valueStart, valueEnd - valueStart)});
		}

		while (isspace(*m_Curr))
			m_Curr++;
	}

	std::deque<BoardState> boardStates;

	Board board;
	Move moves[256];

	while (true)
	{
		while (isspace(*m_Curr))
			m_Curr++;

		if (*m_Curr == '*')
		{
			m_Curr++;
			while (isspace(*m_Curr))
				m_Curr++;
			break;
		}
		if (isdigit(*m_Curr))
		{
			if (m_Curr[1] == '/' || m_Curr[1] == '-')
			{
				m_Curr += m_Curr[1] == '/' ? 7 : 3;
				while (isspace(*m_Curr))
					m_Curr++;
				break;
			}
			else
			{
				while (isdigit(*m_Curr))
					m_Curr++;
				m_Curr++;
				while (isspace(*m_Curr))
					m_Curr++;
			}
		}

		Move* end = genMoves<MoveGenType::LEGAL>(board, moves);
		MoveStrFind find = findMoveFromSAN(board, moves, end, m_Curr);

		if (find.move == nullptr)
		{
			throw std::runtime_error("comm::PGNFile::parseGame() 'invalid move in pgn file'");
		}

		if (find.move == end)
		{
			throw std::runtime_error("comm::PGNFile::parseGame() 'cannot find move'");
		}

		if (find.move == end + 1)
		{
			throw std::runtime_error("comm::PGNFile::parseGame() 'ambiguous move'");
		}

		Move move = *find.move;

		m_Curr = find.end;

		while (isspace(*m_Curr))
			m_Curr++;

		if (*m_Curr == '{')
		{
			m_Curr++;
			const char* commentStart = m_Curr;
			while (*m_Curr != '}')
			{
				m_Curr++;
			}
			const char* commentEnd = m_Curr;

			m_Curr++;
			game.entries.push_back({move, std::string(commentStart, commentEnd - commentStart)});
		}
		else
		{
			game.entries.push_back({move, ""});
		}
		boardStates.push_back({});
		board.makeMove(move, boardStates.back());
	}

	return game;
}

bool PGNFile::hasGame() const
{
	return *m_Curr != '\0';
}


}