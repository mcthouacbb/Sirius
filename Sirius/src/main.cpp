#include <iostream>
#include <string>

#include "attacks.h"
#include "zobrist.h"
#include "comm/icomm.h"
#include "comm/cmdline.h"
#include "comm/uci.h"

namespace comm
{
	IComm* currComm;
}

int main(int argc, char** argv)
{
	attacks::init();
	zobrist::init();

	std::string mode;
	std::getline(std::cin, mode);

	std::cin.sync_with_stdio(false);
	std::cout.sync_with_stdio(false);
	
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