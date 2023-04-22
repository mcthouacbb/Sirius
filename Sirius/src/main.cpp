#include <iostream>
#include <string>

#include "attacks.h"
#include "zobrist.h"
#include "comm/icomm.h"
#include "comm/cmdline.h"

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
	if (mode == "cmdline")
	{
		comm::CmdLine cmdLine;
		comm::currComm = &cmdLine;
		for (;;)
		{
			std::string cmd;
			std::getline(std::cin, cmd);
			if (cmd == "quit")
				break;
			cmdLine.execCommand(cmd);
		}
	}
	else if (mode == "uci")
	{
		std::cout << "id name Sirius v0.2\n";
		std::cout << "id author AspectOfTheNoob\n";
		std::cout << "uciok\n";
	}
	else
	{
		std::cout << "Unrecognized mode" << std::endl;
	}
	return 0;
}