#pragma once

#include "icomm.h"
#include "../book.h"

namespace comm
{

class CmdLine : public IComm
{
public:
	CmdLine();

	enum class Command
	{
		INVALID,
		SET_POSITION,
		MAKE_MOVE,
		UNDO_MOVE,
		PRINT_BOARD,
		STATIC_EVAL,
		QUIESCENCE_EVAL,
		SEARCH,
		RUN_TESTS,
		PERFT,
		BOOK
	};

	virtual void execCommand(const std::string& command) override;
private:
	Command getCommand(const std::string& command) const;

	void setPositionCommand(std::istringstream& stream);
	void makeMoveCommand(std::istringstream& stream);
	void undoMoveCommand();
	void printBoardCommand();
	void staticEvalCommand();
	void quiescenceEvalCommand();
	void searchCommand(std::istringstream& stream);
	void runTestsCommand();
	void runPerftCommand(std::istringstream& stream);
	void probeBookCommand(std::istringstream& stream);

	Book m_Book;
};

}