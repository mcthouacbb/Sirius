#pragma once

#include "icomm.h"
#include "input_queue.h"

#include <sstream>

namespace comm
{

class Debug : public IComm
{
public:
	Debug();
	virtual ~Debug() override = default;

	enum class Command
	{
		INVALID,
		EPD_EVALS,
		PGN_TO_EPD,
		TUNE_ERROR,
		TUNE_OPTIMIZE,
		TUNE_NORMALIZE,
		QUIT
	};

	virtual void run() override;
	virtual void reportSearchInfo(const SearchInfo& info) const override;
	virtual bool checkInput() override;
private:
	static bool shouldQuit(const std::string& str);

	void execCommand(const std::string& command);
	Command getCommand(const std::string& command) const;

	void genEpdEvals(std::istringstream& stream) const;
	void pgnToEpd(std::istringstream& stream) const;
	void selectPositions(std::istringstream& stream) const;
	void tuneError(std::istringstream& stream) const;
	void tuneOptimize(std::istringstream& stream) const;
	void tuneNormalize() const;

	InputQueue m_InputQueue;
};

}