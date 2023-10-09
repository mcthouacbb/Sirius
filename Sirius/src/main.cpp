#include <iostream>
#include <string>

#include "attacks.h"
#include "zobrist.h"
#include "comm/icomm.h"
#include "comm/cmdline.h"
#include "comm/uci.h"
#include "eval/eval.h"

int main()
{
	attacks::init();
	zobrist::init();
	search::init();

	std::string mode;
	std::getline(std::cin, mode);

	if (mode == "cmdline")
	{
		comm::CmdLine cmdLine;
		comm::currComm = &cmdLine;
		cmdLine.run();
	}
	else if (mode == "uci")
	{
		comm::UCI uci;
		comm::currComm = &uci;
		uci.run();
	}
	else
	{
		std::cout << "Unrecognized mode" << std::endl;
	}
	return 0;
}
