#pragma once

#include "icomm.h"

namespace comm
{

class CmdLine : public IComm
{
public:
	enum class Command
	{
		SET_POSITION,
		MAKE_MOVE,
		UNDO_MOVE,
		PRINT_BOARD,
		STATIC_EVAL,
		QUIESCENCE_EVAL,
		SEARCH,
		RUN_TESTS,
		PERFT,
		SET_CLOCK,
		BOOK
	};

	virtual void execCommand(const std::string& command) override;
};

}