#pragma once

#include "icomm.h"
#include "input_queue.h"

#include <sstream>

namespace comm
{

class UCI : public IComm
{
public:
	UCI();
	virtual ~UCI() override = default;

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


	virtual void run() override;
	virtual void reportSearchInfo(const SearchInfo& info) const override;
	virtual bool checkInput() override;
private:
	static bool shouldQuit(const std::string& str);

	void execCommand(const std::string& command);
	Command getCommand(const std::string& command) const;

	void uciCommand() const;
	void newGameCommand();
	void positionCommand(std::istringstream& stream);
	void goCommand(std::istringstream& stream);

	InputQueue m_InputQueue;
};


}