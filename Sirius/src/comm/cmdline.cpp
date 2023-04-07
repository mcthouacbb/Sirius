#include "cmdline.h"

#include <sstream>

namespace comm
{

namespace
{

}

void CmdLine::execCommand(const std::string& command)
{
	std::stringstream stream(command);

	std::string commandName;

	stream >> commandName;
}

}