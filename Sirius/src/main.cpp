#include <iostream>
#include <string>

#include "attacks.h"
#include "comm/cmdline.h"

int main()
{
	attacks::init();

	std::string mode;
	std::getline(std::cin, mode);
	if (mode == "cmdline")
	{
		comm::CmdLine cmdLine;
		for (;;)
		{
			std::string cmd;
			std::getline(std::cin, cmd);
			if (cmd == "quit")
				break;
			cmdLine.execCommand(cmd);
		}
	}
	else
	{
		std::cout << "Unrecognized mode" << std::endl;
	}
	return 0;
}