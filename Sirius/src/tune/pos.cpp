#include "pos.h"

namespace tune
{

std::vector<Pos> parseEpdFile(const std::string& str)
{
	std::vector<Pos> positions;

	int lineStart = 0;
	int lineEnd = str.find('\n', 0);

	do
	{
		if (str[lineStart] == '\0')
			break;
		double result;
		int lastChar = lineEnd == -1 ? str.size() - 1 : lineEnd - 1;
		int len = lineEnd - lineStart;
		if (str[lastChar] == '0')
		{
			result = 1;
			len -= 3;
		}
		else if (str[lastChar] == '2')
		{
			result = 0.5;
			len -= 7;
		}
		else if (str[lastChar] == '1')
		{
			result = 0;
			len -= 3;
		}
		else
		{
			/*std::cout << str[lastChar] << std::endl;
			std::cout << lineEnd << std::endl;
			std::cout.write(&str[lastChar - 10], 50) << std::endl;*/
			throw std::exception("tune::parseEpdFile(): Unexpected character in ");
		}
		positions.push_back({str.c_str() + lineStart, len, result});
		lineStart = lineEnd + 1;
		lineEnd = str.find('\n', lineStart);
	} while (lineEnd != std::string::npos);

	return positions;
}


}