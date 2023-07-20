#pragma once

#include "input_queue.h"
#include "../search.h"

#include <sstream>

namespace comm
{

enum class CommState
{
	IDLE,
	SEARCHING,
	ABORTING,
	QUITTING
};

class UCI
{
public:
	UCI();
	~UCI() = default;

	enum class Command
	{
		INVALID,
		UCI,
		IS_READY,
		NEW_GAME,
		POSITION,
		GO,
		STOP,
		QUIT,

		DBG_PRINT
	};

	void setToFen(const char* fen);
	void makeMove(Move move);
	void unmakeMove();

	void run();
	void reportSearchInfo(const SearchInfo& info) const;
	bool checkInput();
private:
	static bool shouldQuit(const std::string& str);

	void calcLegalMoves();

	void execCommand(const std::string& command);
	Command getCommand(const std::string& command) const;

	void uciCommand() const;
	void newGameCommand();
	void positionCommand(std::istringstream& stream);
	void goCommand(std::istringstream& stream);

	Board m_Board;
	CommState m_State;
	std::deque<BoardState> m_PrevStates;
	std::vector<Move> m_PrevMoves;
	Move m_LegalMoves[256];
	uint32_t m_MoveCount;
	Search m_Search;
	InputQueue m_InputQueue;

	mutable Move currMove;
};


}
